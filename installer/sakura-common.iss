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
AppVerName={cm:AppVerName} {#MyAppVer}
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

; OSバージョン制限
MinVersion=0,5.0

[Languages]
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "en"; MessagesFile: "compiler:Default.isl"; InfoBeforeFile: "instmaterials\info_us.txt"

[CustomMessages]
en.InfoBeforeFile=instmaterials\info_en.txt
ja.InfoBeforeFile=instmaterials\info.txt
en.AppName=SAKURA Editor
ja.AppName=サクラエディタ
en.AppVerName=SAKURA Editor
ja.AppVerName=SAKURA Editor(サクラエディタ)
en.AppPublisher=SAKURA Editor developers
ja.AppPublisher=サクラエディタ開発チーム
en.TypesAll=All
ja.TypesAll=すべて
en.TypesEditorWithHelp=Editor with Help
ja.TypesEditorWithHelp=本体とヘルプ
en.TypesEditorOnly=Editor Only
ja.TypesEditorOnly=本体のみ
en.TypesCustom=Custom
ja.TypesCustom=カスタム
en.ComponentsMain=SAKURA Editor itself
ja.ComponentsMain=サクラエディタ本体
en.ComponentsHelp=help files
ja.ComponentsHelp=ヘルプファイル
en.ComponentsKeyword=KeyWord files
ja.ComponentsKeyword=KeyWordファイル
en.startmenu=Create &Start Menu
ja.startmenu=スタートメニューを作成(&S)
en.proglist=Add &Program List
ja.proglist=プログラム一覧に追加(&P)
en.fileassoc=Add "Open with SAKURA &Editor"
ja.fileassoc=「SAKURA Editorで開く」メニューの追加(&E)
en.sendto=Add to Send&To Menu
ja.sendto=送るに追加(&T)
en.sakuragrep=Add "&Grep with SAKURA Editor"
ja.sakuragrep=「SAKURA EditorでGrep」メニューの追加(&G)
en.fileassocMenu=Open with SAKURA &Editor
ja.fileassocMenu=SAKURA Editorで開く(&E)
en.sakuragrepMenu=&Grep with SAKURA Editor
ja.sakuragrepMenu=SAKURA EditorでGrep(&G)
en.residentStartup=&Resident at startup
ja.residentStartup=起動時に常駐(&R)
en.IconPreferencefolder=Preference folder
ja.IconPreferencefolder=設定フォルダ
en.StartNow=Start now
ja.StartNow=今すぐサクラエディタを起動
en.MultiUser=Install in compatibility mode. When editing system files and programs with user authority, editing results may not be accessible from other applications. (VirtualStore function)
ja.MultiUser=互換モードでインストールします．システムファイルおよびプログラムをユーザ権限で編集すると編集結果が他のアプリケーションから参照できないことがあります．(VirtualStore機能)
en.InitWiz_Title=Selection of Preference preservation method
ja.InitWiz_Title=設定保存方法の選択
en.InitWiz_SubTitle=Please choose whether you want to save the settings for each user or to the location of the executable file
ja.InitWiz_SubTitle=設定をユーザ毎に保存するか実行ファイルの場所へ保存するかを選択してください
en.InitWiz_Comment=Please do not change if you're unsure about this
ja.InitWiz_Comment=よくわからない場合は変更しないでください
en.InitWiz_Check=Manage Preference individually for each user
ja.InitWiz_Check=設定をユーザ毎に個別に管理する
en.ReadyMemo_SaveLocation=Preference file etc. save location
ja.ReadyMemo_SaveLocation=設定ファイル等格納場所
en.ReadyMemo_UserProfileDir=User-Profile directory
ja.ReadyMemo_UserProfileDir=ユーザ個別ディレクトリ
en.ReadyMemo_VirtualStoreDisable=Disable
ja.ReadyMemo_VirtualStoreDisable=無効
en.ReadyMemo_ExecProfileDir=Same as the executable file
ja.ReadyMemo_ExecProfileDir=実行ファイルと同一ディレクトリ
en.ReadyMemo_VirtualStoreEnable=Enable
ja.ReadyMemo_VirtualStoreEnable=有効




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
         selected := MsgBox(
{          'Program Files等のシステムフォルダへインストールする場合はUACを無効にしないと設定が保存できませんがよろしいですか？',}
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
