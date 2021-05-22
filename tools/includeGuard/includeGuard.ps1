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
$headers = Get-ChildItem -File -Recurse -Include *.h -Exclude StdAfx.h,sakura_rc.h,String_define.h
foreach($header in $headers){
  $lines = Get-Content -Path $header
  $filepart = [System.IO.Path]::GetFileNameWithoutExtension($header.Name).ToUpper() -replace "-","_"
  $guardPattern = $lines | Select-String -Pattern "SAKURA_${filepart}_\w+_H_"
  $match = $lines | Select-String -Pattern "#pragma +once"
  if (($match -ne $null) -And ($guardPattern -eq $null)) {
    $guidpart = (New-Guid).Guid.ToUpper() -replace "-","_"
    $guardkey = "SAKURA_${filepart}_${guidpart}_H_"
    $pragmaOnce = $match.LineNumber - 1
    $lines[0..($pragmaOnce - 1)] | Out-File $header -Encoding UTF8
    "#ifndef ${guardkey}" | Out-File $header -Encoding UTF8 -Append
    "#define ${guardkey}" | Out-File $header -Encoding UTF8 -Append
    $lines[$pragmaOnce..($lines.Length - 1)] | Out-File $header -Encoding UTF8 -Append
    "#endif /* ${guardkey} */" | Out-File $header -Encoding UTF8 -Append
  } elseif ($guardPattern -eq $null) {
    Write-Output $header.FullName
  }
}
