Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# Invoke Tests1.
& "$PSScriptRoot\Run-OpenCppCoverage.ps1" `
    "tests1-coverage.xml" `
    "$HomePath\$Platform\$Configuration\tests1.exe" `
    @("--gtest_output=xml:$HomePath\tests1-googletest.xml")
