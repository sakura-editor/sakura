Param(
    [String]$VsVersion = $env:NUM_VSVERSION,
    [String]$Platform = $env:BUILD_PLATFORM,
    [String]$Configuration = $env:BUILD_CONFIGURATION,
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

cmd.exe /C ".\tools\BuildDependencies.bat"

$CMD_MSBUILD = $(vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1)`)")

./Tools/Fetch-BuildWrapper.ps1

$p = Start-Process `
    -FilePath .sonar\build-wrapper\build-wrapper-win-x86-64.exe `
    -ArgumentList @("--out-dir bw-output", $CMD_MSBUILD, "/p:Platform=$Platform", "/p:Configuration=$Configuration", "/t:ReBuild") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
