<#
	.SYNOPSIS
	Git ワーキングツリーに含まれるファイルの更新日時に最終コミット日時を設定します。

	.DESCRIPTION
	Git でチェックアウトしたファイルの更新日時はチェックアウト日時になります。これはファイルのタイムスタンプを比較する場合に不都合な場合があり、代わりに最終コミット日時をタイムスタンプに設定します。

	Git リポジトリ内で実行します。すべての階層のファイルが対象になります。

	制限: ログを遡りすべてのファイルの最終コミット日時を取得することは１分以上かかることがあるため、このスクリプトでは１か月を上限としてログを遡ります。そのため最も古いタイムスタンプは実行日時を基準として前月同日の00:00:00になります。
#>
$TopDir = git rev-parse --show-toplevel
$Oldest = [DateTime]::Today.AddMonths(-1) # Of course, this is incorrect for the oldest timestamp. That's what "Roughly" means.

@(
	"?$($Oldest.ToString("yyyy-MM-dd HH:mm:ss"))"
	, (git ls-files --full-name "$TopDir")
	, (git log --format=format:?%ci --name-only --since="$($Oldest.ToString("yyyy-MM-dd HH:mm:ss"))" --reverse)
) | foreach { $_ | foreach { $_ } } | foreach {
	if ($_ -eq "") {
	} elseif ($_[0] -eq "?") {
		$t = $_.Substring(1) -as [DateTime]
	} else {
		try { ([System.IO.FileInfo]"$TopDir\$_").LastWriteTime = $t }
		catch { Write-Debug($_) }
	}
}
