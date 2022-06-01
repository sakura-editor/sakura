#if VER < EncodeVer(5,6,1)
  #define MySendTo "{sendto}"
#else
  #define MySendTo "{usersendto}"
#endif
#define MyAppVer GetFileVersion("sakura\sakura.exe")
#define MyAppVerH StringChange(MyAppVer, ".", "-")

[Setup]
#if MyArchitecture != "x86"
ArchitecturesInstallIn64BitMode={#MyArchitecture}
ArchitecturesAllowed={#MyArchitecture}
#endif
AppName={cm:AppName}
AppId=sakura editor
AppVersion={#MyAppVer}
AppVerName={cm:AppVerName} {#MyAppVer} ({#MyArchitecture})
AppMutex=MutexSakuraEditor
AppPublisher={cm:AppPublisher}
AppPublisherURL=https://sakura-editor.github.io/
AppSupportURL=https://github.com/sakura-editor/sakura/issues
AppUpdatesURL=https://github.com/sakura-editor/sakura/releases
DefaultDirName={code:getDefautDirName|sakura}
DefaultGroupName={cm:AppName}
UninstallDisplayIcon={app}\sakura.exe
InfoBeforeFile="instmaterials\info.txt"
LanguageDetectionMethod=uilanguage 
SolidCompression=yes

WizardImageFile="instmaterials\SetupModern20.bmp"
WizardSmallImageFile="instmaterials\SetupModernSmall20.bmp"
DisableStartupPrompt=yes
DisableWelcomePage=no

PrivilegesRequired=None

; 出力先ディレクトリ
OutputDir=Output-{#OutputSuffix}

; エディタのバージョンに応じて書き換える場所
OutputBaseFilename=sakura_install{#MyAppVerH}-{#MyArchitecture}
VersionInfoVersion={#MyAppVer}
VersionInfoProductVersion={#MyAppVer}

; OSバージョン制限(Windows 7 以降に対応)
MinVersion=6.1

[Languages]
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "en"; MessagesFile: "compiler:Default.isl"; InfoBeforeFile: "instmaterials\info_us.txt"
Name: "zh_hans"; MessagesFile: "Languages\ChineseSimplified.isl"; InfoBeforeFile: "instmaterials\info_zh_hans.txt"
Name: "zh_hant"; MessagesFile: "Languages\ChineseTraditional.isl"; InfoBeforeFile: "instmaterials\info_zh_hant.txt"

[CustomMessages]
en.InfoBeforeFile=instmaterials\info_en.txt
ja.InfoBeforeFile=instmaterials\info.txt
zh_hans.InfoBeforeFile=instmaterials\info_zh_hans.txt
zh_hant.InfoBeforeFile=instmaterials\info_zh_hant.txt

en.AppName=SAKURA Editor
ja.AppName=サクラエディタ
zh_hans.AppName=樱花编辑器
zh_hant.AppName=櫻花編輯器

en.AppVerName=SAKURA Editor
ja.AppVerName=SAKURA Editor(サクラエディタ)
zh_hans.AppVerName=樱花编辑器
zh_hant.AppVerName=櫻花編輯器

en.AppPublisher=SAKURA Editor developers
ja.AppPublisher=サクラエディタ開発チーム
zh_hans.AppPublisher=樱花编辑器 开发团队
zh_hant.AppPublisher=櫻花編輯器 開發团队

en.TypesAll=All
ja.TypesAll=すべて
zh_hans.TypesAll=全部安装
zh_hant.TypesAll=全部安裝

en.TypesEditorWithHelp=Editor with Help
ja.TypesEditorWithHelp=本体とヘルプ
zh_hans.TypesEditorWithHelp=本体与帮助文件
zh_hant.TypesEditorWithHelp=本體與幫助檔

en.TypesEditorOnly=Editor Only
ja.TypesEditorOnly=本体のみ
zh_hans.TypesEditorOnly=仅安装本体
zh_hant.TypesEditorOnly=僅安裝本體

en.TypesCustom=Custom
ja.TypesCustom=カスタム
zh_hans.TypesCustom=自定义
zh_hant.TypesCustom=自定義

en.ComponentsMain=SAKURA Editor itself
ja.ComponentsMain=サクラエディタ本体
zh_hans.ComponentsMain=樱花编辑器
zh_hant.ComponentsMain=櫻花編輯器

en.ComponentsHelp=help files
ja.ComponentsHelp=ヘルプファイル
zh_hans.ComponentsHelp=帮助文件
zh_hant.ComponentsHelp=幫助檔

en.ComponentsKeyword=KeyWord files
ja.ComponentsKeyword=KeyWordファイル
zh_hans.ComponentsKeyword=语法提示文件
zh_hant.ComponentsKeyword=語法提示檔案

en.startmenu=Create &Start Menu
ja.startmenu=スタートメニューを作成(&S)
zh_hans.startmenu=添加到开始菜单(&S)
zh_hant.startmenu=添加到開始菜單(&S)

en.proglist=Add &Program List
ja.proglist=プログラム一覧に追加(&P)
zh_hans.proglist=添加到程序列表(&P)
zh_hant.proglist=添加到程式清單(&P)

en.fileassoc=Add "Open with SAKURA &Editor"
ja.fileassoc=「SAKURA Editorで開く」メニューの追加(&E)
zh_hans.fileassoc=添加"用 樱花编辑器 打开"(&E)
zh_hant.fileassoc=添加“用 櫻花編輯器 打開”(&E)

en.sendto=Add to Send&To Menu
ja.sendto=送るに追加(&T)
zh_hans.sendto=添加到"发送到"菜单(&T)
zh_hant.sendto=添加到“發送到”選單(&T)

en.sakuragrep=Add "&Grep with SAKURA Editor"
ja.sakuragrep=「SAKURA EditorでGrep」メニューの追加(&G)
zh_hans.sakuragrep=添加到 "Grep with SAKURA Editor"(&G)
zh_hant.sakuragrep=添加到 “Grep with SAKURA Editor”(&G)

en.fileassocMenu=Open with SAKURA &Editor
ja.fileassocMenu=SAKURA Editorで開く(&E)
zh_hans.fileassocMenu=用 樱花编辑器 打开(&E)
zh_hant.fileassocMenu=用 櫻花編輯器 打開く(&E)

en.sakuragrepMenu=&Grep with SAKURA Editor
ja.sakuragrepMenu=SAKURA EditorでGrep(&G)
zh_hans.sakuragrepMenu=Grep with SAKURA Editor(&G)
zh_hant.sakuragrepMenu=Grep with SAKURA Editor(&G)

en.residentStartup=&Resident at startup
ja.residentStartup=起動時に常駐(&R)
zh_hans.residentStartup=开机时启动(&R)
zh_hant.residentStartup=開機時啟動(&R)

en.IconPreferencefolder=Preference folder
ja.IconPreferencefolder=設定フォルダー
zh_hans.IconPreferencefolder=文件夹设置
zh_hant.IconPreferencefolder=資料夾設定

en.StartNow=Start now
ja.StartNow=今すぐサクラエディタを起動
zh_hans.StartNow=现在启动
zh_hant.StartNow=現在啟動

en.MultiUser=Install in compatibility mode. When editing system files and programs with user authority, editing results may not be accessible from other applications. (VirtualStore function)
ja.MultiUser=互換モードでインストールします．システムファイルおよびプログラムをユーザー権限で編集すると編集結果が他のアプリケーションから参照できないことがあります．(VirtualStore機能)
zh_hans.MultiUser=软件将会以兼容模式安装。使用非管理员用户编辑配置文件时，文件可能无法被管理员用户访问。(VirtualStore功能)
zh_hant.MultiUser=軟件將會以相容模式安裝。使用非管理員用戶編輯設定檔時，檔案可能無法被管理員用戶訪問。（VirtualStore功能）

en.InitWiz_Title=Selection of Preference preservation method
ja.InitWiz_Title=設定保存方法の選択
zh_hans.InitWiz_Title=配置文件保存位置
zh_hant.InitWiz_Title=設定檔保存位置

en.InitWiz_SubTitle=Please choose whether you want to save the settings for each user or to the location of the executable file
ja.InitWiz_SubTitle=設定をユーザー毎に保存するか実行ファイルの場所へ保存するかを選択してください
zh_hans.InitWiz_SubTitle=选择将Sukura配置文件保存至当前用户或软件目录内
zh_hant.InitWiz_SubTitle=選擇將Sukura設定檔保存至當前用戶或軟件目錄內

en.InitWiz_Comment=Please do not change if you are unsure about this
ja.InitWiz_Comment=よくわからない場合は変更しないでください
zh_hans.InitWiz_Comment=若您不清楚此选项，请不要修改
zh_hant.InitWiz_Comment=若您不清楚此選項，請不要修改

en.InitWiz_Check=Manage Preference individually for each user
ja.InitWiz_Check=設定をユーザー毎に個別に管理する
zh_hans.InitWiz_Check=将每个用户的配置文件单独保存
zh_hant.InitWiz_Check=將每個用戶的設定檔單獨保存

en.ReadyMemo_SaveLocation=Preference file etc. save location
ja.ReadyMemo_SaveLocation=設定ファイル等格納場所
zh_hans.ReadyMemo_SaveLocation=设定文件保存位置
zh_hant.ReadyMemo_SaveLocation=設定檔案保存位置

en.ReadyMemo_UserProfileDir=User-Profile directory
ja.ReadyMemo_UserProfileDir=ユーザー個別ディレクトリ
zh_hans.ReadyMemo_UserProfileDir=用户配置文件目录
zh_hant.ReadyMemo_UserProfileDir=用戶設定檔目錄

en.ReadyMemo_VirtualStoreDisable=Disable
ja.ReadyMemo_VirtualStoreDisable=無効
zh_hans.ReadyMemo_VirtualStoreDisable=无效
zh_hant.ReadyMemo_VirtualStoreDisable=無效

en.ReadyMemo_ExecProfileDir=Same as the executable file
ja.ReadyMemo_ExecProfileDir=実行ファイルと同一ディレクトリ
zh_hans.ReadyMemo_ExecProfileDir=与可执行文件相同
zh_hant.ReadyMemo_ExecProfileDir=與可執行文件相同

en.ReadyMemo_VirtualStoreEnable=Enable
ja.ReadyMemo_VirtualStoreEnable=有効
zh_hans.ReadyMemo_VirtualStoreEnable=生效
zh_hant.ReadyMemo_VirtualStoreEnable=生效




[Types]
Name: all;                 Description: "{cm:TypesAll}"
Name: TypesEditorWithHelp; Description: "{cm:TypesEditorWithHelp}"
Name: TypesEditorOnly;     Description: "{cm:TypesEditorOnly}"
Name: custom;              Description: "{cm:TypesCustom}";    Flags: iscustom

[Components]
Name: main;        Description: "{cm:ComponentsMain}";    Types: all TypesEditorWithHelp TypesEditorOnly custom; Flags: fixed
Name: help;        Description: "{cm:ComponentsHelp}";    Types: all TypesEditorWithHelp
Name: keyword;     Description: "{cm:ComponentsKeyword}"; Types: all

[Tasks]
Name: startmenu;   Description: "{cm:startmenu}";                         Components: main;
Name: quicklaunch; Description: "{cm:CreateQuickLaunchIcon}";             Components: main;
Name: proglist;    Description: "{cm:proglist}";                          Components: main;
Name: desktopicon; Description: "{cm:CreateDesktopIcon}";                 Components: main; Flags: unchecked;
Name: fileassoc;   Description: "{cm:fileassoc}";                         Components: main; Flags: unchecked;
Name: startup;     Description: "{cm:residentStartup}";                   Components: main; Flags: unchecked;
Name: sendto;      Description: "{cm:sendto}";                            Components: main; Flags: unchecked;
Name: sakuragrep;  Description: "{cm:sakuragrep}";                        Components: main; Flags: unchecked;

[Files]
Source: "sakura\sakura.exe";           DestDir: "{app}";                  Components: main; Flags: ignoreversion;
Source: "sakura\sakura_lang_en_US.dll";DestDir: "{app}";                  Components: main; Flags: ignoreversion;
Source: "sakura\license\LICENSE";      DestDir: "{app}\license";          Components: main
Source: "sakura\bregonig.dll";         DestDir: "{app}";                  Components: main
Source: "sakura\license\bregonig\*";   DestDir: "{app}\license\bregonig"; Components: main
Source: "sakura\ctags.exe";            DestDir: "{app}";                  Components: main
Source: "sakura\license\ctags\*";      DestDir: "{app}\license\ctags";    Components: main
Source: "sakura\sakura.exe.manifest.x";DestDir: "{app}";                  Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserDisabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.exe.manifest.v";DestDir: "{app}";                  Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.chm";           DestDir: "{app}";                  Components: help
Source: "sakura\macro.chm";            DestDir: "{app}";                  Components: help
Source: "sakura\plugin.chm";           DestDir: "{app}";                  Components: help
Source: "sakura\sakura.exe.ini";       DestDir: "{app}";                  Components: main; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;

Source: "sakura\keyword\*";             DestDir: "{app}\keyword";         Components: keyword; Flags: recursesubdirs

[Registry]
; registry for all user (Admin only)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";                       ValueType: string; ValueName: "";     ValueData: "{cm:fileassocMenu}";          Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";                       ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";        Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command";               ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist;  Flags: uninsdeletekey; Check: CheckPrivilege(true)

Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep";         ValueType: string; ValueName: "";     ValueData: "{cm:sakuragrepMenu}";                 Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";            Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" -GREPDLG -GREPMODE  -GFOLDER=""%1"" -GOPT=""SP"" -GCODE=99 "; Tasks: sakuragrep;  Flags: uninsdeletekey; Check: CheckPrivilege(true)

Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep";         ValueType: string; ValueName: "";     ValueData: "{cm:sakuragrepMenu}";      Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe"""; Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" -GREPDLG -GREPMODE  -GFOLDER=""%V"" -GOPT=""SP"" -GCODE=99  "; Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(true)

; add ProgID
; see https://www.glamenv-septzen.net/view/14#idf5215e
; see https://docs.microsoft.com/en-us/visualstudio/extensibility/registering-verbs-for-file-name-extensions?view=vs-2017
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document";                    ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell";              ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell\open";         ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell\open\command"; ValueType: string; ValueName: "";       ValueData: """{app}\sakura.exe"" ""%1""";   Flags: uninsdeletekey; Check: CheckPrivilege(true)

; add File Handlers to each extensions
; see https://docs.microsoft.com/en-us/visualstudio/extensibility/specifying-file-handlers-for-file-name-extensions?view=vs-2017
Root: HKLM; Subkey: "SOFTWARE\Classes\.txt\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.log\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.c\OpenWithProgids";                       ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.cpp\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.cs\OpenWithProgids";                      ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.h\OpenWithProgids";                       ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.md\OpenWithProgids";                      ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.ini\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.java\OpenWithProgids";                    ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\.rst\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(true)

; registry for each user (non-Admin only)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";                       ValueType: string; ValueName: "";     ValueData: "{cm:fileassocMenu}";          Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";                       ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";        Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command";               ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist;  Flags: uninsdeletekey; Check: CheckPrivilege(false)

Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep";         ValueType: string; ValueName: "";     ValueData: "{cm:sakuragrepMenu}";                 Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";            Tasks: sakuragrep; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\shell\sakuraGrep\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" -GREPDLG -GREPMODE  -GFOLDER=""%1"" -GOPT=""SP"" -GCODE=99  "; Tasks: sakuragrep;  Flags: uninsdeletekey; Check: CheckPrivilege(false)

Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep";         ValueType: string; ValueName: "";     ValueData: "{cm:sakuragrepMenu}";      Tasks: sakuragrep;  Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe"""; Tasks: sakuragrep;  Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\directory\BackGround\shell\sakuraGrep\command"; ValueType: string; ValueName: "";     ValueData: """{app}\sakura.exe"" -GREPDLG -GREPMODE  -GFOLDER=""%V"" -GOPT=""SP"" -GCODE=99  "; Tasks: sakuragrep;  Flags: uninsdeletekey; Check: CheckPrivilege(false)

; add ProgID
; see https://www.glamenv-septzen.net/view/14#idf5215e
; see https://docs.microsoft.com/en-us/visualstudio/extensibility/registering-verbs-for-file-name-extensions?view=vs-2017
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document";                    ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell";              ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell\open";         ValueType: string; ValueName: "";       ValueData: "";                              Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\SakuraEditor.Document\shell\open\command"; ValueType: string; ValueName: "";       ValueData: """{app}\sakura.exe"" ""%1""";   Flags: uninsdeletekey; Check: CheckPrivilege(false)

; add File Handlers to each extensions
; see https://docs.microsoft.com/en-us/visualstudio/extensibility/specifying-file-handlers-for-file-name-extensions?view=vs-2017
Root: HKLM; Subkey: "SOFTWARE\Classes\.txt\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.log\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.c\OpenWithProgids";                       ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.cpp\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.cs\OpenWithProgids";                      ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.h\OpenWithProgids";                       ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.md\OpenWithProgids";                      ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.ini\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.java\OpenWithProgids";                    ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)
Root: HKLM; Subkey: "SOFTWARE\Classes\.rst\OpenWithProgids";                     ValueType: string; ValueName: "SakuraEditor.Document"; ValueData: "";           Flags: uninsdeletevalue; Check: CheckPrivilege(false)

[Icons]
Name: "{group}\{cm:AppName}";                                                Filename: "{app}\sakura.exe";                         Components: main; Check: InTopMenu(false);   Tasks: startmenu;
Name: "{userstartmenu}\{cm:AppName}";                                        Filename: "{app}\sakura.exe";                         Components: main; Check: InTopMenu(true);    Tasks: startmenu;
Name: "{group}\{cm:ComponentsHelp}";                                         Filename: "{app}\sakura.chm";                         Components: help;                            Tasks: startmenu;
Name: "{group}\{cm:IconPreferencefolder}";                                   Filename: "%APPDATA%\sakura";                         Components: main; Check: isMultiUserEnabled; Tasks: startmenu;
Name: "{userdesktop}\{cm:AppName}";                                          Filename: "{app}\sakura.exe";                         Components: main;                            Tasks: desktopicon;
Name: "{group}\{cm:UninstallProgram,{cm:AppName}}";                          Filename: "{uninstallexe}";                                                                        Tasks: startmenu;
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{cm:AppName}"; Filename: "{app}\sakura.exe";                         Components: main;                            Tasks: quicklaunch;
Name: "{userstartup}\{cm:residentStartup}";                                  Filename: "{app}\sakura.exe";   Parameters: "-NOWIN"; Components: main;                            Tasks: startup;
Name: "{#MySendTo}\{cm:AppName}";                                            Filename: "{app}\sakura.exe";                         Components: main;                            Tasks: sendto;

[Run]
FileName: "{app}\sakura.exe"; Description: "{cm:StartNow}"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent; Check: CheckPrivilege(false);

[UninstallDelete]
;Uninstall時に確認無く消されるのでコメントアウト
;Type: files; Name: "{app}\sakura.ini"
;Type: files; Name: "{userappdata}\sakura\sakura.ini"; Check: isMultiUserEnabled
;Type: files; Name: "{app}\sakura.ini"; Check: isMultiUserDisabled

[Dirs]
Name: "{userappdata}\sakura"; Components: main; Tasks: startmenu; Check: isMultiUserEnabled

[Code]
var
  MultiUserPage: TInputOptionWizardPage;
  MultiUserPageEnabled : Boolean;

{ **********************************
   Utility Functions
  ********************************** }

function isMultiUserEnabled : Boolean;
begin
  Result := False;
  if MultiUserPageEnabled then
    Result := MultiUserPage.Values[0];
end;

function isMultiUserDisabled : Boolean;
begin
  Result := not isMultiUserEnabled;
end;

function CheckPrivilege( admin: Boolean ) : Boolean;
begin
	if admin then
	  begin
		Result := IsAdminLoggedOn;
	  end
	else
	  begin
		Result := not IsAdminLoggedOn;
	  end;
end;

{
  default installation target directory
  Admin: Program Files\sakura
  User:  <AppData>\sakura
}
function getDefautDirName( progdir : String ) : String;
var
  basedir : String;
begin
  if IsAdminLoggedOn then
    begin
      basedir := ExpandConstant( '{pf}' );
    end
  else
    begin
     basedir := ExpandConstant( '{userappdata}' );
    end;
  Result := basedir + '\' + progdir;
end;

function IsVistaOrLater : Boolean;
begin
  Result := UsingWinNT and (( GetWindowsVersion shr 24) >= 6 );
end;

function IsWin2kOrLater : Boolean;
begin
  Result := UsingWinNT and (( GetWindowsVersion shr 24) >= 5 );
end;

function IsWin10OrLater : Boolean;
var
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  if (Version.Major >= 10) then
  begin
    Result := True;
  end else begin
    Result := False;
  end;
end;
function InTopMenu( TopMenu : Boolean ) : Boolean;
begin
  if TopMenu then
  begin
    if IsWin10OrLater then
    begin
      Result := True;
    end else begin
      Result := False;
    end
  end else begin
    Result := True;
  end;
end;
{ **********************************
   Custom Wizard Page
  ********************************** }

{ Callback event functions }

function NextButtonClickMultiUser( Sender : TWizardPage): Boolean;
var
  selected: Integer;
{  t : String;}
begin
  Result := True;
  if MultiUserPageEnabled then
  begin

{ DEBUG CODE
   t := 'MultiUser Setting :';
   if IsAdminLoggedOn then
     t := t + 'Administrator ';
   if ((GetWindowsVersion shr 24) >= 6 ) then
     t := t + 'Vista ';
   if MultiUserPage.Values[0] = False then
     t := t + 'SingleUser ';
   t := t + Format( 'WinVer: %.8x', [GetWindowsVersion] );
   MsgBox( t, mbConfirmation, MB_OK);
}

    { Alert if admin mode && multi user = False }
    if IsAdminLoggedOn and
       IsVistaOrLater and
      ( MultiUserPage.Values[0] = False ) then
      begin
{
         Program Files等のシステムフォルダーへインストールする場合はUACを無効にしないと設定が保存できません。
}
         selected := MsgBox(
          CustomMessage('MultiUser'),
          mbConfirmation,
          ( MB_OKCANCEL ));
        if selected = IDCANCEL then
          Result := False;
      end;
  end;
end;

function ShoudSkipMultiUser(Sender : TWizardPage ): Boolean;
var
  selectdir : String;
  prev_version_h, prev_version_l : Cardinal;
begin
  Result := False;
    if ( not MultiUserPageEnabled ) then
      Result := True
    else
      begin
        selectdir := AddBackslash(ExpandConstant( '{app}' ));
        if FileExists( selectdir + 'sakura.exe' ) then
          begin
            Result := True;
            if GetIniInt( 'Settings', 'MultiUser', 0, 0, 1, selectdir + 'sakura.exe.ini' ) = 1 then
              MultiUserPage.Values[0] := True
            else
              MultiUserPage.Values[0] := False;
          end;
      end;
end;

{ **********************************
   System Event Functions
  ********************************** }

{ Add multi user selection page if supported }
procedure InitializeWizard;
begin
  { Create multi user page }
  MultiUserPage := CreateInputOptionPage( wpSelectComponents, CustomMessage('InitWiz_Title'),
    CustomMessage('InitWiz_SubTitle'),
    CustomMessage('InitWiz_Comment'), False, False );
  MultiUserPage.Add( CustomMessage('InitWiz_Check') );
  MultiUserPage.Values[0] := False;
  MultiUserPage.OnShouldSkipPage := @ShoudSkipMultiUser;
  MultiUserPage.OnNextButtonClick := @NextButtonClickMultiUser;
  MultiUserPageEnabled := False;
  {Show multiuser mode configuration only in case of Win2K or later }
  if IsWin2kOrLater then
  begin
    { multi user mode is supported }
    MultiUserPageEnabled := True;

    {
      default multi user mode
      User: OFF because installation target is also local
      Admin:
        Vista or later: ON
        Other: OFF
    }
    if IsVistaOrLater and
      IsAdminLoggedOn then
        MultiUserPage.Values[0] := True
      else
        MultiUserPage.Values[0] := False;
  end;
end;

{ Build List of installation configuration for ready page }
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo,
  MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  MemoMuitiUser, MemoVirtualStore : String;
begin
  MemoMuitiUser := CustomMessage('ReadyMemo_SaveLocation') + NewLine + Space;
  MemoVirtualStore := 'VirtualStore' + NewLine + Space;

  if isMultiUserEnabled then
    begin
      MemoMuitiUser := MemoMuitiUser + CustomMessage('ReadyMemo_UserProfileDir');
      MemoVirtualStore := MemoVirtualStore + CustomMessage('ReadyMemo_VirtualStoreDisable');
    end
  else
    begin
      MemoMuitiUser := MemoMuitiUser + CustomMessage('ReadyMemo_ExecProfileDir');
      MemoVirtualStore := MemoVirtualStore + CustomMessage('ReadyMemo_VirtualStoreEnable');
    end;

  if IsVistaOrLater then
    MemoVirtualStore := MemoVirtualStore + NewLine + NewLine
  else
    MemoVirtualStore := '';

  Result := MemoDirInfo + NewLine + NewLine +
            MemoMuitiUser + NewLine + NewLine +
            MemoVirtualStore +
            MemoComponentsInfo + NewLine + NewLine +
            MemoGroupInfo + NewLine + NewLine +
            MemoTasksInfo;
end;


{ **********************************
   Sample codes (not used at this moment
  ********************************** }

{
Const
  UAC_SUPPORT_LOWEST_VERSION_H = $00010005;  1.5.
  UAC_SUPPORT_LOWEST_VERSION_L = $000f00c8;  .15.200

        if GetVersionNumbers( selectdir + 'sakura.exe', prev_version_h, prev_version_l ) then
          begin
            Result := False;
            MsgBox( Format( 'existing version at %s : %d.%d.%d.%d', [
              selectdir + 'sakura.exe',
              (prev_version_h shr 16 ),
              (prev_version_h and $ffff ),
              (prev_version_l shr 16 ),
              (prev_version_l and $ffff )] ),
            mbConfirmation, MB_OK);
            if ( prev_version_h < UAC_SUPPORT_LOWEST_VERSION_H ) or
              (( prev_version_h = UAC_SUPPORT_LOWEST_VERSION_H )
              and ( prev_version_l < UAC_SUPPORT_LOWEST_VERSION_L )) then
                begin
                end
          end
}
