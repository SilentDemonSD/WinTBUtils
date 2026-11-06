@{
    Summary = 'Clear the taskbar at every sign-in'
    Run     = { param($cli) $cli.Taskbar.EnableAutostart() }
}
