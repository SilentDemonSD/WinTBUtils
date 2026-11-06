@{
    Summary = 'Print the installed version'
    Run     = {
        param($cli)
        $file = Join-Path $cli.Root 'VERSION'
        Write-Host $(if (Test-Path $file) { (Get-Content $file -Raw).Trim() } else { 'dev' })
    }
}
