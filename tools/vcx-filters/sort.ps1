# sort.ps1
param(
  [String]$path = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..\sakura_core\sakura.vcxproj.filters")
)

$document = [System.Xml.XmlDocument]::new()
$document.PreserveWhitespace = $true
$document.Load($path)

$ns = [System.Xml.XmlNamespaceManager]::new($document.NameTable)
$ns.AddNamespace('msb', $document.DocumentElement.NamespaceURI)

foreach ($group in $document.SelectNodes('/msb:Project/msb:ItemGroup[msb:*[@Include]]', $ns)) {
  $nodes = @($group.SelectNodes('msb:*[@Include]', $ns))

  $sorted = @(
    $nodes |
      Sort-Object -Property { $_.GetAttribute('Include') } -CaseSensitive |
      ForEach-Object { $_.CloneNode($true) }
  )

  for ($i = 0; $i -lt $nodes.Count; $i++) {
    [void]$group.ReplaceChild($sorted[$i], $nodes[$i])
  }
}

$document.Save($path)
