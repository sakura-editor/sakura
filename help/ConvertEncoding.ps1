param ($TempHelp)
dir $TempHelp -Recurse -Include *.html,*.css,*.js |
	%{ (Get-Content $_ -Encoding UTF8)`
	-replace '(?<=<META http-equiv="Content-Type" content="text/html; charset=|@charset ")UTF-8','Shift_JIS' |
	Set-Content $_ -Encoding default}
