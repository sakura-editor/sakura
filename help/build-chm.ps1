#remove comment
(Get-Content -LiteralPath $env:HH_INPUT -Encoding UTF8) -replace '//.*' |
 Set-Content -LiteralPath $env:HH_OUTPUT -Encoding UTF8

$cssrc=(Get-Content -LiteralPath "$env:SRC_HELP\EncoderEscapingFallbackBuffer.cs" -Raw),
	((Get-Content -LiteralPath "$env:SRC_HELP\EncoderEscapingFallback.cs" -Raw) -replace "using System.+\r\n") -join ""
Add-Type -TypeDefinition $cssrc -Language CSharp
$sjis=[Text.Encoding]::GetEncoding("shift_jis",[ChmSourceConverter.EncoderEscapingFallback]::new("&#{0};"),[Text.DecoderFallback]::ExceptionFallback)
$re=[Regex]::new('(?<=<META http-equiv="Content-Type" content="text/html; charset=|@charset ")UTF-8',"IgnoreCase")

#create a folder tree
robocopy $env:SRC_HELP $env:TMP_HELP /e /xf *.html *.css *.js *.chm *.chw *.log /r:5 /w:10 > $null

#convert
dir -LiteralPath $env:SRC_HELP -Recurse -File |
	%{ 
		if($_.name -match "\.(?:html|css|js)$"){
			$utf8=Get-Content -LiteralPath $_.fullname -Encoding UTF8 -Raw
			$string=$re.Replace($utf8, 'Shift_JIS', 1)
			$bytes=$sjis.GetBytes($string, 0, $string.Length)
			[IO.File]::WriteAllBytes(($_.fullname -replace ($env:SRC_HELP -replace "[[\]\\]",'\$&'), $env:TMP_HELP), $bytes)
		}
	}

#compile
$CP_ja=932
$backup_ACP=(Get-ItemProperty HKLM:\System\CurrentControlSet\Control\Nls\CodePage).ACP
"current ACP: $backup_ACP"

if($backup_ACP -ne $CP_ja){
	reg add HKLM\System\CurrentControlSet\Control\Nls\CodePage /f /v ACP /d $CP_ja
	"current ACP: $((Get-ItemProperty HKLM:\System\CurrentControlSet\Control\Nls\CodePage).ACP)"
}

&"$env:SRC_HELP\CompileChm.ps1" "$env:TMP_HELP\macro\macro.HHP" "$env:SRC_HELP\macro\macro.chm"
&"$env:SRC_HELP\CompileChm.ps1" "$env:TMP_HELP\plugin\plugin.HHP" "$env:SRC_HELP\plugin\plugin.chm"
&"$env:SRC_HELP\CompileChm.ps1" "$env:TMP_HELP\sakura\sakura.HHP" "$env:SRC_HELP\sakura\sakura.chm"

if($backup_ACP -ne $CP_ja){
	reg add HKLM\System\CurrentControlSet\Control\Nls\CodePage /f /v ACP /d $backup_ACP
	"current ACP: $((Get-ItemProperty HKLM:\System\CurrentControlSet\Control\Nls\CodePage).ACP)"
}

