# Build-Project.ps1
Param(
  [String]$Platform = "x64",
  [String]$Configuration = "Debug",
  [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
  [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

# build-wrapperがパス内にあるかチェックする
$buildWrapperPath = "build-wrapper-win-x86-64.exe"

cmd.exe /C "where.exe $buildWrapperPath >NUL 2>&1"
if ($LASTEXITCODE -ne 0) {
  . "$PSScriptRoot/Fetch-BuildWrapper.ps1"
  $buildWrapperPath = "$HomePath\.sonar\build-wrapper-win-x86-64.exe"
}

$msBuild = vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1))" -requires Microsoft.Component.MSBuild

$msBuildArgs = @(
  "--out-dir build\$Platform\$Configuration\bw-output",
  "$msBuild",
  "/p:Platform=$Platform",
  "/p:Configuration=$Configuration",
  "/t:ReBuild",
  "/flp:logfile=msbuild-$Platform-$Configuration.log",
  "/clp:ForceNoAlign",
  "/verbosity:normal"
)

if (-not(Test-Path "$HomePath\build\$Platform\$Configuration")) {
  New-Item -Path "$HomePath\build\$Platform\$Configuration" -ItemType Directory
}

# build-wrapperでプロジェクトをリビルドする
$p = Start-Process `
  -FilePath $buildWrapperPath `
  -ArgumentList $msBuildArgs `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
