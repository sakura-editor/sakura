Param($Arg1)

try
{
    Add-Type -AssemblyName "System.IO.Compression.FileSystem"
    
    $sourcezip = [System.IO.Compression.Zipfile]::OpenRead($Arg1)
    foreach($file in $sourcezip.Entries){
        Write-Host $file.LastWriteTime $file.Length $file.FullName
    }
    $sourcezip.Dispose()

}
catch
{
}
