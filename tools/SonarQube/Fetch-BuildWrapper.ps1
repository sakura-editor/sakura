# Fetch-BuildWrapper.ps1
Param(
    [String]$Destination = [System.IO.Path]::Combine($PSScriptRoot, "..\..\.sonar")
)

# 宛先フォルダが存在しなければ作成する
if (-not(Test-Path $Destination)) {
    New-Item -Path $Destination -ItemType Directory
}

# BuildWrapperが存在しなければダウンロードする
if (-not(Test-Path "$Destination\build-wrapper-win-x86-64.exe")) {
    # BuildWrapperをダウンロードする
    if (-not(Test-Path "$Destination\build-wrapper-win-x86.zip")) {
        $sonarHostUrl = "https://sonarcloud.io"

        $SonarScannerProperties = "$Destination\scanner\conf\sonar-scanner.properties"
        if (Test-Path $SonarScannerProperties) {
            $hostUrlLine = Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=.+"
            if ($hostUrlLine -and ($hostUrlLine -match "sonar.host.url=(.+)")) {
                $sonarHostUrl = $matches[1]
            }
        }
        Invoke-WebRequest -OutFile "$Destination\build-wrapper-win-x86.zip" "$sonarHostUrl/static/cpp/build-wrapper-win-x86.zip"
    }

    # BuildWrapperを展開する
    if (-not(Test-Path "$Destination\build-wrapper-win-x86-64.exe")) {
        # zipを展開する
        $extractedRoot = Join-Path $Destination "build-wrapper-win-x86"
        Expand-Archive -Path "$Destination\build-wrapper-win-x86.zip" -DestinationPath $Destination -Force

        $extractedExe = Join-Path $extractedRoot "build-wrapper-win-x86-64.exe"
        if (-not(Test-Path $extractedExe)) {
            throw "build-wrapper-win-x86-64.exe was not found in archive."
        }

        Move-Item -Path $extractedExe -Destination "$Destination\build-wrapper-win-x86-64.exe" -Force
        Remove-Item -Path $extractedRoot -Recurse -Force
    }
}
