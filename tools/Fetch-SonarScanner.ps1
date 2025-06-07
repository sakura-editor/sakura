Param(
    [String]$Version = $(Get-Content -Path "$PSScriptRoot\SCANNER_CLI_VERSION"),
    [String]$Destination = [System.IO.Path]::Combine($PSScriptRoot, "..\.sonar")
)
if (-not(Test-Path $Destination)) {
    New-Item -Path $Destination -ItemType Directory
}
if (-not(Test-Path "$Destination\scanner\bin")) {
    Push-Location $Destination
    if (-not(Test-Path "$Destination\sonar-scanner.zip")) {
        Invoke-WebRequest -OutFile sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-$Version.zip
        7z rn sonar-scanner.zip sonar-scanner-$Version scanner
    }
    7z x sonar-scanner.zip
    Pop-Location
}
