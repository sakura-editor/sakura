#Wait-FileAccess.ps1
Param(
	[Parameter(Mandatory)]
    [string]$CompiledHelp,
	[Parameter(Mandatory)]
    [string]$CompileLog
)

# ファイルロックチェック
# Path Is Locked と読めるように作成
function Test-FileAccess
{
	[CmdletBinding()]
	Param(
		[Parameter(Mandatory, ValueFromPipeline)]
		[string]$Path
	)

	try
	{
		$File = New-Object System.IO.FileInfo $Path
		$Stream = $File.Open(
			[System.IO.FileMode]::Open,
			[System.IO.FileAccess]::ReadWrite,
			[System.IO.FileShare]::None)

		return $false
	}
	catch [System.IO.IOException]
	{
		echo "file is locked by a process."
		return $true
	}
	finally
	{
		if ($Stream -ne $null)
		{
			$Stream.Close()
		}
	}
}

#wait for complete
for ($count = 0; $count -lt 30; $count++)
{
  if (-not(Test-Path -Path $CompiledHelp))
  {
    echo "`$CompiledHelp is missing: $CompiledHelp"
  }
  elseif (Test-FileAccess $CompiledHelp)
  {
    echo "`$CompiledHelp is still locked: $CompiledHelp"
  }
  elseif (-not(Test-Path -Path $CompileLog))
  {
    echo "`$CompileLog is missing: $CompileLog"
  }
  elseif (Test-FileAccess $CompileLog)
  {
    echo "`$CompileLog is still locked: $CompileLog"
  }
  else
  {
    try
    {
      Copy-Item -Path $CompiledHelp -Destination "$env:TEMP\" -Force
      exit 0
    }
    catch #[System.IO.IOException]
    {
      echo "Copy Chm error [$($PSItem.Exception)]: $($PSItem.ToString())"
    }
  }

  Start-Sleep -Second 2
}

throw "Copy `$CompiledHelp has failed"
