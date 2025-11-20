# Fetch-SonarScanner.ps1
Param(
    [String]$Version = $(Get-Content -Path "$PSScriptRoot\SCANNER_CLI_VERSION"),
    [String]$Destination = [System.IO.Path]::Combine($PSScriptRoot, "..\..\.sonar")
)

# 宛先フォルダが存在しなければ作成する
if (-not(Test-Path $Destination)) {
    New-Item -Path $Destination -ItemType Directory
}

# SonarScannerCLIが存在しなければダウンロードする
if (-not(Test-Path "$Destination\scanner\bin\sonar-scanner")) {
    # JAVA_HOMEのJavaバージョンを確認する
    $javaVersion = 0
    if ($(cmd.exe /C "`"$env:JAVA_HOME\bin\java.exe`" -version" | select-object -first 1) -match '^openjdk version "(\d+).*') {
        $javaVersion = [Int32]$matches[1]
    }

    # ダウンロードするJREの種類を判定する
    $jreSurfix = "-windows-x64"
    if ([String]::IsNullOrEmpty($env:JAVA_HOME) -or $javaVersion -lt 17) {
        $jreSurfix = ""
    }

    # SonarScannerCLIのファイル名を組み立てる
    $sonarScannerFileName = "sonar-scanner-$Version$jreSurfix"

    # SonarScannerCLIをダウンロードする
    if (-not(Test-Path "$Destination\$sonarScannerFileName.zip")) {
        $sonarScannerDistUrl = "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli"
        Invoke-WebRequest -OutFile "$Destination\$sonarScannerFileName.zip" "$sonarScannerDistUrl/sonar-scanner-cli-$Version$jreSurfix.zip"
    }

    # SonarScannerCLIを展開する
    if (-not(Test-Path "$Destination\scanner")) {
        # zipを展開する
        7z x "$Destination\$sonarScannerFileName.zip" "-o$Destination" "*"

        # 解凍したフォルダ名からバージョンを取り除く
        Move-Item -Path "$Destination\$sonarScannerFileName" -Destination "$Destination\scanner"
    }
}
