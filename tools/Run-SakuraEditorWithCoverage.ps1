Param(
    [String]$coverage,
    [String]$Platform = "x64",
    [String]$Configuration = "Debug"
)

#起動時マクロ
$startupMacro = (@"
// Sakura Editor のマクロ
//
// ・"//"に続く部分はコメントです。
// ・空行は無視されます。

Down();
Up();
Right();
Left();

Outline(0);     // アウトライン解析
ShowFunckey();  // ShowFunckey 出す
ShowMiniMap();  // ShowMiniMap 出す
ShowTab();      // ShowTab 出す
SelectAll();    // 全選択
GoFileEnd();    // ファイルの最後に移動
GoFileTop();    // ファイルの先頭に移動
ShowFunckey();  // ShowFunckey 消す
ShowMiniMap();  // ShowMiniMap 消す
ShowTab();      // ShowTab 消す

ExpandParameter('`$I');     // INIファイルパスの取得(呼ぶだけ)

// フォントサイズ設定のテスト(ここから)
SetFontSize(0, 1, 0);       // 相対指定 - 拡大 - 対象：共通設定
SetFontSize(0, -1, 0);      // 相対指定 - 縮小 - 対象：共通設定
SetFontSize(100, 0, 0);     // 直接指定 - 対象：共通設定
SetFontSize(100, 0, 1);     // 直接指定 - 対象：タイプ別設定
SetFontSize(100, 0, 2);     // 直接指定 - 対象：一時適用
SetFontSize(100, 0, 3);     // 直接指定 - 対象が不正
SetFontSize(0, 0, 0);       // 直接指定 - フォントサイズ下限未満
SetFontSize(9999, 0, 0);    // 直接指定 - フォントサイズ上限超過
SetFontSize(0, 0, 2);       // 相対指定 - サイズ変化なし
SetFontSize(0, 1, 2);       // 相対指定 - 拡大
SetFontSize(0, -1, 2);      // 相対指定 - 縮小
SetFontSize(0, 9999, 2);    // 相対指定 - 限界まで拡大
SetFontSize(0, 1, 2);       // 相対指定 - これ以上拡大できない
SetFontSize(0, -9999, 2);   // 相対指定 - 限界まで縮小
SetFontSize(0, -1, 2);	    // 相対指定 - これ以上縮小できない
SetFontSize(100, 0, 2);	    // 元に戻す
// フォントサイズ設定のテスト(ここまで)

Outline(2);     // アウトライン解析を閉じる

ExitAll();
"@ -split "`r`n" | ForEach-Object { $_ -replace "(;?)\s*//.*$", '$1' } | Where-Object { $_ -ne "" }) -join ""

$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

$CmdExe = [System.IO.Path]::GetFullPath("$HomePath\$Platform\$Configuration\sakura.exe")

$CmdArgs = @"
${HomePath}\tests\unittests\test-winmain.cpp
-MTYPE=`"js`"
-M=`"${startupMacro}`"
"@

# Invoke sakura-editor with OpenCppCoverage.
& "$PSScriptRoot\Run-OpenCppCoverage.ps1" $coverage $CmdExe $CmdArgs
