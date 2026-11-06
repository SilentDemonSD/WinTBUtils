@{
    Summary = 'Show autostart, watcher and last-run state'
    Run     = { param($cli) $cli.Taskbar.Status() }
}
