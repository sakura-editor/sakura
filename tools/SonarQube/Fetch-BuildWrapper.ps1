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
        7z e "$Destination\build-wrapper-win-x86.zip" "-o$Destination" "build-wrapper-win-x86\build-wrapper-win-x86-64.exe"
    }
}
