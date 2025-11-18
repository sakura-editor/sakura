# Analyze-SakuraEditor.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

# SONAR_TOKEN未定義の場合、ファイルから取得を試みる
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    $env:SONAR_TOKEN = "$(Get-Content $HomePath\SONAR_TOKEN)"
}

# それでもSONAR_TOKEN未定義の場合、異常終了する
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Re-Build project.
. "$PSScriptRoot\Build-Project.ps1" $Platform $Configuration $VsVersion $HomePath

# Run Tests.
. "$PSScriptRoot\Run-Tests.ps1" $Platform $Configuration $VsVersion $HomePath

# Run SonarScanner.
. "$PSScriptRoot\Run-SonarScanner.ps1" $Platform $Configuration $VsVersion $HomePath
