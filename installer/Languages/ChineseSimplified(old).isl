; *** Inno Setup version 5.1.11+ ChineseSimplified(old).isl ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/files/istrans/
;
; EldersJavas@gmail.com edited at 2021.01.02.
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).

[LangOptions]
; The following three entries are very important. Be sure to read and
; understand the '[LangOptions] section' topic in the help file.
LanguageName=<7B80><4F53><4E2D><6587>
LanguageID=$0804
LanguageCodePage=936
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

SetupAppTitle=安裝
SetupWindowTitle=安裝 - %1
UninstallAppTitle=卸載
UninstallAppFullTitle=%1 卸載

; *** Misc. common
InformationTitle=信息
ConfirmTitle=確認
ErrorTitle=錯誤

; *** SetupLdr messages
SetupLdrStartupMessage=現在將安裝 %1。您想要繼續嗎？
LdrCannotCreateTemp=不能創建臨時文件。安裝中斷。
LdrCannotExecTemp=不能執行臨時目錄中的文件。安裝中斷。

; *** Startup error messages
LastErrorMessage=%1.%n%n錯誤 %2: %3
SetupFileMissing=安裝目錄中的文件 %1 丟失。請修正這個問題或獲取一個新的程序副本。
SetupFileCorrupt=安裝文件被破壞。請獲取一個新的程序副本。
SetupFileCorruptOrWrongVer=安裝文件被破壞，或是與這個安裝程序的版本不兼容。請修正這個問題或獲取新的程序副本。
NotOnThisPlatform=這個程序將不能運行於 %1。
OnlyOnThisPlatform=這個程序必須運行於 %1。
OnlyOnTheseArchitectures=這個程序只能在為下列處理器結構設計的 Windows 版本中進行安裝:%n%n%1
MissingWOW64APIs=你正在運行的 Windows 版不包含執行 64 位安裝程序所需的功能。要修正這個問題，請安裝 Service Pack %1。
WinVersionTooLowError=這個程序需要 %1 版本 %2 或更高。
WinVersionTooHighError=這個程序不能安裝於 %1 版本 %2 或更高。
AdminPrivilegesRequired=在安裝這個程序時您必須以管理員身份登錄。
PowerUserPrivilegesRequired=在安裝這個程序時您必須以管理員身份或有權限的用戶組身份登錄。
SetupAppRunningError=安裝程序發現 %1 當前正在運行。 %n%n請先關閉所有運行的窗口，然後單擊“確定”繼續，或按“取消”退出。
UninstallAppRunningError=卸載程序發現 %1 當前正在運行。 %n%n請先關閉所有運行的窗口，然後單擊“確定”繼續，或按“取消”退出。

; *** Misc. errors
ErrorCreatingDir=安裝程序不能創建目錄“%1”。
ErrorTooManyFilesInDir=不能在目錄“%1”中創建文件，因為裡面的文件太多

; *** Setup common messages
ExitSetupTitle=退出安裝程序
ExitSetupMessage=安裝程序未完成安裝。如果您現在退出，您的程序將不能安裝。 %n%n您可以以後再運行安裝程序完成安裝。 %n%n退出安裝程序嗎？
AboutSetupMenuItem=關於安裝程序(&A)...
AboutSetupTitle=關於安裝程序
AboutSetupMessage=%1 版本 %2%n%3%n%n%1 主頁:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< 上一步(&B)
ButtonNext=下一步(&N) >
ButtonInstall=安裝(&I)
ButtonOK=確定
ButtonCancel=取消
ButtonYes=是(&Y)
ButtonYesToAll=全部是(&A)
ButtonNo=否(&N)
ButtonNoToAll=全部否(&O)
ButtonFinish=完成(&F)
ButtonBrowse=瀏覽(&B)...
ButtonWizardBrowse=瀏覽(&R)...
ButtonNewFolder=新建文件夾(&M)

; *** "Select Language" dialog messages
SelectLanguageTitle=選擇安裝語言
SelectLanguageLabel=選擇安裝時要使用的語言:

; *** Common wizard text
ClickNext=單擊“下一步”繼續，或單擊“取消”退出安裝程序。
BeveledLabel=
BrowseDialogTitle=瀏覽文件夾
BrowseDialogLabel=在下列列表中選擇一個文件夾，然後單擊“確定”。
NewFolderName=新建文件夾

; *** "Welcome" wizard page
WelcomeLabel1=歡迎使用 [name] 安裝嚮導
WelcomeLabel2=現在將安裝 [name/ver] 到您的電腦中。 %n%n推薦您在繼續安裝前關閉所有其它應用程序。

; *** "Password" wizard page
WizardPassword=密碼
PasswordLabel1=這個安裝程序有密碼保護。
PasswordLabel3=請輸入密碼，然後單擊“下一步”繼續。密碼區分大小寫。
PasswordEditLabel=密碼(&P):
IncorrectPassword=您輸入的密碼不正確，請重試。

; *** "License Agreement" wizard page
WizardLicense=許可協議
LicenseLabel=繼續安裝前請閱讀下列重要信息。
LicenseLabel3=請仔細閱讀下列許可協議。您在繼續安裝前必須同意這些協議條款。
LicenseAccepted=我同意此協議(&A)
LicenseNotAccepted=我不同意此協議(&D)

; *** "Information" wizard pages
WizardInfoBefore=信息
InfoBeforeLabel=請在繼續安裝前閱讀下列重要信息。
InfoBeforeClickLabel=如果您想繼續安裝，單擊“下一步”。
WizardInfoAfter=信息
InfoAfterLabel=請在繼續安裝前閱讀下列重要信息。
InfoAfterClickLabel=如果您想繼續安裝，單擊“下一步”。

; *** "User Information" wizard page
WizardUserInfo=用戶信息
UserInfoDesc=請輸入您的信息。
UserInfoName=用戶名(&U):
UserInfoOrg=組織(&O):
UserInfoSerial=序列號(&S):
UserInfoNameRequired=您必須輸入名字。

; *** "Select Destination Location" wizard page
WizardSelectDir=選擇目標位置
SelectDirDesc=您想將 [name] 安裝在什麼地方？
SelectDirLabel3=安裝程序將安裝 [name] 到下列文件夾中。
SelectDirBrowseLabel=單擊“下一步”繼續。如果您想選擇其它文件夾，單擊“瀏覽”。
DiskSpaceMBLabel=至少需要有 [mb] MB 的可用磁盤空間。
ToUNCPathname=安裝程序不能安裝到一個 UNC 路徑名。如果您正在嘗試安裝到網絡，您需要映射一個網絡驅動器。
InvalidPath=您必須輸入一個帶驅動器卷標的完整路徑，例如:%n%nC:/APP%n%n或下列形式的 UNC 路徑:%n%n//server/share
InvalidDrive=您選定的驅動器或 UNC 共享不存在或不能訪問。請選選擇其它位置。
DiskSpaceWarningTitle=沒有足夠的磁盤空間
DiskSpaceWarning=itle=沒有足夠的磁盤空間
DirNameTooLong=文件夾名或路徑太長。
InvalidDirName=文件夾名是無效的。
BadDirName32=文件夾名不能包含下列任何字符:%n%n%1
DirExists=文件夾存在,繼續嗎?
DirExistsTitle=金思維

DirDoesntExist=文件夾不存在,繼續嗎?
DirDoesntExistTitle=金思維
; *** "Select Components" wizard page
WizardSelectComponents=選擇組件
SelectComponentsDesc=您想安裝哪些程序的組件？
SelectComponentsLabel2=選擇您想要安裝的組件；清除您不想安裝的組件。就緒後單擊“下一步”繼續。
FullInstallation=完全安裝
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=簡潔安裝
CustomInstallation=自定義安裝
NoUninstallWarningTitle=組件存在
NoUninstallWarning=itle=組件存在
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=當前選擇的組件至少需要 [mb] MB 的磁盤空間。

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=選擇附加任務
SelectTasksDesc=您想要安裝程序執行哪些附加任務？
SelectTasksLabel2=選擇您想要安裝程序在安裝 [name] 時執行的附加任務，然後單擊“下一步”。

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=選擇開始菜單文件夾
SelectStartMenuFolderDesc=您想在哪裡放置程序的快捷方式？
SelectStartMenuFolderLabel3=安裝程序現在將在下列開始菜單文件夾中創建程序的快捷方式。
SelectStartMenuFolderBrowseLabel=單擊“下一步”繼續。如果您想選擇其它文件夾，單擊“瀏覽”。
MustEnterGroupName=您必須輸入一個文件夾名。
GroupNameTooLong=文件夾名或路徑太長。
InvalidGroupName=文件夾名是無效的。
BadGroupName=文件夾名不能包含下列任何字符:%n%n%1
NoProgramGroupCheck2=不創建開始菜單文件夾(&D)

; *** "Ready to Install" wizard page
WizardReady=準備安裝
ReadyLabel1=安裝程序現在準備開始安裝 [name] 到您的電腦中。
ReadyLabel2a=單擊“安裝”繼續此安裝程序。如果您想要回顧或改變設置，請單擊“上一步”。
ReadyLabel2b=單擊“安裝”繼續此安裝程序?
ReadyMemoUserInfo=用戶信息:
ReadyMemoDir=目標位置:
ReadyMemoType=安裝類型:
ReadyMemoComponents=選定組件:
ReadyMemoGroup=開始菜單文件夾:
ReadyMemoTasks=附加任務:

; *** "Preparing to Install" wizard page
WizardPreparing=正在準備安裝
PreparingDesc=安裝程序正在準備安裝 [name] 到您的電腦中。
PreviousInstallNotCompleted=先前程序的安裝/卸載未完成。您需要重新啟動您的電腦才能完成安裝。 %n%n在重新啟動電腦後，再運行安裝完成 [name] 的安裝。
CannotContinue=安裝程序不能繼續。請單擊“取消”退出。

; *** "Installing" wizard page
WizardInstalling=正在安裝
InstallingLabel=安裝程序正在安裝 [name] 到您的電腦中，請等待。

; *** "Setup Completed" wizard page
FinishedHeadingLabel=[name] 安裝嚮導完成
FinishedLabelNoIcons=安裝程序已在您的電腦中安裝了 [name]。
FinishedLabel=oIcons=安裝程序已在您的電腦中安裝了 [name]。
ClickFinish=單擊“完成”退出安裝程序。
FinishedRestartLabel=要完成 [name] 的安裝，安裝程序必須重新啟動您的電腦。您想現在重新啟動嗎？
FinishedRestartMessage=要完成 [name] 的安裝，安裝程序必須重新啟動您的電腦。 %n%n您想現在重新啟動嗎？
ShowReadmeCheck=是，您想查閱自述文件
YesRadio=是，立即重新啟動電腦(&Y)
NoRadio=否，稍後重新啟動電腦(&N)
; used for example as 'Run MyProg.exe'
RunEntryExec=運行 %1
; used for example as 'View Readme.txt'
RunEntryShellExec=查閱 %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=安裝程序需要下一張磁盤
SelectDiskLabel2=請插入磁盤 %1 並單擊“確定”。 %n%n如果這個磁盤中的文件不能在不同於下列顯示的文件夾中找到，輸入正確的路徑或單擊“瀏覽”。
PathLabel=路徑(&P):
FileNotInDir2=文件“%1”不能在“%2”定位。請插入正確的磁盤或選擇其它文件夾。
SelectDirectoryLabel=請指定下一張磁盤的位置。

; *** Installation phase messages
SetupAborted=安裝程序未完成安裝。 %n%n請修正這個問題並重新運行安裝程序。
EntryAbortRetryIgnore=單擊“重試”進行重試，單擊“忽略”繼續，或單擊“中斷”取消安裝。

; *** Installation status messages
StatusCreateDirs=正在創建目錄...
StatusExtractFiles=正在解壓縮文件...
StatusCreateIcons=正在創建快捷方式...
StatusCreateIniEntries=正在創建 INI 條目...
StatusCreateRegistryEntries=正在創建註冊表條目...
StatusRegisterFiles=正在註冊文件...
StatusSavingUninstall=正在保存卸載信息...
StatusRunProgram=正在完成安裝...
StatusRollback=正在設置改變...

; *** Misc. errors
ErrorInternal2=內部錯誤: %1
ErrorFunctionFailedNoCode=%1 失敗
ErrorFunctionFailed=oCode=%1 失敗
ErrorFunctionFailedWithMessage=%1 失敗；代碼 %2.%n%3
ErrorExecutingProgram=不能執行文件:%n%1

; *** Registry errors
ErrorRegOpenKey=錯誤打開註冊表鍵:%n%1/%2
ErrorRegCreateKey=錯誤創建註冊表鍵:%n%1/%2
ErrorRegWriteKey=錯誤寫入註冊表鍵:%n%1/%2

; *** INI errors
ErrorIniEntry=在文件“%1”創建 INI 條目錯誤。

; *** File copying errors
FileAbortRetryIgnore=單擊“重試”進行重試，單擊“忽略”跳過這個文件 (不推薦)，或單擊“中斷”取消安裝。
FileAbortRetryIgnore2=單擊“重試”進行重試，單擊“忽略”繼續處理 (不推薦)，或單擊“中斷”取消安裝。
SourceIsCorrupted=源文件被破壞
SourceDoesntExist=源文件“%1”不存在
ExistingFileReadOnly=現有的文件標記為只讀。 %n%n單擊“重試”刪除只讀屬性後再試，單擊“忽略”跳過這個文件，或單擊“取消”退出安裝。
ErrorReadingExistingDest=嘗試讀了現有的文件時發生一個錯誤:
FileExists=文件已經存在。 %n%n您想要安裝程序覆蓋它嗎？
ExistingFileNewer=現有的文件新與安裝程序要安裝的文件。推薦您保留現有文件。 %n%n您想要保留現有的文件嗎？
ErrorChangingAttr=嘗試改變下列現有的文件的屬性時發生一個錯誤:
ErrorCreatingTemp=嘗試在目標目錄創建文件時發生一個錯誤:
ErrorReadingSource=嘗試讀取下列源文件時發生一個錯誤:
ErrorCopying=嘗試複製下列文件時發生一個錯誤:
ErrorReplacingExistingFile=嘗試替換現有的文件時發生錯誤:
ErrorRestartReplace=RestartReplace 失敗:
ErrorRenamingTemp=嘗試重新命名以下目標目錄中的一個文件時發生錯誤:
ErrorRegisterServer=不能註冊 DLL/OCX: %1
ErrorRegSvr32Failed=RegSvr32 failed with exit code %1
ErrorRegisterTypeLib=不能註冊類型庫: %1

; *** Post-installation errors
ErrorOpeningReadme=當嘗試打開自述文件時發生一個錯誤。
ErrorRestartingComputer=安裝程序不能重新啟動電腦，請手動重啟。

; *** Uninstaller messages
UninstallNotFound=文件“%1”不存在。不能卸載。
UninstallOpenError=文件“%1”不能打開。不能卸載
UninstallUnsupportedVer=卸載日誌文件“%1”有未被這個版本的卸載器承認的格式。不能卸載
UninstallUnknownEntry=在卸載日誌中遇到一個未知的條目 (%1)
ConfirmUninstall=您確認想要完全刪除 %1 及它的所有組件嗎？
UninstallOnlyOnWin64=這個安裝程序只能在 64 位 Windows 中進行卸載。
OnlyAdminCanUninstall=這個安裝的程序只能是有管理員權限的用戶才能卸載。
UninstallStatusLabel=正在從您的電腦中刪除 %1，請等待。
UninstalledAll=%1 已順利地從您的電腦中刪除。
UninstalledMost=%1 卸載完成。 %n%n有一些內容不能被刪除。您可以手工刪除它們。
UninstalledAndNeedsRestart=要完成 %1 的卸載，您的電腦必須重新啟動。 %n%n您現在想重新啟動電腦嗎？
UninstallDataCorrupted=“%1”文件被破壞，不能卸載

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=刪除共享文件嗎？
ConfirmDeleteSharedFile2=系統中包含的下列共享文件已經不被其它程序使用。您想要卸載程序刪除這些共享文件嗎？ %n%n如果這些文件被刪除，但還有程序正在使用這些文件，這些程序可能不能正確執行。如果您不能確定，選擇“否”。把這些文件保留在系統中以免引起問題。
SharedFileNameLabel=文件名:
SharedFileLocationLabel=位置:
WizardUninstalling=卸載狀態
StatusUninstalling=正在卸載 %1...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 版本 %2
AdditionalIcons=附加快捷方式:
CreateDesktopIcon=創建桌面快捷方式(&D)
CreateQuickLaunchIcon=創建快速運行欄快捷方式(&Q)
ProgramOnTheWeb=%1 網站
UninstallProgram=卸載 %1
LaunchProgram=運行 %1
AssocFileExtension=將 %2 文件擴展名與 %1 建立關聯(&A)
AssocingFileExtension=正在將 %2 文件擴展名與 %1 建立關聯...

 

 

 