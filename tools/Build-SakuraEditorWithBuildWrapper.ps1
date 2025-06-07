Param(
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

$CMD_MSBUILD = $(vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1)`)")

$buildWrapperPath = "build-wrapper-win-x86-64"

where.exe build-wrapper-win-x86-64

if ($LASTEXITCODE -ne 0) {
    ./Tools/Fetch-BuildWrapper.ps1
    $buildWrapperPath = ".sonar\build-wrapper\build-wrapper-win-x86-64.exe"
}

$p = Start-Process `
    -FilePath $buildWrapperPath `
    -ArgumentList @("--out-dir bw-output", $CMD_MSBUILD, "/p:Platform=$Platform", "/p:Configuration=$Configuration", "/t:ReBuild", "/flp:logfile=msbuild-$Platform-$Configuration.log") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
