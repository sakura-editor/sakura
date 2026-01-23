# sjis_to_utf8bom.ps1
# Converts a Shift-JIS encoded file to UTF-8 with BOM encoding.
param(
    [Parameter(Mandatory=$true)]
    [string]$File,
    
    [Parameter(Mandatory=$true)]
    [string]$Destination
)

$sjis = [System.Text.Encoding]::GetEncoding(932)
$utf8 = [System.Text.UTF8Encoding]::new($true)

[System.IO.File]::WriteAllText($Destination, [System.IO.File]::ReadAllText($File, $sjis), $utf8)
