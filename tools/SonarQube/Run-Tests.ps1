# Run-Tests.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

$ctest = @( (Get-Command ctest.exe -ErrorAction Ignore).Source, $(vswhere -find 'Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\ctest.exe' -version "[$VsVersion,$([int]$VsVersion + 1))") ) | Where-Object { $_ } | Select-Object -First 1
if ([String]::IsNullOrWhiteSpace($ctest)) {
    throw "ctest.exe was not found."
}

$p = Start-Process `
  -FilePath $ctest `
  -ArgumentList @(
    "--test-dir", "build/$Platform/CMakeTools",
    "-C", $Configuration,
    "--output-on-failure",
    "-V"
  ) `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
