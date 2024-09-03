Param(
    [String]$VsVersion = $env:NUM_VSVERSION,
    [String]$Platform = $env:BUILD_PLATFORM,
    [String]$Configuration = $env:BUILD_CONFIGURATION,
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

$SonarScannerProperties = "$HomePath\.sonar\scanner\conf\sonar-scanner.properties"

$hostUrlMatcher = (Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=(.+)").Matches

if (-not($hostUrlMatcher.Success) -or $hostUrlMatcher.Groups[1].Value -eq "https://sonarcloud.io") {
  .\tools\Build-SakuraEditorWithBuildWrapper.ps1 $VsVersion $Platform $Configuration $HomePath
  exit 0
}

cmd.exe /C ".\tools\BuildDependencies.bat"

$CMD_MSBUILD = $(vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1)`)")

$p = Start-Process `
    -FilePath $CMD_MSBUILD `
    -ArgumentList @("/p:Platform=$Platform", "/p:Configuration=$Configuration", "/t:ReBuild") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
