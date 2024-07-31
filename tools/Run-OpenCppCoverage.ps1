Param(
    [String]$coverage,
    [String]$command,
    [String]$commandArgs
)

Write-Host "coverage was ${coverage}."
Write-Host "command was ${command}."
Write-Host "commandArgs was ${commandArgs}."

$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

$command = [System.IO.Path]::GetFullPath($command)

$openCppCoverageArgs = @( `
  "--export_type xml:$coverage", `
  "--modules $command", `
  "--sources $HomePath", `
  "--excluded_sources $HomePath\build", `
  "--working_dir $HomePath", `
  "--cover_children", `
  "--", `
  $command)

$openCppCoverageArgs += $commandArgs -split "`r`n"

# Invoke command with OpenCppCoverage.
$p = Start-Process `
    -FilePath "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" `
    -ArgumentList $openCppCoverageArgs `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "$(Split-Path -Path $command -Leaf) was Failed."
}
