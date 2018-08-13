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
AppName=サクラエディタ
AppId=sakura editor
AppVersion={#MyAppVer}
AppVerName=sakura editor(サクラエディタ) {#MyAppVer}
AppMutex=MutexSakuraEditor
AppPublisher=サクラエディタ開発チーム
AppPublisherURL=https://sakura-editor.github.io/
AppSupportURL=https://github.com/sakura-editor/sakura/issues
AppUpdatesURL=https://github.com/sakura-editor/sakura/releases
DefaultDirName={code:getDefautDirName|sakura}
DefaultGroupName=サクラエディタ
UninstallDisplayIcon={app}\sakura.exe
InfoBeforeFile="instmaterials\info.txt"

WizardImageFile="instmaterials\SetupModern20.bmp"
WizardSmallImageFile="instmaterials\SetupModernSmall20.bmp"
DisableStartupPrompt=no

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

[Types]
Name: all;                Description: "All"
Name: editorwithhelp;     Description: "Editor with Help"
Name: editoronly;         Description: "Editor Only"
Name: custom;             Description: "カスタム"; Flags: iscustom

[Components]
Name: main;        Description: "サクラエディタ本体";             Types: all editorwithhelp editoronly custom; Flags: fixed
Name: help;        Description: "ヘルプファイル";                 Types: all editorwithhelp
Name: keyword;     Description: "KeyWordファイル";                Types: all

[Tasks]
Name: startmenu;   Description: "スタートメニューを作成(&S)";         Components: main;
Name: quicklaunch; Description: "Quick Launchにアイコン作成(&Q)";     Components: main;
Name: proglist;    Description: "プログラム一覧に追加(&P)"; Components: main;
Name: desktopicon; Description: "デスクトップにアイコン作成(&D)";     Components: main; Flags: unchecked;
Name: fileassoc;   Description: "「SAKURAで開く」メニューの追加(&E)"; Components: main; Flags: unchecked;
Name: startup;     Description: "起動時に常駐(&B)";                   Components: main; Flags: unchecked;
Name: sendto;      Description: "送るに追加(&T)";                     Components: main; Flags: unchecked;

[Files]
Source: "sakura\sakura.exe";           DestDir: "{app}";         Components: main; Flags: ignoreversion;
Source: "sakura\sakura_lang_en_US.dll";DestDir: "{app}";         Components: main; Flags: ignoreversion;
Source: "sakura\bregonig.dll";         DestDir: "{app}";         Components: main
Source: "sakura\bsd_license.txt";      DestDir: "{app}";         Components: main
Source: "sakura\sakura.exe.manifest.x";DestDir: "{app}";         Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserDisabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.exe.manifest.v";DestDir: "{app}";         Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.chm";           DestDir: "{app}";         Components: help
Source: "sakura\macro.chm";            DestDir: "{app}";         Components: help
Source: "sakura\plugin.chm";           DestDir: "{app}";         Components: help
Source: "sakura\sakura.exe.ini";       DestDir: "{app}";         Components: main; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;

Source: "sakura\keyword\*"; DestDir: "{app}\keyword"; Components: keyword; Flags: recursesubdirs

[Registry]
; registry for all user (Admin only)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: ""; ValueData: "SAKURAで開く(&E)";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist; Flags: uninsdeletekey; Check: CheckPrivilege(true)

; registry for each user (non-Admin only)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: ""; ValueData: "SAKURAで開く(&E)";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\sakura.exe""";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist; Flags: uninsdeletekey; Check: CheckPrivilege(false)

[Icons]
Name: "{group}\サクラエディタ";                                                Filename: "{app}\sakura.exe";                         Components: main; Check: InTopMenu(false); Tasks: startmenu;
Name: "{userstartmenu}\サクラエディタ";                                        Filename: "{app}\sakura.exe";                         Components: main; Check: InTopMenu(true); Tasks: startmenu;
Name: "{group}\ヘルプファイル";                                                Filename: "{app}\sakura.chm";                         Components: help; Tasks: startmenu;
Name: "{group}\設定フォルダ";                                                  Filename: "%APPDATA%\sakura";                         Components: main; Check: isMultiUserEnabled; Tasks: startmenu;
Name: "{userdesktop}\サクラエディタ";                                          Filename: "{app}\sakura.exe";                         Components: main; Tasks: desktopicon;
Name: "{group}\アンインストール";                                              Filename: "{uninstallexe}";                           Tasks: startmenu;
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\サクラエディタ"; Filename: "{app}\sakura.exe";                         Components: main; Tasks: quicklaunch;
Name: "{userstartup}\サクラエディタ常駐";                                      Filename: "{app}\sakura.exe";   Parameters: "-NOWIN"; Components: main; Tasks: startup;
Name: "{#MySendTo}\サクラエディタ";                                            Filename: "{app}\sakura.exe";                         Components: main; Tasks: sendto;

[Run]
FileName: "{app}\sakura.exe"; Description: "今すぐサクラエディタを起動"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent; Check: CheckPrivilege(false);

[UninstallDelete]
;Uninstall時に確認無く消されるのでコメントアウト
;Type: files; Name: "{app}\sakura.ini"

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
          '互換モードでインストールします．システムファイルおよびプログラムをユーザ権限で編集すると編集結果が他のアプリケーションから参照できないことがあります．(VirtualStore機能)',
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
  MultiUserPage := CreateInputOptionPage( wpSelectComponents, '設定保存方法の選択',
    '設定をユーザ毎に保存するか実行ファイルの場所へ保存するかを選択してください',
    'よくわからない場合は変更しないでください', False, False );
  MultiUserPage.Add( '設定をユーザ毎に個別に管理する' );
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
  MemoMuitiUser := '設定ファイル等格納場所' + NewLine + Space;
  MemoVirtualStore := 'VirtualStore' + NewLine + Space;

  if isMultiUserEnabled then
    begin
      MemoMuitiUser := MemoMuitiUser + 'ユーザ個別ディレクトリ';
      MemoVirtualStore := MemoVirtualStore + '無効';
    end
  else
    begin
      MemoMuitiUser := MemoMuitiUser + '実行ファイルと同一ディレクトリ';
      MemoVirtualStore := MemoVirtualStore + '有効';
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
