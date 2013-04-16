/*!	@file
	@brief �v���O�C���Ǘ��N���X

*/
/*
	Copyright (C) 2009, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "plugin/CWSHPlugin.h"
#include "plugin/CDllPlugin.h"
#include "util/module.h"
#include "io/CZipFile.h"

//�R���X�g���N�^
CPluginManager::CPluginManager()
{

	//plugins�t�H���_�̏ꏊ���擾
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir( szPluginPath, _T("plugins\\") );	//ini�Ɠ����K�w��plugins�t�H���_������
	m_sBaseDir.append(szPluginPath);

	//Exe�t�H���_�z��plugins�t�H���_�̃p�X���擾
	TCHAR	szPath[_MAX_PATH];
	TCHAR	szFolder[_MAX_PATH];
	TCHAR	szFname[_MAX_PATH];

	::GetModuleFileName( NULL, szPath, _countof(szPath)	);
	SplitPath_FolderAndFile(szPath, szFolder, szFname);
	Concat_FolderAndFile(szFolder, _T("plugins\\"), szPluginPath);

	m_sExePluginDir.append(szPluginPath);
}

//�S�v���O�C�����������
void CPluginManager::UnloadAllPlugin()
{
	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		delete *it;
	}
	
	// 2010.08.04 Moca m_plugins.claer����
	// �������ACJackManager��UnRegisterPlug���Ȃ��̂ōēǂݍ��݂���Ƃ��������Ȃ�܂�
	m_plugins.clear();
}

//�V�K�v���O�C����ǉ�����
bool CPluginManager::SearchNewPlugin( CommonSetting& common, HWND hWndOwner )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPlugin\n"));
#endif

	HANDLE hFind;
	CZipFile	cZipFile;


	//�v���O�C���t�H���_�̔z��������
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//�v���O�C���t�H���_�����݂��Ȃ�
		if (!CreateDirectory(m_sBaseDir.c_str(), NULL)) {
			InfoMessage( hWndOwner, _T("%s"), _T("�v���O�C���t�H���_���쐬�o���܂���"));
			return true;
		}
	}
	::FindClose(hFind);

	//�v���O�C���t�H���_�̔z��������
	bool bFindNewDir = SearchNewPluginDir(common, hWndOwner, m_sBaseDir);
	if (m_sBaseDir != m_sExePluginDir) {
		bFindNewDir |= SearchNewPluginDir(common, hWndOwner, m_sExePluginDir);
	}
	if (cZipFile.IsOk()) {
		bFindNewDir |= SearchNewPluginZip(common, hWndOwner, m_sBaseDir);
		if (m_sBaseDir != m_sExePluginDir) {
			bFindNewDir |= SearchNewPluginZip(common, hWndOwner, m_sExePluginDir);
		}
	}

	if(!bFindNewDir){
		InfoMessage( hWndOwner, _T("%s"), _T("�V�����v���O�C���͌�����܂���ł���"));
	}

	return true;
}


//�V�K�v���O�C����ǉ�����(������)
bool CPluginManager::SearchNewPluginDir( CommonSetting& common, HWND hWndOwner, tstring sSearchDir )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginDir\n"));
#endif

	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	HANDLE hFind;

	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (sSearchDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//�v���O�C���t�H���_�����݂��Ȃ�
		return false;
	}
	bool bFindNewDir = false;
	do {
		if( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			(wf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 &&
			_tcscmp(wf.cFileName, _T("."))!=0 && _tcscmp(wf.cFileName, _T(".."))!=0 &&
			auto_stricmp(wf.cFileName, _T("unuse")) !=0 )
		{
			//�C���X�g�[���ς݃`�F�b�N�B�t�H���_�����v���O�C���e�[�u���̖��O�Ȃ�C���X�g�[�����Ȃ�
			// 2010.08.04 �啶�����������ꎋ�ɂ���
			bool isNotInstalled = true;
			for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( auto_stricmp( wf.cFileName, to_tchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if( !isNotInstalled ){ continue; }

			// 2011.08.20 syat plugin.def�����݂��Ȃ��t�H���_�͔�΂�
			if( ! IsFileExists( (sSearchDir + wf.cFileName + _T("\\") + PII_FILENAME).c_str(), true ) ){
				continue;
			}

			bFindNewDir = true;
			TCHAR msg[512];
			auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���u%ts�v���C���X�g�[�����܂����H"), wf.cFileName );
			if( ConfirmMessage( hWndOwner, _T("%s"), msg ) == IDYES ){
				std::wstring errMsg;
				int pluginNo = InstallPlugin( common, wf.cFileName, hWndOwner, errMsg );
				if( pluginNo < 0 ){
					auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���u%ts�v���C���X�g�[���ł��܂���ł���\n���R�F%ls"), wf.cFileName, errMsg.c_str() );
					WarningMessage( hWndOwner, _T("%s"), msg );
				}
			}
		}
	} while( FindNextFile( hFind, &wf ));

	FindClose( hFind );
	return bFindNewDir;
}


//�V�K�v���O�C����ǉ�����(������)Zip File
bool CPluginManager::SearchNewPluginZip( CommonSetting& common, HWND hWndOwner, tstring sSearchDir )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginZip\n"));
#endif

	HANDLE hFind;

	WIN32_FIND_DATA wf;
	bool		bNewPlugin = false;
	bool		bFound;
	CZipFile	cZipFile;

	hFind = INVALID_HANDLE_VALUE;

	// Zip File ������
	if (cZipFile.IsOk()) {
		hFind = FindFirstFile( (sSearchDir + _T("*.zip")).c_str(), &wf );

		for (bFound = (hFind != INVALID_HANDLE_VALUE); bFound; bFound = (FindNextFile( hFind, &wf ) != 0)) {
			if( (wf.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) == 0)
			{
				bNewPlugin |= InstZipPluginSub(common, hWndOwner, sSearchDir + wf.cFileName, wf.cFileName, true);
			}
		}
	}

	FindClose( hFind );
	return bNewPlugin;
}


//Zip�v���O�C���𓱓�����
bool CPluginManager::InstZipPlugin( CommonSetting& common, HWND hWndOwner, tstring sZipFile, bool bInSearch )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Entry InstZipPlugin\n"));
#endif

	CZipFile		cZipFile;
	TCHAR			msg[512];

	// ZIP�t�@�C���������邩
	if (!cZipFile.IsOk()) {
		auto_snprintf_s( msg, _countof(msg), _T("ZIP�t�@�C���͈����܂���") );
		InfoMessage( hWndOwner, _T("%s"), msg);
		return false;
	}

	//�v���O�C���t�H���_�̑��݂��m�F
	WIN32_FIND_DATA wf;
	HANDLE		hFind;
	if ((hFind = ::FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf )) == INVALID_HANDLE_VALUE) {
		//�v���O�C���t�H���_�����݂��Ȃ�
		if (m_sBaseDir == m_sExePluginDir) {
			auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���t�H���_������܂���") );
			InfoMessage( hWndOwner, _T("%s"), msg);
			::FindClose(hFind);
			return false;
		}
		else {
			if (!CreateDirectory(m_sBaseDir.c_str(), NULL)) {
				auto_strcpy_s( msg, _countof(msg), _T("�v���O�C���t�H���_���쐬�o���܂���") );
				WarningMessage( hWndOwner, _T("%s"), msg);
				::FindClose(hFind);
				return false;
			}
		}
	}
	::FindClose(hFind);

	return CPluginManager::InstZipPluginSub( common, hWndOwner, sZipFile, sZipFile, false );
}

//Zip�v���O�C���𓱓�����(������)
bool CPluginManager::InstZipPluginSub( CommonSetting& common, HWND hWndOwner, tstring sZipFile, tstring sDispName, bool bInSearch )
{
	PluginRec*		plugin_table = common.m_sPlugin.m_PluginTable;
	CZipFile		cZipFile;
	std::tstring	sFolderName;
	TCHAR			msg[512];
	std::wstring	errMsg;
	bool			bOk = true;
	bool			bSkip = false;
	bool			bNewPlugin = false;

	// Plugin �t�H���_���̎擾,��`�t�@�C���̊m�F
	if (bOk && !cZipFile.SetZip(sZipFile)) {
		auto_snprintf_s( msg, _countof(msg), _T("ZIP�t�@�C���u%ts�v�ɃA�N�Z�X�o���܂���"), sDispName.c_str() );
		bOk = false;
		bSkip = bInSearch;
	}

	// Plgin �t�H���_���̎擾,��`�t�@�C���̊m�F
	if (bOk && !cZipFile.ChkPluginDef(PII_FILENAME, sFolderName)) {
		auto_snprintf_s( msg, _countof(msg), _T("ZIP�t�@�C���u%ts�v�ɂ̓v���O�C����`�t�@�C���iplugin.def�j������܂���"), sDispName.c_str() );
		bOk = false;
		bSkip = bInSearch;
	}

	if (!bInSearch) {
		// �P�ƃC���X�g�[��
		//�C���X�g�[���ς݃`�F�b�N�B
		bool	isNotInstalled = true;
		int		iNo;
		if (bOk) {
			for( iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( auto_stricmp( to_wchar( sFolderName.c_str()), to_wchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if (isNotInstalled) {
				bNewPlugin = true;
			}
			else {
				auto_snprintf_s( msg, _countof(msg), _T("�u%ts�v�͊��ɃC���X�g�[������Ă��܂�\n�㏑�����܂����H"), sDispName.c_str());
				if( ConfirmMessage( hWndOwner, _T("%s"), msg ) != IDYES ){
					// Yes�Ŗ����Ȃ�I��
					return false;
				}
			}
		}
	}
	else {
		// plugins�t�H���_������
		// �t�H���_ �`�F�b�N�B���łɉ𓀂���Ă����Ȃ�C���X�g�[�����Ȃ�(�O�i�ŃC���X�g�[���ς݈��͉ۂ��m�F�ς�)
		if (bOk && fexist(to_tchar((m_sBaseDir + to_tchar(sFolderName.c_str())).c_str()))
			|| fexist(to_tchar((m_sExePluginDir + to_tchar(sFolderName.c_str())).c_str())) ) {
			bOk = false;
			bSkip = true;
		}
		if (bOk) {
			bNewPlugin= true;
			auto_snprintf_s( msg, _countof(msg), _T("ZIP�v���O�C���u%ts�v���u%ts�v�ɃC���X�g�[�����܂����H"), sDispName.c_str(), sFolderName.c_str());
			if( ConfirmMessage( hWndOwner, _T("%s"), msg ) != IDYES ){
				bOk = false;
				bSkip = true;
			}
		}
	}

	// Zip��
	if (bOk && !cZipFile.Unzip(m_sBaseDir)) {
		auto_snprintf_s( msg, _countof(msg), _T("ZIP�v���O�C���u%ts�v�̉𓀂Ɏ��s���܂���"), sDispName.c_str() );
		bOk = false;
	}
	if (bOk) {
		int pluginNo = InstallPlugin( common, to_tchar(sFolderName.c_str()), hWndOwner, errMsg, true );
		if( pluginNo < 0 ){
			auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���u%ts�v���C���X�g�[���ł��܂���ł���\n���R�F%ls"), sDispName.c_str(), errMsg.c_str() );
			bOk = false;
		}
	}

	if (!bOk && !bSkip) {
		// �G���[���b�Z�[�W�o��
		WarningMessage( hWndOwner, _T("%s"), msg);
	}

	return bNewPlugin;
}

//�v���O�C���̏�������������
//	common			���L�ݒ�ϐ�
//	pszPluginName	�v���O�C����
//	hWndOwner		
//	errorMsg		�G���[���b�Z�[�W��Ԃ�
//	bUodate			���łɓo�^���Ă����ꍇ�A�m�F�����㏑������
int CPluginManager::InstallPlugin( CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, std::wstring& errorMsg, bool bUpdate )
{
	CDataProfile cProfDef;				//�v���O�C����`�t�@�C��

	//�v���O�C����`�t�@�C����ǂݍ���
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( (m_sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() )
		&& !cProfDef.ReadProfile( (m_sExePluginDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() ) ){
		errorMsg = L"�v���O�C����`�t�@�C���iplugin.def�j������܂���";
		return -1;
	}

	std::wstring sId;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_ID, sId );
	if( sId.length() == 0 ){
		errorMsg = L"Plugin.ID������܂���";
		return -1;
	}
	//2010.08.04 ID�g�p�s�̕������m�F
	//  ��X�t�@�C������ini�Ŏg�����Ƃ��l���Ă��������ۂ���
	const WCHAR szReservedChars[] = L"/\\,[]*?<>&|;:=\" \t";
	for( int x = 0; x < _countof(szReservedChars); ++x ){
		if( sId.npos != sId.find(szReservedChars[x]) ){
			errorMsg = std::wstring(L"Plugin.ID��\"") + szReservedChars + L"\"�͎g�p�ł��܂���";
			return -1;
		}
	}
	if( WCODE::Is09(sId[0]) ){
		errorMsg = L"Plugin.ID�̐擪�ɐ����͎g�p�ł��܂���";
		return -1;
	}

	//ID�d���E�e�[�u���󂫃`�F�b�N
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( nEmpty == -1 && plugin_table[iNo].m_state == PLS_NONE ){
			nEmpty = iNo;
			// break ���Ă͂����Ȃ��B���œ���ID�����邩��
		}
		if( wcscmp( sId.c_str(), plugin_table[iNo].m_szId ) == 0 ){	//ID��v
			if (!bUpdate) {
				const TCHAR* msg = _T("�����v���O�C�����ʂ̖��O�ŃC���X�g�[������Ă��܂��B�㏑�����܂����H\n�@�͂��@���@�V�����u%ts�v���g�p\n�@���������@�C���X�g�[���ς݂́u%ls�v���g�p");
				// 2010.08.04 �폜����ID�͌��̈ʒu�֒ǉ�(����������)
				if( plugin_table[iNo].m_state != PLS_DELETED &&
				  ConfirmMessage( hWndOwner, msg, static_cast<const TCHAR*>(pszPluginName), static_cast<const WCHAR*>(plugin_table[iNo].m_szName) ) != IDYES ){
					errorMsg = L"���[�U�L�����Z��";
					return -1;
				}
			}
			nEmpty = iNo;
			isDuplicate = plugin_table[iNo].m_state != PLS_DELETED;
			break;
		}
	}

	if( nEmpty == -1 ){
		errorMsg = L"�v���O�C��������ȏ�o�^�ł��܂���";
		return -1;
	}

	wcsncpy( plugin_table[nEmpty].m_szName, to_wchar(pszPluginName), MAX_PLUGIN_NAME );
	plugin_table[nEmpty].m_szName[ MAX_PLUGIN_NAME-1 ] = '\0';
	wcsncpy( plugin_table[nEmpty].m_szId, sId.c_str(), MAX_PLUGIN_ID );
	plugin_table[nEmpty].m_szId[ MAX_PLUGIN_ID-1 ] = '\0';
	plugin_table[nEmpty].m_state = isDuplicate ? PLS_UPDATED : PLS_INSTALLED;

	// �R�}���h���̐ݒ�	2010/7/11 Uchi
	int			i;
	WCHAR		szPlugKey[10];
	wstring		sPlugCmd;

	plugin_table[nEmpty].m_nCmdNum = 0;
	for (i = 1; i < MAX_PLUG_CMD; i++) {
		auto_sprintf( szPlugKey, L"C[%d]", i);
		sPlugCmd.clear();
		cProfDef.IOProfileData( PII_COMMAND, szPlugKey, sPlugCmd );
		if (sPlugCmd == L"") {
			break;
		}
		plugin_table[nEmpty].m_nCmdNum = i;
	}

	return nEmpty;
}

//�S�v���O�C����ǂݍ���
bool CPluginManager::LoadAllPlugin()
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter LoadAllPlugin\n"));
#endif

	if( ! GetDllShareData().m_Common.m_sPlugin.m_bEnablePlugin ) return true;

	//�v���O�C���e�[�u���ɓo�^���ꂽ�v���O�C����ǂݍ���
	PluginRec* plugin_table = GetDllShareData().m_Common.m_sPlugin.m_PluginTable;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( plugin_table[iNo].m_szName[0] == '\0' ) continue;
		// 2010.08.04 �폜��Ԃ�����(���̂Ƃ���ی�)
		if( plugin_table[iNo].m_state == PLS_DELETED ) continue;
		std::tstring name = to_tchar(plugin_table[iNo].m_szName);
		CPlugin* plugin = LoadPlugin( m_sBaseDir.c_str(), name.c_str() );
		if( !plugin ){
			plugin = LoadPlugin( m_sExePluginDir.c_str(), name.c_str() );
		}
		if( plugin ){
			// �v�����Fplugin.def��id��sakuraw.ini��id�̕s��v����
			assert_warning( 0 == auto_strcmp( plugin_table[iNo].m_szId, plugin->m_sId.c_str() ) );
			plugin->m_id = iNo;		//�v���O�C���e�[�u���̍s�ԍ���ID�Ƃ���
			m_plugins.push_back( plugin );
			plugin_table[iNo].m_state = PLS_LOADED;
			// �R�}���h���ݒ�
			plugin_table[iNo].m_nCmdNum = plugin->GetCommandCount();
		}
	}

	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		RegisterPlugin( *it );
	}
	
	return true;
}

//�v���O�C����ǂݍ���
CPlugin* CPluginManager::LoadPlugin( const TCHAR* pszPluginDir, const TCHAR* pszPluginName )
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	CDataProfile cProfDef;				//�v���O�C����`�t�@�C��
	CDataProfile cProfOption;			//�I�v�V�����t�@�C��
	CPlugin* plugin = NULL;

#ifdef _UNICODE
	DEBUG_TRACE(_T("Load Plugin %ts\n"),  pszPluginName );
#endif
	//�v���O�C����`�t�@�C����ǂݍ���
	Concat_FolderAndFile( pszPluginDir, pszPluginName, pszBasePath );
	Concat_FolderAndFile( pszBasePath, PII_FILENAME, pszPath );
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( pszPath ) ){
		//�v���O�C����`�t�@�C�������݂��Ȃ�
		return NULL;
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  ��`�t�@�C���Ǎ� %ts\n"),  pszPath );
#endif

	std::wstring sPlugType;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_PLUGTYPE, sPlugType );

	if( wcsicmp( sPlugType.c_str(), L"wsh" ) == 0 ){
		plugin = new CWSHPlugin( tstring(pszBasePath) );
	}else if( wcsicmp( sPlugType.c_str(), L"dll" ) == 0 ){
		plugin = new CDllPlugin( tstring(pszBasePath) );
	}else{
		return NULL;
	}
	plugin->m_sOptionDir = m_sBaseDir + pszPluginName;
	plugin->ReadPluginDef( &cProfDef );
#ifdef _UNICODE
	DEBUG_TRACE(_T("  �v���O�C���^�C�v %ls\n"), sPlugType.c_str() );
#endif

	//�I�v�V�����t�@�C����ǂݍ���
	cProfOption.SetReadingMode();
	if( cProfOption.ReadProfile( plugin->GetOptionPath().c_str() ) ){
		//�I�v�V�����t�@�C�������݂���ꍇ�A�ǂݍ���
		plugin->ReadPluginOption( &cProfOption );
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  �I�v�V�����t�@�C���Ǎ� %ts\n"),  pszPath );
#endif

	return plugin;
}

//�v���O�C����CJackManager�ɓo�^����
bool CPluginManager::RegisterPlugin( CPlugin* plugin )
{
	CJackManager* pJackMgr = CJackManager::getInstance();
	CPlug::Array plugs = plugin->GetPlugs();

	for( CPlug::ArrayIter plug = plugs.begin() ; plug != plugs.end(); plug++ ){
		pJackMgr->RegisterPlug( (*plug)->m_sJack.c_str(), *plug );
	}

	return true;
}

//�v���O�C�����擾����
CPlugin* CPluginManager::GetPlugin( int id )
{
	for( CPlugin::ListIter plugin = m_plugins.begin() ; plugin != m_plugins.end(); plugin++ ){
		if( (*plugin)->m_id == id ) return *plugin;
	}
	return NULL;
}

//�v���O�C�����폜����
void CPluginManager::UninstallPlugin( CommonSetting& common, int id )
{
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;

	// 2010.08.04 �����ł�ID��ێ�����B��ōēx�ǉ�����Ƃ��ɓ����ʒu�ɒǉ�
	// PLS_DELETED��m_szId/m_szName��ini��ۑ�����ƍ폜����܂�
//	plugin_table[id].m_szId[0] = '\0';
	plugin_table[id].m_szName[0] = '\0';
	plugin_table[id].m_state = PLS_DELETED;
	plugin_table[id].m_nCmdNum = 0;
}
