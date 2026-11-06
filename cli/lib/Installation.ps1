class Installation {
    [string]$Root = (Join-Path $env:LOCALAPPDATA 'WinTBUtils')
    hidden [string]$Rundll32 = "$env:WINDIR\System32\rundll32.exe"
    hidden [string]$EnvironmentKey = 'HKCU:\Environment'
    hidden [string]$RunOnceKey = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\RunOnce'

    [void] Install([string]$payload) {
        $watcherWasRunning = $this.StopWatcher()
        $this.ClearPreviousFiles()
        New-Item $this.Root -ItemType Directory -Force | Out-Null
        Copy-Item (Join-Path $payload '*') $this.Root -Recurse -Force
        $this.AddToPath()
        if ($watcherWasRunning) {
            Start-Process $this.Rundll32 -ArgumentList "`"$($this.Dll())`",Run"
        }
        Write-Host "Installed to $($this.Root). Try: tbutil clear"
    }

    [void] Uninstall([string]$runningFrom, [object]$taskbar) {
        if ($runningFrom -ne $this.Root) {
            throw "Refusing to uninstall a checkout at $runningFrom; only $($this.Root) is removable."
        }
        $taskbar.RollbackForUninstall()
        $this.StopWatcher() | Out-Null
        $this.RemoveFromPath()
        Remove-Item $this.Root -Recurse -Force -ErrorAction SilentlyContinue
        if (Test-Path $this.Root) {
            Set-ItemProperty $this.RunOnceKey -Name WinTBUtilsCleanup -Value "cmd /c rd /s /q `"$($this.Root)`""
            Write-Host 'Explorer still holds the DLL; the remaining files are removed at your next sign-in.'
        }
        Write-Host 'Uninstalled.'
    }

    hidden [string] Dll() {
        return Join-Path $this.Root 'bin\TaskbarClear.dll'
    }

    hidden [bool] StopWatcher() {
        $dll = $this.Dll()
        $watchers = @(Get-CimInstance Win32_Process -Filter "Name='rundll32.exe'" | Where-Object { $_.CommandLine -like "*$dll*" })
        $watchers | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        return $watchers.Count -gt 0
    }

    hidden [void] ClearPreviousFiles() {
        $dll = $this.Dll()
        Remove-Item "$dll.*.old" -Force -ErrorAction SilentlyContinue
        if (Test-Path $dll) {
            Move-Item $dll "$dll.$([guid]::NewGuid()).old"
        }
        Remove-Item (Join-Path $this.Root 'cli') -Recurse -Force -ErrorAction SilentlyContinue
    }

    hidden [void] AddToPath() {
        $entries = $this.PathEntries()
        if ($entries -notcontains $this.Root) {
            $this.WritePath($entries + $this.Root)
        }
        if (($env:Path -split ';') -notcontains $this.Root) {
            $env:Path = "$env:Path;$($this.Root)"
        }
    }

    hidden [void] RemoveFromPath() {
        $this.WritePath(@($this.PathEntries() | Where-Object { $_ -ne $this.Root }))
    }

    hidden [string[]] PathEntries() {
        $raw = (Get-Item $this.EnvironmentKey).GetValue('Path', '', 'DoNotExpandEnvironmentNames')
        return @($raw -split ';')
    }

    hidden [void] WritePath([string[]]$entries) {
        $key = Get-Item $this.EnvironmentKey
        $kind = if ($key.GetValueNames() -contains 'Path') { $key.GetValueKind('Path') } else { 'ExpandString' }
        [Microsoft.Win32.Registry]::SetValue('HKEY_CURRENT_USER\Environment', 'Path', ($entries -join ';'), $kind)
        [Environment]::SetEnvironmentVariable('WINTBUTILS_REFRESH', '1', 'User')
        [Environment]::SetEnvironmentVariable('WINTBUTILS_REFRESH', $null, 'User')
    }
}
