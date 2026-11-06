@{
    Summary = 'Bring back the default taskbar and turn off autostart'
    Run     = { param($cli) $cli.Taskbar.Rollback() }
}
