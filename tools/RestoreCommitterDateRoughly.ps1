<#
	.SYNOPSIS
	Git ワーキングツリーに含まれるファイルの更新日時に最終コミット日時を設定します。

	.DESCRIPTION
	Git でチェックアウトしたファイルの更新日時はチェックアウト日時になります。これはファイルのタイムスタンプを比較するのに不都合があるため、代わりに最終コミット日時を設定します。

	Git ワーキングツリー内で実行します。すべての階層のファイルが対象になります。

	制限: ログを遡りすべてのファイルの最終コミット日時を取得することは１分以上かかることがあるため、このスクリプトでは１か月を上限としてログを遡ります。そのため最も古いタイムスタンプは実行日時を基準として前月同日の00:00:00になります。
#>
$TopDir = (git rev-parse --show-toplevel)
$Oldest = [DateTime]::Today.AddMonths(-1).ToString("yyyy-MM-dd HH:mm:ss") # Of course, this is incorrect for the oldest timestamp. That's what "Roughly" means.

$waiting = @{}
(git ls-files --full-name "$TopDir") | foreach {
	$waiting[$_] = $NULL
}

@(
	(git log --format=format:?%ci --name-only --since="$Oldest")
	, "?$Oldest"
	, (git ls-files --full-name "$TopDir")
) | foreach { $_ | foreach {
	if ($_ -eq "") {
	} elseif ($_[0] -eq "?") {
		$t = $_.Substring(1)
	} else {
		$f = [System.IO.FileInfo] "$TopDir\$_"

		# run once a file.
		$c = $waiting.Count
		$waiting.Remove($_)
		if ($waiting.Count -ne $c -and $f.Exists) {
			try {
				$t = [System.DateTime] $t
				$f.LastWriteTime = $t
			} catch {
				Write-Debug $_
			}
		}
	}
} }
