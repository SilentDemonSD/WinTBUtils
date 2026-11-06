@{
    Summary = 'Make the taskbar fully transparent and keep it that way'
    Run     = { param($cli) $cli.Taskbar.Apply() }
}
