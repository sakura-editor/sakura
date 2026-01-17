# Run-Test.ps1
Param(
  [String]$testCommand,
  [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

$testName = $([System.IO.Path]::GetFileNameWithoutExtension($testCommand))

# テストコマンドをフルパスにする
$testCommand = [System.IO.Path]::GetFullPath($testCommand)

# OpenCppCoverageの引数配列を作成する
$openCppCoverageArgs = @(
  "--export_type cobertura:$HomePath\$testName-coverage.xml",
  "--modules $testCommand",
  "--sources $HomePath",
  "--excluded_sources $HomePath\build",
  "--working_dir $([System.IO.Path]::GetDirectoryName($testCommand))",
  "--cover_children",
  "--",
  $testCommand,
  "--gtest_output=xml:$testName-googletest.xml"
)

# Invoke command with OpenCppCoverage.
$p = Start-Process `
  -FilePath "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" `
  -ArgumentList $openCppCoverageArgs `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "$(Split-Path -Path $testCommand -Leaf) was Failed."
}
