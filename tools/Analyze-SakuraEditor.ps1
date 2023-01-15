Param(
    [String]$VsVersion = "2019",
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# SONAR_TOKEN未定義の場合、ファイルから取得を試みる
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    $env:SONAR_TOKEN = "$(Get-Content $HomePath\SONAR_TOKEN)"
}

# Check SONAR_TOKEN
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Re-Build project with Build-Wrapper.
.\tools\Build-SakuraEditorWithBuildWrapper.ps1 $VsVersion $Platform $Configuration $HomePath

# Run Tests.
.\tools\Run-Tests.ps1 tests1 $Platform $Configuration $HomePath

# Run SonarScanner.
.\tools\Run-SonarScanner.ps1 $Platform, $Configuration, $HomePath
