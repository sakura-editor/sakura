Param(
    [String]$VsVersion = "17",
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

cmd.exe /C ".\tools\BuildDependencies.bat"

$CMD_MSBUILD = $(vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1)`)")

./Tools/Fetch-BuildWrapper.ps1

$p = Start-Process `
    -FilePath .sonar\build-wrapper\build-wrapper-win-x86-64.exe `
    -ArgumentList @("--out-dir bw-output", $CMD_MSBUILD, "/p:Platform=$Platform", "/p:Configuration=$Configuration", "/t:ReBuild", "/flp:logfile=msbuild-$Platform-$Configuration.log") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
