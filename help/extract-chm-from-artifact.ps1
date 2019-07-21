# this script was designed to run only for appveyor.
# https://www.appveyor.com/docs/api/samples/download-artifacts-ps/

$apiUrl = 'https://ci.appveyor.com/api'
$token = ''
$headers = @{
  "Authorization" = "Bearer $token"
  "Content-type" = "application/json"
}

if ($env:APPVEYOR_ACCOUNT_NAME -eq 'sakuraeditor') {
	# get project with current build details
	$getProjectApi = "$apiUrl/projects/sakuraeditor/sakura/build/$env:APPVEYOR_BUILD_VERSION"
} else {
	# get project with last build details
	$getProjectApi = "$apiUrl/projects/sakuraeditor/sakura"
}

try {
	# get project with build details
	$project = Invoke-RestMethod -Method Get -Uri $getProjectApi -Headers $headers -ErrorAction Stop

	$chmJob = $project.build.jobs | Where-Object name -eq 'Configuration: Release; Platform: BuildChm'
	$jobId = $chmJob.jobId

	# get job artifacts (just to see what we've got)
	$artifacts = Invoke-RestMethod -Method Get -Uri "$apiUrl/buildjobs/$jobId/artifacts" -Headers $headers -ErrorAction Stop

	# here we just take the first artifact, but you could specify its file name
	# $artifactFileName = 'sakura-Chm.zip'
	$artifactFileName = $artifacts[0].fileName
	if ($artifactFileName -notmatch '^sakura-.*-Chm.zip$') {
		throw "unexpected file name $artifactFileName."
	}

	# artifact will be downloaded as
	$localArtifactPath = "$PSScriptRoot\$artifactFileName"

	# download artifact
	# -OutFile - is local file name where artifact will be downloaded into
	# the Headers in this call should only contain the bearer token, and no Content-type, otherwise it will fail!
	Invoke-RestMethod -Method Get -Uri "$apiUrl/buildjobs/$jobId/artifacts/$artifactFileName" `
		-OutFile $localArtifactPath -Headers @{ "Authorization" = "Bearer $token" } `
		-ErrorAction Stop

	Start-Process -FilePath $env:CMD_7Z -ArgumentList "x -y $localArtifactPath" `
		-NoNewWindow `
		-WorkingDirectory $PSScriptRoot `
		-RedirectStandardError "$PSScriptRoot\unzip.err" `
		-Wait

	$unzipResult = Get-Content -Path "$PSScriptRoot\unzip.err" -TotalCount 3
	if ($unzipResult -is [array]) {
		throw "$unzipResult"
	}

} catch {
	Write-Output 'caught an error.'
	Write-Output $error[0]
	exit 1
}
