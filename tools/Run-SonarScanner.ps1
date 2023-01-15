Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# Fetch the sonar-scanner.
.\tools\Fetch-SonarScanner.ps1 "$(Get-Content $HomePath\tools\SCANNER_CLI_VERSION)" "$HomePath\.sonar"

$SonarScannerProperties = "$HomePath\.sonar\scanner\conf\sonar-scanner.properties"

$RemoteBranchName = $((git remote show "origin" | Select-String 'Fetch URL: https://github.com/') -replace '^ *Fetch URL: https://github.com/', '' -replace '\.git$', '')
$GitHubOrganization = $($RemoteBranchName.Split('/')[0])
$GitHubProjectName = $($RemoteBranchName.Split('/')[1])

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.organization=").Matches.Success)) {
    Add-Content -Value "sonar.organization=$GitHubOrganization" -Encoding utf8 -Path $SonarScannerProperties
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.projectKey=").Matches.Success)) {
    Add-Content -Value "sonar.projectKey=$GitHubOrganization_$GitHubProjectName" -Encoding utf8 -Path $SonarScannerProperties
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=").Matches.Success)) {
    Add-Content -Value "sonar.host.url=https://sonarcloud.io" -Encoding utf8 -Path $SonarScannerProperties
}

# Check SONAR_TOKEN
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Run SonarScanner.
$p = Start-Process `
    -FilePath .sonar\scanner\bin\sonar-scanner.bat `
    -ArgumentList @( `
        "-D`"sonar.cfamily.threads=${env:NUMBER_OF_PROCESSORS}`"") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "SonarScanner was Failed."
}
