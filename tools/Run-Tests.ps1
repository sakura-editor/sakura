Param(
    [String]$Platform = $env:BUILD_PLATFORM,
    [String]$Configuration = $env:BUILD_CONFIGURATION
)

$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

# Invoke Tests1.
& "$PSScriptRoot\Run-OpenCppCoverage.ps1" `
  $([System.IO.Path]::GetFullPath("$HomePath\tests1-coverage.xml")) `
  $([System.IO.Path]::GetFullPath("$HomePath\$Platform\$Configuration\tests1.exe")) `
  @("--gtest_output=xml:tests1-googletest.xml")

# Invoke Tests2.
& "$PSScriptRoot\Run-SakuraEditorWithCoverage.ps1" `
  $([System.IO.Path]::GetFullPath("$HomePath\tests2-coverage.xml")) `
  $Platform `
  $Configuration
