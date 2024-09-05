Param(
    [String]$coverage,
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")
)

# INIファイルの格納パス
$ProfilePath = "$HomePath\$Platform\$Configuration"

# サクラエディタのパス
$SakuraPath = "$ProfilePath\sakura.exe"

function Initialize-Test-Data
{
}

# サクラエディタを起動して終了する
function Invoke-SakuraEditorForExit
{
    # サクラエディタの全終了をマクロ経由で呼び出す
    Start-Process `
        -FilePath $SakuraPath `
        -ArgumentList @( `
            "-MTYPE=js", `
            "-M=ExitAll();") `
        -WorkingDirectory $HomePath

    # かなり時間かかるので長めに待つ
    Start-Sleep -Seconds 5
}

# サクラエディタをテスト用に起動する
function Invoke-SakuraEditorForFuncTest
{
    # Invoke sakura-editor with OpenCppCoverage.
    Start-Process `
        -FilePath "powershell.exe" `
        -ArgumentList @("$PSScriptRoot\Run-OpenCppCoverage.ps1", $coverage, $SakuraPath) `
        -NoNewWindow `
        -WorkingDirectory $HomePath `
        -PassThru

    # かなり時間かかるので長めに待つ
    Start-Sleep -Seconds 10
}

# winget install CoreyButler.NVMforWindows

npm install --location=global appium
appium setup desktop
node -v
npm -v
appium -v

# サクラエディタの全終了をマクロ経由で呼び出す
Invoke-SakuraEditorForExit

# # テストデータを作成する
# Initialize-Test-Data

# サクラエディタをテスト用に起動する
Invoke-SakuraEditorForFuncTest

# Run tests3.
$p = Start-Process `
    -FilePath "py.exe" `
    -ArgumentList @("-m", "pytest", "-v", "-s") `
    -NoNewWindow `
    -WorkingDirectory "$HomePath\tests\functests" `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "tests3 was Failed."
}

# サクラエディタの全終了をマクロ経由で呼び出す
Invoke-SakuraEditorForExit
