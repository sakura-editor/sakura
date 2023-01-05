param($SRC_HELP, $TMP_HELP, $HH_INPUT, $HH_OUTPUT, $CMD_HHC)

#remove comment
(Get-Content -LiteralPath $HH_INPUT -Encoding UTF8) -replace '//.*' |
 Set-Content -LiteralPath $HH_OUTPUT -Encoding UTF8

Add-Type -TypeDefinition (Get-Content -LiteralPath "$SRC_HELP\EncoderEscapingFallback.cs" -Raw) -Language CSharp
$sjis=[Text.Encoding]::GetEncoding("shift_jis",[EncoderEscapingFallback]::new("&#{0};"),[Text.DecoderFallback]::ExceptionFallback)
$re=[Regex]::new('(?<=<META http-equiv="Content-Type" content="text/html; charset=|@charset ")UTF-8',"IgnoreCase")

#create a folder tree
md $TMP_HELP >$null
dir -LiteralPath $SRC_HELP -Recurse -Directory |%{md ($_.fullname -replace ($SRC_HELP -replace "[[\]\\]",'\$&'), $TMP_HELP) >$null}

#convert or copy
dir -LiteralPath $SRC_HELP -Recurse -File |
	%{ 
		if($_.name -match "\.(?:html|css|js)$"){
			$utf8=Get-Content -LiteralPath $_.fullname -Encoding UTF8 -Raw
			$string=$re.Replace($utf8, 'Shift_JIS', 1)
			$bytes=$sjis.GetBytes($string, 0, $string.Length)
			[IO.File]::WriteAllBytes(($_.fullname -replace ($SRC_HELP -replace "[[\]\\]",'\$&'), $TMP_HELP), $bytes)
		}else{
			Copy-Item -LiteralPath $_.fullname ($_.fullname -replace ($SRC_HELP -replace "[[\]\\]",'\$&'), $TMP_HELP)
		}
	}

#"コピー元のファイル: $((dir -LiteralPath $SRC_HELP -Recurse -File|measure).count)個"
#"コピー先のファイル: $((dir -LiteralPath $TMP_HELP -Recurse -File|measure).count)個"


#compile
&"$SRC_HELP\CompileChm.ps1" "$TMP_HELP\macro\macro.HHP" "$SRC_HELP\macro\macro.chm"
&"$SRC_HELP\CompileChm.ps1" "$TMP_HELP\plugin\plugin.HHP" "$SRC_HELP\plugin\plugin.chm"
&"$SRC_HELP\CompileChm.ps1" "$TMP_HELP\sakura\sakura.HHP" "$SRC_HELP\sakura\sakura.chm"

