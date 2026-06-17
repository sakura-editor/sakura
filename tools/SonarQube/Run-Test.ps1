# Run-Test.ps1
Param(
  [String]$testCommand,
  [bool]$useOpenCppCoverage,
  [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

$testName = $([System.IO.Path]::GetFileNameWithoutExtension($testCommand))

if (-not($useOpenCppCoverage)) {
  $VsInstallationPath = vswhere -property installationPath -version "[$VsVersion,$([int]$VsVersion + 1))"
  $VsExtentions = "$VsInstallationPath\Common7\IDE\Extensions"

  Write-Host "`$VsExtentions is '$VsExtentions'"

  # GoogleTestAdapterのパスを取得する
  $testAdapterPath = (Get-ChildItem $VsExtentions -Filter GoogleTestAdapter.TestAdapter.dll -Recurse).DirectoryName

  Write-Host "`$testAdapterPath is '$testAdapterPath'"

  # データコレクターのパスを取得する
  $DataCollectorPath = (Get-ChildItem $VsExtentions -Filter Microsoft.VisualStudio.TraceDataCollector.dll -Recurse).DirectoryName

  Write-Host "`$DataCollectorPath is '$DataCollectorPath'"

  $vstest = "$VsInstallationPath\Common7\IDE\Extensions\TestPlatform\vstest.console.exe"

  # VSTestを実行する(一旦trx形式で出す)
  & $vstest @(
    $testCommand,
    "/EnableCodeCoverage",
    "/Collect:`"Code Coverage;Format=Xml`"",
    "/ResultsDirectory:TestResults"
    "/TestAdapterPath:`"$testAdapterPath;$DataCollectorPath`"",
    "/Logger:trx;LogFileName=$testName-vstest.trx"
  )

  # trx2junitのインストールチェック
  cmd.exe /c "where.exe trx2junit >NUL"
  if ($LASTEXITCODE -ne 0) {
      dotnet tool install --global trx2junit
  }

  # trx2junitを実行する
  trx2junit "$HomePath\TestResults\$testName-vstest.trx"
  Copy-Item -Path "$HomePath\TestResults\$testName-vstest.xml" -Destination "$HomePath\$testName-googletest.xml"

  # xmlファイルは消しておく
  Remove-Item -Path "$HomePath\TestResults\$testName-vstest.xml"

  return 0
}

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
