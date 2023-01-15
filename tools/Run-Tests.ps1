Param(
    [String]$TestName = "tests1",
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# Resolve Path to execute.
$CmdTests = [System.IO.Path]::Combine($HomePath, "$Platform\$Configuration\$TestName.exe")

# Run Test.
$p = Start-Process `
    -FilePath "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" `
    -ArgumentList @( `
      "--export_type xml:$HomePath\$TestName-coverage.xml", `
      "--modules $CmdTests", `
      "--sources $HomePath", `
      "--excluded_sources $HomePath\tests\googletest", `
      "--working_dir $HomePath\$Platform\$Configuration", `
      "--cover_children", `
      "--", `
      "$CmdTests", `
      "--gtest_output=xml:$HomePath\$TestName-googletest.xml") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "$TestName was Failed."
}
