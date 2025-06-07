Param(
    [String]$Destination = [System.IO.Path]::Combine($PSScriptRoot, "..\.sonar")
)
if (-not(Test-Path $Destination)) {
    New-Item -Path $Destination -ItemType Directory
}
if (-not(Test-Path "$Destination\build-wrapper\build-wrapper-win-x86-64.exe")) {
    Push-Location $Destination
    if (-not(Test-Path "$Destination\build-wrapper-win-x86.zip")) {
        Invoke-WebRequest -OutFile build-wrapper-win-x86.zip https://sonarcloud.io/static/cpp/build-wrapper-win-x86.zip
        7z rn build-wrapper-win-x86.zip build-wrapper-win-x86 build-wrapper
    }
    7z x build-wrapper-win-x86.zip
    Pop-Location
}
