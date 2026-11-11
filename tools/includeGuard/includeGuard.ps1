<#
.SYNOPSIS
インクルードガード付加マクロ(powershell版)

.DESCRIPTION
ヘッダーファイルを検索してインクルードガードを付加します。

過去のjsマクロをps1化したpowershellスクリプトです。
https://sakura-editor.github.io/bbslog/sf/unicode/985.html

このマクロに引数はありません。
ファイル内の #pragma once を検索して見つかった位置にインクルードガードを挿入します。

.EXAMPLE
PS C:\work\sakura-editor\sakura\sakura_core> ..\tools\includeGuard\includeGuard.ps1

カレントディレクトリ配下のヘッダーファイルを検索してインクルードガードを付加します。
検索対象は 拡張子.h のファイルで、StdAfx.h,sakura_rc.h,String_define.hは除外されます。

.LINK
https://github.com/sakura-editor/sakura/pull/1378
https://sakura-editor.github.io/bbslog/sf/unicode/985.html
#>
$headers = Get-ChildItem -File -Recurse -Include "*.h","*.hpp" -Exclude "StdAfx.h","sakura_rc.h","String_define.h"
$utf8Bom = [System.Text.UTF8Encoding]::new($true)
foreach($header in $headers){
  $lines = Get-Content -Path $header
  $filepart = [System.IO.Path]::GetFileNameWithoutExtension($header.Name).ToUpper() -replace "-","_"
  $guardPattern = $lines | Select-String -Pattern "SAKURA_${filepart}_\w+_H_"
  $match = $lines | Select-String -Pattern "#pragma +once"
  if (($null -ne $match) -And ($null -eq $guardPattern)) {
    $guidpart = (New-Guid).Guid.ToUpper() -replace "-","_"
    $guardkey = "SAKURA_${filepart}_${guidpart}_H_"
    $pragmaOnce = $match.LineNumber - 1
    $outputLines = [System.Collections.Generic.List[string]]::new()
    if ($pragmaOnce -gt 0) {
      $outputLines.AddRange([string[]]$lines[0..($pragmaOnce - 1)])
    }
    $outputLines.Add("#ifndef ${guardkey}")
    $outputLines.Add("#define ${guardkey}")
    $outputLines.AddRange([string[]]$lines[$pragmaOnce..($lines.Length - 1)])
    $outputLines.Add("#endif /* ${guardkey} */")
    $content = ($outputLines -join "`r`n") + "`r`n"
    [System.IO.File]::WriteAllText($header.FullName, $content, $utf8Bom)
  } elseif ($null -eq $guardPattern) {
    Write-Output $header.FullName
  }
}
