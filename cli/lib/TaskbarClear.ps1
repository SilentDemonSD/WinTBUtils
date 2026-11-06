class TaskbarClear {
    [string]$Dll
    [string]$Rundll32 = "$env:WINDIR\System32\rundll32.exe"
    [string]$RunKey = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
    [string]$StateKey = 'HKCU:\Software\TaskbarClear'
    [string]$TranslucentTbFamily = '28017CharlesMilette.TranslucentTB_v826wp6bftszj'
    [string]$TranslucentTbStartupKey

    TaskbarClear([string]$dll) {
        $this.Dll = $dll
        $this.TranslucentTbStartupKey = "HKCU:\Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppModel\SystemAppData\$($this.TranslucentTbFamily)\TranslucentTB"
    }

    [void] Apply() {
        $this.RequireDll()
        $this.SaveRollbackPoint()
        Get-Process TranslucentTB -ErrorAction SilentlyContinue | Stop-Process -Force
        Remove-ItemProperty $this.StateKey -Name Mode -ErrorAction SilentlyContinue
        Start-Process $this.Rundll32 -ArgumentList "`"$($this.Dll)`",Run"
        if (-not $this.WaitForResult(1)) { throw 'Taskbar was not fully cleared.' }
    }

    [void] EnableAutostart() {
        $this.RequireDll()
        $this.SaveRollbackPoint()
        if (Test-Path $this.TranslucentTbStartupKey) {
            Set-ItemProperty $this.TranslucentTbStartupKey -Name State -Value 1
        }
        Set-ItemProperty $this.RunKey -Name TaskbarClear -Value "`"$($this.Rundll32)`" `"$($this.Dll)`",Run"
    }

    [void] Rollback() {
        $this.RequireDll()
        Remove-ItemProperty $this.RunKey -Name TaskbarClear -ErrorAction SilentlyContinue
        Remove-ItemProperty $this.StateKey -Name Mode -ErrorAction SilentlyContinue
        Start-Process $this.Rundll32 -ArgumentList "`"$($this.Dll)`",Restore" -Wait
        $restored = $this.WaitForResult(2)
        $saved = Get-ItemProperty $this.StateKey -ErrorAction SilentlyContinue
        if ($null -ne $saved.TranslucentTbStartupState -and (Test-Path $this.TranslucentTbStartupKey)) {
            Set-ItemProperty $this.TranslucentTbStartupKey -Name State -Value $saved.TranslucentTbStartupState
        }
        if ($saved.TranslucentTbWasRunning -eq 1) {
            Start-Process "shell:AppsFolder\$($this.TranslucentTbFamily)!TranslucentTB"
        }
        if (-not $restored) { throw 'Taskbar was not fully restored; restart Explorer to finish rollback.' }
        Remove-Item $this.StateKey -Recurse -ErrorAction SilentlyContinue
    }

    [void] RollbackForUninstall() {
        if (-not (Test-Path $this.StateKey)) { return }
        try {
            $this.Rollback()
        } catch {
            Write-Warning $_.Exception.Message
        }
        Remove-ItemProperty $this.RunKey -Name TaskbarClear -ErrorAction SilentlyContinue
        Remove-Item $this.StateKey -Recurse -ErrorAction SilentlyContinue
    }

    [void] Status() {
        $run = (Get-ItemProperty $this.RunKey -ErrorAction SilentlyContinue).TaskbarClear
        $watcher = Get-CimInstance Win32_Process -Filter "Name='rundll32.exe'" | Where-Object { $_.CommandLine -like '*TaskbarClear*' }
        Write-Host "autostart : $(if ($run) { 'on' } else { 'off' })"
        Write-Host "watcher   : $(if ($watcher) { "running, $([math]::Round($watcher.WorkingSetSize / 1MB, 1)) MB" } else { 'stopped' })"
        $state = Get-ItemProperty $this.StateKey -ErrorAction SilentlyContinue
        if ($state.Mode) {
            Write-Host "last run  : mode=$($state.Mode) advise=0x$('{0:X8}' -f $state.AdviseHr) unadvise=0x$('{0:X8}' -f $state.UnadviseHr) matched=$($state.Matched)/$($state.Expected) guardHits=$($state.GuardHits)"
        }
    }

    hidden [void] RequireDll() {
        if (-not (Test-Path $this.Dll)) { throw "Missing $($this.Dll); run build.cmd first." }
    }

    hidden [void] SaveRollbackPoint() {
        if (Test-Path $this.StateKey) {
            if ($null -ne (Get-ItemProperty $this.StateKey).TranslucentTbStartupState) { return }
        } else {
            New-Item $this.StateKey | Out-Null
        }
        $startup = (Get-ItemProperty $this.TranslucentTbStartupKey -ErrorAction SilentlyContinue).State
        if ($null -ne $startup) { Set-ItemProperty $this.StateKey -Name TranslucentTbStartupState -Value $startup }
        $wasRunning = [int][bool](Get-Process TranslucentTB -ErrorAction SilentlyContinue)
        Set-ItemProperty $this.StateKey -Name TranslucentTbWasRunning -Value $wasRunning
    }

    hidden [bool] WaitForResult([int]$mode) {
        $deadline = [DateTime]::UtcNow.AddSeconds(90)
        while ([DateTime]::UtcNow -lt $deadline) {
            $state = Get-ItemProperty $this.StateKey -ErrorAction SilentlyContinue
            if ($state.Mode -eq $mode) {
                Write-Host "mode=$mode advise=0x$('{0:X8}' -f $state.AdviseHr) unadvise=0x$('{0:X8}' -f $state.UnadviseHr) matched=$($state.Matched)/$($state.Expected)"
                return $state.Expected -gt 0 -and $state.Matched -ge $state.Expected
            }
            Start-Sleep -Milliseconds 200
        }
        Write-Host 'No result from Explorer within 90 seconds.'
        return $false
    }
}
