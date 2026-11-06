$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'lib\TaskbarClear.ps1')

class Cli {
    [string]$Root
    [object]$Taskbar
    [string[]]$Arguments
    hidden [string]$CommandsDir

    Cli([string]$root, [object]$taskbar, [string[]]$arguments) {
        $this.Root = $root
        $this.Taskbar = $taskbar
        $this.Arguments = $arguments
        $this.CommandsDir = Join-Path $root 'cli\commands'
    }

    [object] Find([string]$name) {
        $file = Join-Path $this.CommandsDir "$name.ps1"
        if (-not (Test-Path -LiteralPath $file)) { return $null }
        return & $file
    }

    [string[]] Names() {
        return (Get-ChildItem $this.CommandsDir -Filter '*.ps1').BaseName
    }
}

$root = Split-Path $PSScriptRoot
$cli = [Cli]::new($root, [TaskbarClear]::new((Join-Path $root 'bin\TaskbarClear.dll')), @($args | Select-Object -Skip 1))
$name = if ($args.Count) { "$($args[0])".TrimStart('-', '/').ToLowerInvariant() } else { 'help' }
$command = $cli.Find($name)

if (-not $command) {
    [Console]::Error.WriteLine("tbutil: unknown command '$name'")
    & $cli.Find('help').Run $cli
    exit 2
}

try {
    & $command.Run $cli
} catch {
    [Console]::Error.WriteLine("tbutil: $($_.Exception.Message)")
    exit 1
}
