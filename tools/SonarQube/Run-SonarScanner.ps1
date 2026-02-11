# Run-SonarScanner.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

# SONAR_TOKEN未定義の場合、ファイルから取得を試みる
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    $env:SONAR_TOKEN = "$(Get-Content $HomePath\SONAR_TOKEN)"
}

# それでもSONAR_TOKEN未定義の場合、異常終了する
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

if ([String]::IsNullOrEmpty($env:CMD_GIT)) {
    $env:CMD_GIT = cmd.exe /C 'where.exe "%PATH%;%ProgramFiles%\Git\Cmd:git.exe" 2>&1' | select-object -first 1
}

$remote = & $env:CMD_GIT "remote" "get-url" "origin"

if ($remote -match "git@github\.com:(.+)\.git") {
    $organization = $matches[1] -replace '(.+)/.+', '$1'
    $projectKey = $matches[1] -replace '/', '_'

} else {
    throw $remote
    Throw "Remote URL is not a GitHub repository.($remote)"
}

# Fetch the sonar-scanner.
. "$PSScriptRoot\Fetch-SonarScanner.ps1"

$sonarScannerArgs = @(
    "-D`"sonar.organization=$organization`"",
    "-D`"sonar.projectKey=$projectKey`"")

$SonarScannerProperties = "$HomePath\.sonar\scanner\conf\sonar-scanner.properties"

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=.+").Matches.Success)) {
    $sonarScannerArgs += @(
        "-D`"sonar.host.url=https://sonarcloud.io`""
    )
}

$sonarScannerArgs += @(
    "-D`"sonar.cfamily.compile-commands=build/$Platform/$Configuration/bw-output/compile_commands.json`""
)

$productId = vswhere -property productId -version "[$VsVersion,$([int]$VsVersion + 1))"

$useOpenCppCoverage = ($env:GITHUB_ACTIONS -eq 'true') -or (-not ($productId -match "Enterprise$"))

if (-not($useOpenCppCoverage)) {
  $sonarScannerArgs += @(
    "-D`"sonar.cfamily.vscoveragexml.reportsPath=TestResults/*/*.xml`""
  )

} elseif (Test-Path -Path "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe") {
  $sonarScannerArgs += @(
    "-D`"sonar.cfamily.cppunit.reportPath=*-googletest.xml`"",
    "-D`"sonar.cfamily.cobertura.reportPaths=tests1-coverage.xml`""
  )
}

$branch = & $env:CMD_GIT @("branch", "--show-current")

if (-not([String]::IsNullOrEmpty($branch))) {
  $sonarScannerArgs += @(
    "-D`"sonar.branch.name=$branch`""
  )
}

if ([string]::IsNullOrEmpty($env:LC_ALL)) {
  $env:LC_ALL = "ja_JP.UTF-8"
}

Write-Host "SonarScanner Arguments:"
$sonarScannerArgs | ForEach-Object { Write-Host $_ }

# Run SonarScanner.
$p = Start-Process `
  -FilePath $HomePath\.sonar\scanner\bin\sonar-scanner.bat `
  -ArgumentList $sonarScannerArgs `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "SonarScanner was Failed."
}

# SonarSourceはsonarscanner-cliのDockerイメージも提供している。
# これを使うとローカル環境に依存せずSonarScannerを実行できるはずだがC/C++には未対応らしい。

# docker run `
#     --rm `
#     -e SONAR_HOST_URL="$env:SONAR_HOST_URL"  `
#     -e SONAR_TOKEN="$env:SONAR_TOKEN" `
#     -v "$($HomePath):/usr/src" `
#     sonarsource/sonar-scanner-cli `
#     -D"sonar.organization=berryzplus" `
#     -D"sonar.projectKey=berryzplus_sakura-editor" `
#     -D"sonar.branch.name=work"
# 
# if ($LASTEXITCODE -ne 0) {
#     throw "SonarScanner was Failed."
# }
