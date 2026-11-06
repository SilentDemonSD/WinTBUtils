@{
    Summary = 'Restore the taskbar and remove tbutil from this PC'
    Run     = {
        param($cli)
        . (Join-Path $cli.Root 'cli\lib\Installation.ps1')
        [Installation]::new().Uninstall($cli.Root, $cli.Taskbar)
    }
}
