Param($Arg1)

Add-Type -AssemblyName "System.IO.Compression.FileSystem"

if (Test-Path $Arg1)
{
    if (-Not(Split-Path -IsAbsolute $Arg1))
    {
        $arg1 = Convert-Path $Arg1
    }
    [System.IO.Compression.Zipfile]::OpenRead($Arg1).entries | ft -a LastWriteTime,Length,FullName
}
