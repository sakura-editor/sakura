Set Fs=WScript.CreateObject("Scripting.FileSystemObject")
getVersion = Fs.GetFileVersion(".\sakura.exe")
Version = Split(getVersion,".")
WScript.Echo "#define dVer1 """ & Version(0) & """"
WScript.Echo "#define dVer2 """ & Version(1) & """"
WScript.Echo "#define dVer3 """ & Version(2) & """"
WScript.Echo "#define dVer4 """ & Version(3) & """"
WScript.Echo
