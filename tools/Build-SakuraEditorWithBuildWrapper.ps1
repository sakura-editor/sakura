Param(
    [String]$VsVersion = "2019",
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

switch ($VsVersion)
{
    "2022" { $VsVersion = "17" }
    "2019" { $VsVersion = "16" }
    "2017" { $VsVersion = "15" }
}
$verFilter = "[$VsVersion,$([int]$VsVersion + 1)`)"

$CMD_MSBUILD = $(vswhere -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe -version $verFilter)

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
