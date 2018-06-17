[Setup]
AppName=�T�N���G�f�B�^
AppId=sakura editor
AppVerName=sakura editor(�T�N���G�f�B�^)
AppMutex=MutexSakuraEditor
AppPublisher=�T�N���G�f�B�^�J���`�[��
AppPublisherURL=http://sakura-editor.sourceforge.net
AppSupportURL=http://sakura-editor.sourceforge.net
AppUpdatesURL=http://sourceforge.net/projects/sakura-editor/
DefaultDirName={code:getDefautDirName|sakura}
DefaultGroupName=�T�N���G�f�B�^
UninstallDisplayIcon={app}\sakura.exe
InfoBeforeFile="instmaterials\info.txt"

WizardImageFile="instmaterials\SetupModern20.bmp"
WizardSmallImageFile="instmaterials\SetupModernSmall20.bmp"
DisableStartupPrompt=no

PrivilegesRequired=None

; �G�f�B�^�̃o�[�W�����ɉ����ď���������ꏊ
OutputBaseFilename=sakura_install2-3-0-0
VersionInfoVersion=2.3.0.0

; OS�o�[�W��������
MinVersion=0,5.0

[Languages]
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"

[Types]
Name: all;                Description: "All"
Name: editorwithhelp;     Description: "Editor with Help"
Name: editoronly;         Description: "Editor Only"
Name: custom;             Description: "�J�X�^��"; Flags: iscustom

[Components]
Name: main;        Description: "�T�N���G�f�B�^�{��";             Types: all editorwithhelp editoronly custom; Flags: fixed
Name: help;        Description: "�w���v�t�@�C��";                 Types: all editorwithhelp
Name: keyword;     Description: "KeyWord�t�@�C��";                Types: all

[Tasks]
Name: startmenu;   Description: "�X�^�[�g���j���[���쐬(&S)";         Components: main;
Name: quicklaunch; Description: "Quick Launch�ɃA�C�R���쐬(&Q)";     Components: main;
Name: proglist;    Description: "�v���O�����ꗗ�ɒǉ�(&P)"; Components: main;
Name: desktopicon; Description: "�f�X�N�g�b�v�ɃA�C�R���쐬(&D)";     Components: main; Flags: unchecked;
Name: fileassoc;   Description: "�uSAKURA�ŊJ���v���j���[�̒ǉ�(&E)"; Components: main; Flags: unchecked;
Name: startup;     Description: "�N�����ɏ풓(&B)";                   Components: main; Flags: unchecked;
Name: sendto;      Description: "����ɒǉ�(&T)";                     Components: main; Flags: unchecked;

[Files]
Source: "sakura\sakura.exe";           DestDir: "{app}";         Components: main; Flags: ignoreversion;
Source: "sakura\sakura_lang_en_US.dll";DestDir: "{app}";         Components: main; Flags: ignoreversion;
Source: "sakura\bregonig.dll";         DestDir: "{app}";         Components: main
Source: "sakura\bsd_license.txt";      DestDir: "{app}";         Components: main
Source: "sakura\perl_license.txt";     DestDir: "{app}";         Components: main
Source: "sakura\perl_license_jp.txt";  DestDir: "{app}";         Components: main
Source: "sakura\sakura.exe.manifest.x";DestDir: "{app}";         Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserDisabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.exe.manifest.v";DestDir: "{app}";         Components: main; DestName: "sakura.exe.manifest"; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;
Source: "sakura\sakura.chm";           DestDir: "{app}";         Components: help
Source: "sakura\macro.chm";            DestDir: "{app}";         Components: help
Source: "sakura\plugin.chm";           DestDir: "{app}";         Components: help
Source: "sakura\sakura.exe.ini";       DestDir: "{app}";         Components: main; Check: isMultiUserEnabled; Flags: onlyifdoesntexist;

Source: "sakura\keyword\*"; DestDir: "{app}\keyword"; Components: keyword; Flags: recursesubdirs

[Registry]
; registry for all user (Admin only)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: ""; ValueData: "SAKURA�ŊJ��(&E)";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(true)
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist; Flags: uninsdeletekey; Check: CheckPrivilege(true)

; registry for each user (non-Admin only)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor";         ValueType: string; ValueName: ""; ValueData: "SAKURA�ŊJ��(&E)";            Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\*\shell\sakuraeditor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: fileassoc; Flags: uninsdeletekey; Check: CheckPrivilege(false)
Root: HKCU; Subkey: "SOFTWARE\Classes\Applications\sakura.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\sakura.exe"" ""%1"""; Tasks: proglist; Flags: uninsdeletekey; Check: CheckPrivilege(false)

[Icons]
Name: "{group}\�T�N���G�f�B�^";                                                Filename: "{app}\sakura.exe";                         Components: main; Tasks: startmenu;
Name: "{group}\�w���v�t�@�C��";                                                Filename: "{app}\sakura.chm";                         Components: help; Tasks: startmenu;
Name: "{group}\�ݒ�t�H���_";                                                  Filename: "%APPDATA%\sakura";                         Components: main; Check: isMultiUserEnabled; Tasks: startmenu;
Name: "{userdesktop}\�T�N���G�f�B�^";                                          Filename: "{app}\sakura.exe";                         Components: main; Tasks: desktopicon;
Name: "{group}\�A���C���X�g�[��";                                              Filename: "{uninstallexe}";                           Tasks: startmenu;
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\�T�N���G�f�B�^"; Filename: "{app}\sakura.exe";                         Components: main; Tasks: quicklaunch;
Name: "{userstartup}\�T�N���G�f�B�^�풓";                                      Filename: "{app}\sakura.exe";   Parameters: "-NOWIN"; Components: main; Tasks: startup;
Name: "{sendto}\�T�N���G�f�B�^";                                               Filename: "{app}\sakura.exe";                         Components: main; Tasks: sendto;

[Run]
FileName: "{app}\sakura.exe"; Description: "�������T�N���G�f�B�^���N��"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent; Check: CheckPrivilege(false);

[UninstallDelete]
;Uninstall���Ɋm�F�����������̂ŃR�����g�A�E�g
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
{          'Program Files���̃V�X�e���t�H���_�փC���X�g�[������ꍇ��UAC�𖳌��ɂ��Ȃ��Ɛݒ肪�ۑ��ł��܂��񂪂�낵���ł����H',}
          '�݊����[�h�ŃC���X�g�[�����܂��D�V�X�e���t�@�C������уv���O���������[�U�����ŕҏW����ƕҏW���ʂ����̃A�v���P�[�V��������Q�Ƃł��Ȃ����Ƃ�����܂��D(VirtualStore�@�\)',
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
  MultiUserPage := CreateInputOptionPage( wpSelectComponents, '�ݒ�ۑ����@�̑I��',
    '�ݒ�����[�U���ɕۑ����邩���s�t�@�C���̏ꏊ�֕ۑ����邩��I�����Ă�������',
    '�悭�킩��Ȃ��ꍇ�͕ύX���Ȃ��ł�������', False, False );
  MultiUserPage.Add( '�ݒ�����[�U���ɌʂɊǗ�����' );
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
  MemoMuitiUser := '�ݒ�t�@�C�����i�[�ꏊ' + NewLine + Space;
  MemoVirtualStore := 'VirtualStore' + NewLine + Space;

  if isMultiUserEnabled then
    begin
      MemoMuitiUser := MemoMuitiUser + '���[�U�ʃf�B���N�g��';
      MemoVirtualStore := MemoVirtualStore + '����';
    end
  else
    begin
      MemoMuitiUser := MemoMuitiUser + '���s�t�@�C���Ɠ���f�B���N�g��';
      MemoVirtualStore := MemoVirtualStore + '�L��';
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
