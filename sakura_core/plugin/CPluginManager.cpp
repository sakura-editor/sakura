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
#include "stdafx.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "plugin/CWSHPlugin.h"
#include "plugin/CDllPlugin.h"
#include "util/module.h"

//�R���X�g���N�^
CPluginManager::CPluginManager()
{

	//plugins�t�H���_�̏ꏊ���擾
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir( szPluginPath, _T("plugins\\") );	//ini�Ɠ����K�w��plugins�t�H���_������
	m_sBaseDir.append(szPluginPath);
}

//�S�v���O�C�����������
void CPluginManager::UnloadAllPlugin()
{
	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		delete *it;
	}
}

//�V�K�v���O�C����ǉ�����
bool CPluginManager::SearchNewPlugin( CommonSetting& common, HWND hWndOwner )
{
#if _DEBUG & _UNICODE
	DebugOut(L"Enter ControlProcessInit\n");
#endif

	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	HANDLE hFind;


	//�v���O�C���t�H���_�̔z��������
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//�v���O�C���t�H���_�����݂��Ȃ�
		InfoMessage( hWndOwner, _T("%s"), _T("�v���O�C���t�H���_������܂���"));
		return true;
	}
	bool bFindNewDir = false;
	do {
		if( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(wf.cFileName, _T("."))!=0 && _tcscmp(wf.cFileName, _T(".."))!=0 )
		{
			//�C���X�g�[���ς݃`�F�b�N�B�t�H���_�����v���O�C���e�[�u���̖��O�Ȃ�C���X�g�[�����Ȃ�
			bool isNotInstalled = true;
			for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( _tcscmp( wf.cFileName, to_tchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if( !isNotInstalled ){ continue; }
			bFindNewDir = true;

			TCHAR msg[512];
			auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���u%ts�v���C���X�g�[�����܂����H"), wf.cFileName );
			if( ::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) == IDYES ){
				int pluginNo = InstallPlugin( common, wf.cFileName, hWndOwner );
				if( pluginNo < 0 ){
					auto_snprintf_s( msg, _countof(msg), _T("�v���O�C���u%ts�v���C���X�g�[���ł��܂���ł���\n���R�F%ls"), wf.cFileName, m_sInstallError.c_str() );
					::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_OK | MB_ICONERROR );
				}
			}
		}
	} while( FindNextFile( hFind, &wf ));
	
	if(!bFindNewDir){
		InfoMessage( hWndOwner, _T("%s"), _T("�V�����v���O�C���͌�����܂���ł���"));
	}

	FindClose( hFind );
	return true;
}

//�v���O�C���̏�������������
int CPluginManager::InstallPlugin( CommonSetting& common, TCHAR* pszPluginName, HWND hWndOwner )
{
	CDataProfile cProfDef;				//�v���O�C����`�t�@�C��

	//�v���O�C����`�t�@�C����ǂݍ���
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( (m_sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() ) ){
		m_sInstallError = L"�v���O�C����`�t�@�C���iplugin.def�j������܂���";
		return -1;
	}

	std::wstring sId;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_ID, sId );
	if( sId.length() == 0 ){
		m_sInstallError = L"Plugin.ID������܂���";
		return -1;
	}

	//ID�d���E�e�[�u���󂫃`�F�b�N
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( nEmpty == -1 && plugin_table[iNo].m_state == PLS_NONE ){
			nEmpty = iNo;
		}
		if( wcscmp( sId.c_str(), plugin_table[iNo].m_szId ) == 0 ){	//ID��v
			TCHAR msg[512];
			auto_snprintf_s( msg, _countof(msg), _T("�����v���O�C�����ʂ̖��O�ŃC���X�g�[������Ă��܂��B�㏑�����܂����H\n�@�͂��@���@�V�����u%ts�v���g�p\n�@���������@�C���X�g�[���ς݂́u%ls�v���g�p"), pszPluginName, plugin_table[iNo].m_szName );
			if( ::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) == IDYES ){
				nEmpty = iNo;
				isDuplicate = true;
				break;
			}else{
				m_sInstallError = L"���[�U�L�����Z��";
				return -1;
			}
		}
	}

	if( nEmpty == -1 ){
		m_sInstallError = L"�v���O�C��������ȏ�o�^�ł��܂���";
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
#if _DEBUG & _UNICODE
	DebugOut(L"Enter LoadAllPlugin\n");
#endif

	if( ! GetDllShareData().m_Common.m_sPlugin.m_bEnablePlugin ) return true;

	//�v���O�C���e�[�u���ɓo�^���ꂽ�v���O�C����ǂݍ���
	PluginRec* plugin_table = GetDllShareData().m_Common.m_sPlugin.m_PluginTable;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( plugin_table[iNo].m_szName[0] == '\0' ) continue;

		CPlugin* plugin = LoadPlugin( m_sBaseDir.c_str(), const_cast<TCHAR*>(to_tchar(plugin_table[iNo].m_szName)) );
		if( plugin ){
			plugin->m_id = iNo;		//�v���O�C���e�[�u���̍s�ԍ���ID�Ƃ���
			m_plugins.push_back( plugin );
			plugin_table[iNo].m_state = PLS_LOADED;
			// �R�}���h���ݒ�
			plugin_table[iNo].m_nCmdNum = plugin->GetCommandCount();
		}
	}

	m_plugins.sort();

	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		RegisterPlugin( *it );
	}
	
	return true;
}

//�v���O�C����ǂݍ���
CPlugin* CPluginManager::LoadPlugin( const TCHAR* pszPluginDir, TCHAR* pszPluginName )
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	CDataProfile cProfDef;				//�v���O�C����`�t�@�C��
	CDataProfile cProfOption;			//�I�v�V�����t�@�C��
	CPlugin* plugin = NULL;

#if _DEBUG & _UNICODE
	DebugOut(L"Load Plugin %ts\n",  pszPluginName );
#endif
	//�v���O�C����`�t�@�C����ǂݍ���
	Concat_FolderAndFile( pszPluginDir, pszPluginName, pszBasePath );
	Concat_FolderAndFile( pszBasePath, PII_FILENAME, pszPath );
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( pszPath ) ){
		//�v���O�C����`�t�@�C�������݂��Ȃ�
		return NULL;
	}
#if _DEBUG & _UNICODE
	DebugOut(L"  ��`�t�@�C���Ǎ� %ts\n",  pszPath );
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
	plugin->ReadPluginDef( &cProfDef );
#if _DEBUG & _UNICODE
	DebugOut(L"  �v���O�C���^�C�v %ls\n", sPlugType.c_str() );
#endif

	//�I�v�V�����t�@�C����ǂݍ���
	_tcscpy( pszPath, pszBasePath );
	_tcscat( pszPath, PII_OPTFILEEXT );
	cProfOption.SetReadingMode();
	if( cProfOption.ReadProfile( pszPath ) ){
		//�I�v�V�����t�@�C�������݂���ꍇ�A�ǂݍ���
		plugin->ReadPluginOption( &cProfOption );
	}
#if _DEBUG & _UNICODE
	DebugOut(L"  �I�v�V�����t�@�C���Ǎ� %ts\n",  pszPath );
#endif

	return plugin;
}

//�v���O�C����CJackManager�ɓo�^����
bool CPluginManager::RegisterPlugin( CPlugin* plugin )
{
	CJackManager* pJackMgr = CJackManager::Instance();
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

	plugin_table[id].m_szId[0] = '\0';
	plugin_table[id].m_szName[0] = '\0';
	plugin_table[id].m_state = PLS_DELETED;
	plugin_table[id].m_nCmdNum = 0;
}
