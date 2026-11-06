@{
    Summary = 'List commands'
    Run     = {
        param($cli)
        Write-Host 'usage: tbutil <command>'
        foreach ($name in $cli.Names()) {
            Write-Host ('  {0,-10} {1}' -f $name, $cli.Find($name).Summary)
        }
    }
}
