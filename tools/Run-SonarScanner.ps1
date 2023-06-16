Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

$SonarScannerProperties = "$HomePath\.sonar\scanner\conf\sonar-scanner.properties"

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.organization=.+").Matches.Success)) {
    Throw "Missing 'sonar.organization' in $SonarScannerProperties."
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.projectKey=.+").Matches.Success)) {
    Throw "Missing 'sonar.projectKey' in $SonarScannerProperties."
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=.+").Matches.Success)) {
    Throw "Missing 'sonar.host.url' in $SonarScannerProperties."
}

# Fetch the sonar-scanner.
.\tools\Fetch-SonarScanner.ps1

# Check SONAR_TOKEN
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Run SonarScanner.
$p = Start-Process `
    -FilePath .sonar\scanner\bin\sonar-scanner.bat `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "SonarScanner was Failed."
}
