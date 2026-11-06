Add-Type -AssemblyName System.Drawing, System.Windows.Forms
Add-Type -Namespace TaskbarClearNative -Name User32 -MemberDefinition @'
[StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
[DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
[DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern IntPtr FindWindowExW(IntPtr parent, IntPtr after, string className, string title);
[DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr window, out RECT rect);
[DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr window, IntPtr after, int x, int y, int w, int h, uint flags);
'@

class TaskbarProbe {
    hidden [type]$Native = [type]'TaskbarClearNative.User32'
    hidden [type]$NativeRect = [type]'TaskbarClearNative.User32+RECT'

    [bool] AllClear() {
        $this.Native::SetProcessDPIAware() | Out-Null
        $allClear = $true
        foreach ($taskbar in $this.TaskbarWindows()) {
            $rect = [Activator]::CreateInstance($this.NativeRect)
            $this.Native::GetWindowRect($taskbar, [ref]$rect) | Out-Null
            $isClear = $this.IsClear($rect)
            "taskbar [$($rect.Left),$($rect.Top) - $($rect.Right),$($rect.Bottom)] clear=$isClear" | Write-Host
            $allClear = $allClear -and $isClear
        }
        return $allClear
    }

    hidden [IntPtr[]] TaskbarWindows() {
        $windows = [Collections.Generic.List[IntPtr]]::new()
        foreach ($class in 'Shell_TrayWnd', 'Shell_SecondaryTrayWnd') {
            $found = [IntPtr]::Zero
            while (($found = $this.Native::FindWindowExW([IntPtr]::Zero, $found, $class, $null)) -ne [IntPtr]::Zero) {
                $windows.Add($found)
            }
        }
        return $windows.ToArray()
    }

    hidden [bool] IsClear([object]$rect) {
        $width = $rect.Right - $rect.Left
        $height = $rect.Bottom - $rect.Top
        $form = New-Object System.Windows.Forms.Form
        $form.FormBorderStyle = 'None'
        $form.ShowInTaskbar = $false
        $form.StartPosition = 'Manual'
        $form.BackColor = 'Magenta'
        $form.Show()
        $this.Native::SetWindowPos($form.Handle, [IntPtr]::Zero, $rect.Left, $rect.Top, $width, $height, 0x0050) | Out-Null
        ([type]'System.Windows.Forms.Application')::DoEvents()
        Start-Sleep -Milliseconds 700
        $bitmap = New-Object System.Drawing.Bitmap $width, $height
        $graphics = ([type]'System.Drawing.Graphics')::FromImage($bitmap)
        $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
        $form.Close()
        $graphics.Dispose()
        $magenta = 0
        $samples = 0
        for ($x = 0; $x -lt $width; $x += 4) {
            $samples++
            $pixel = $bitmap.GetPixel($x, [int]($height / 2))
            if ($pixel.R -gt 230 -and $pixel.G -lt 40 -and $pixel.B -gt 230) { $magenta++ }
        }
        $bitmap.Dispose()
        Write-Host ("  magenta pixels: {0}/{1}" -f $magenta, $samples)
        return ($magenta / $samples) -gt 0.5
    }
}
