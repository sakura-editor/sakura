Param(
    [String]$VsVersion = "17",
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# Fetch the sonar-scanner.
.\tools\Fetch-SonarScanner.ps1

# SONAR_TOKEN未定義の場合、ファイルから取得を試みる
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    $env:SONAR_TOKEN = "$(Get-Content $HomePath\tools\SONAR_TOKEN)"
}

# Check SONAR_TOKEN
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Re-Build project.
.\tools\Build-SakuraEditor.ps1 $VsVersion $Platform $Configuration

# Run Tests.
.\tools\Run-Tests.ps1 $Platform $Configuration $HomePath

# Run SonarScanner.
.\tools\Run-SonarScanner.ps1 $Platform, $Configuration
