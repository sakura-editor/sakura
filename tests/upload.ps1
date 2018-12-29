#-------------------------------------------------------------------------------------------------
# upload result of googletest.
# https://www.appveyor.com/docs/running-tests/#pushing-real-time-test-results-to-build-console
#-------------------------------------------------------------------------------------------------
$base    = "https://ci.appveyor.com/api/testresults/junit/"
$url     = $base + $env:APPVEYOR_JOB_ID
$xmlfile = $Args[0]

Write-Output $url
Write-Output $xmlfile

$webclient = New-Object System.Net.WebClient 
$webclient.UploadFile($url , $xmlfile)
