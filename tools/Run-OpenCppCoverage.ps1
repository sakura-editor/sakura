Param(
    [String]$coverage,
    [String]$command,
    [String]$commandArgs = "`r`n"
)

$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

$command = [System.IO.Path]::GetFullPath($command)

$openCppCoverageArgs = @( `
  "--export_type xml:$HomePath\$coverage", `
  "--modules $command", `
  "--sources $HomePath", `
  "--excluded_sources $HomePath\build", `
  "--working_dir $([System.IO.Path]::GetDirectoryName($command))", `
  "--cover_children", `
  "--", `
  $command)

$openCppCoverageArgs += $commandArgs -split "`r`n" | Where-Object { $_ -ne '' }

# Invoke command with OpenCppCoverage.
$p = Start-Process `
    -FilePath "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" `
    -ArgumentList $openCppCoverageArgs `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

# if ($p.ExitCode -ne 0) {
#   throw "$(Split-Path -Path $command -Leaf) was Failed."
# }
