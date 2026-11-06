& {
    $ErrorActionPreference = 'Stop'
    $ProgressPreference = 'SilentlyContinue'
    if ([Environment]::OSVersion.Version.Build -lt 22000) { throw 'WinTBUtils requires Windows 11.' }
    if ($env:PROCESSOR_ARCHITECTURE -ne 'AMD64') { throw 'WinTBUtils requires x64 Windows.' }
    $source = if ($env:WINTBUTILS_ZIP) { $env:WINTBUTILS_ZIP } else { 'https://github.com/SilentDemonSD/WinTBUtils/releases/latest/download/WinTBUtils.zip' }
    $work = Join-Path $env:TEMP "WinTBUtils-$([guid]::NewGuid())"
    New-Item $work -ItemType Directory | Out-Null
    try {
        $zip = Join-Path $work 'WinTBUtils.zip'
        $payload = Join-Path $work 'payload'
        Invoke-WebRequest $source -OutFile $zip -UseBasicParsing
        Expand-Archive $zip $payload
        Invoke-Expression (Get-Content (Join-Path $payload 'cli\lib\Installation.ps1') -Raw)
        [Installation]::new().Install($payload)
    } finally {
        Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue
    }
}
