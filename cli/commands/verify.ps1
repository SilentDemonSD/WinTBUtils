@{
    Summary = 'Check on screen that the taskbar is see-through; exits 1 if not'
    Run     = {
        param($cli)
        . (Join-Path $cli.Root 'cli\lib\TaskbarProbe.ps1')
        if (-not [TaskbarProbe]::new().AllClear()) { exit 1 }
    }
}
