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
#ifndef SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_
#define SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_

#include "plugin/CPlugin.h"
#include <list>
#include <string>

class CPluginManager : public TSingleton<CPluginManager>{
	// �^��`
private:
	typedef std::wstring wstring;
	typedef std::string string;
public:
	friend class TSingleton<CPluginManager>;

	// �R���X�g���N�^
protected:
	CPluginManager();

	// �f�X�g���N�^
	// Singleton�̂��ߌĂ΂�Ȃ��B

	// ����
public:
	bool LoadAllPlugin();				//�S�v���O�C����ǂݍ���
	void UnloadAllPlugin();				//�S�v���O�C�����������
	bool SearchNewPlugin( CommonSetting& common, HWND hWndOwner );		//�V�K�v���O�C���𓱓�����
	int InstallPlugin( CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, wstring& errorMsg, bool bUpdate = false );	//�v���O�C���̏�������������
	bool InstZipPlugin( CommonSetting& common, HWND hWndOwner, tstring sZipName, bool bInSearch=false );		//Zip�v���O�C����ǉ�����
	CPlugin* GetPlugin( int id );		//�v���O�C�����擾����
	void UninstallPlugin( CommonSetting& common, int id );		//�v���O�C�����폜����

private:
	CPlugin* LoadPlugin( const TCHAR* pszPluginDir, const TCHAR* pszPluginName );	//�v���O�C����ǂݍ���
	bool RegisterPlugin( CPlugin* plugin );	//�v���O�C����CJackManager�ɓo�^����

	//����
public:
	//plugins�t�H���_�̃p�X
	const tstring GetBaseDir() { return m_sBaseDir; }
	const tstring GetExePluginDir() { return m_sExePluginDir; }
	bool SearchNewPluginDir( CommonSetting& common, HWND hWndOwner, tstring sSearchDir, bool& bCancel );		//�V�K�v���O�C����ǉ�����(������)
	bool SearchNewPluginZip( CommonSetting& common, HWND hWndOwner, tstring sSearchDir, bool& bCancel );		//�V�K�v���O�C����ǉ�����(������)Zip File
	bool InstZipPluginSub( CommonSetting& common, HWND hWndOwner, tstring sZipName, tstring sDispName, bool bInSearch, bool& bCancel );		//Zip�v���O�C���𓱓�����(������)

	// �����o�ϐ�
private:
	CPlugin::List m_plugins;
	tstring m_sBaseDir;					//plugins�t�H���_�̃p�X
	tstring m_sExePluginDir;			//Exe�t�H���_�z��plugins�t�H���_�̃p�X

};

#endif /* SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_ */
/*[EOF]*/
