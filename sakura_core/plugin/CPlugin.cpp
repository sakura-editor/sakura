/*!	@file
	@brief �v���O�C����{�N���X

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
#include "CPlugin.h"
#include "CJackManager.h"

/////////////////////////////////////////////
// CPlug �����o�֐�
bool CPlug::Invoke( CEditView* view, CWSHIfObj::List& params ){
	return m_cPlugin.InvokePlug( view, *this, params );
}

EFunctionCode CPlug::GetFunctionCode() const{
	return static_cast<EFunctionCode>(m_cPlugin.m_id * 100 + m_id + F_PLUGCOMMAND_FIRST);
}

/////////////////////////////////////////////
// CPlugin �����o�֐�

//�R���X�g���N�^
CPlugin::CPlugin( tstring sBaseDir )
	: m_sBaseDir( sBaseDir )
{
	m_nCommandCount = 0;
}

//�f�X�g���N�^
CPlugin::~CPlugin(void)
{
}

//�v���O�C����`�t�@�C����Common�Z�N�V������ǂݍ���
bool CPlugin::ReadPluginDefCommon( CDataProfile *cProfile )
{
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_ID, m_sId );
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_NAME, m_sName );
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_DESCRIPTION, m_sDescription );
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_AUTHOR, m_sAuthor );
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_VERSION, m_sVersion );
	cProfile->IOProfileData( PII_PLUGIN, PII_PLUGIN_URL, m_sUrl );

#if _DEBUG & _UNICODE
	DebugOut(_T("    Name:%ls\n"), m_sName.c_str());
	DebugOut(_T("    Description:%ls\n"), m_sDescription.c_str());
	DebugOut(_T("    Author:%ls\n"), m_sAuthor.c_str());
	DebugOut(_T("    Version:%ls\n"), m_sVersion.c_str());
	DebugOut(_T("    Url:%ls\n"), m_sUrl.c_str());
#endif

	return true;
}

//�v���O�C����`�t�@�C����Plug�Z�N�V������ǂݍ���
bool CPlugin::ReadPluginDefPlug( CDataProfile *cProfile )
{
	unsigned int i;
	std::vector<JackDef> jacks = CJackManager::Instance()->GetJackDef();
	wstring sKey;
	wstring sHandler;
	wstring sLabel;

	for( i=0; i<jacks.size(); i++ ){
		sKey = jacks[i].szName;
		if( cProfile->IOProfileData( PII_PLUG, sKey.c_str(), sHandler ) ){
			//���x���̎擾
			sKey += L".Label";
			cProfile->IOProfileData( PII_PLUG, sKey.c_str(), sLabel );

			CPlug *newPlug = CreatePlug( *this, 0, jacks[i].szName, sHandler, sLabel );
			m_plugs.push_back( newPlug );
		}
	}

	return true;
}

//�v���O�C����`�t�@�C����Command�Z�N�V������ǂݍ���
bool CPlugin::ReadPluginDefCommand( CDataProfile *cProfile )
{
	wstring sHandler;
	WCHAR bufKey[64];

	for( int nCount = 1; nCount < 100; nCount++ ){	//�Y�����͂P����n�߂�
		swprintf( bufKey, L"C[%d]", nCount );
		if( cProfile->IOProfileData( PII_COMMAND, bufKey, sHandler ) ){
			wstring sLabel;
			wstring sIcon;

			//���x���̎擾
			swprintf( bufKey, L"C[%d].Label", nCount );
			cProfile->IOProfileData( PII_COMMAND, bufKey, sLabel );
			//�A�C�R���̎擾
			swprintf( bufKey, L"C[%d].Icon", nCount );
			cProfile->IOProfileData( PII_COMMAND, bufKey, sIcon );

			AddCommand( sHandler.c_str(), sLabel.c_str(), sIcon.c_str(), false );
		}else{
			break;		//��`���Ȃ���Γǂݍ��݂��I��
		}
	}

	return true;
}

//�v���O�C���t�H���_��̑��΃p�X���t���p�X�ɕϊ�
CPlugin::tstring CPlugin::GetFilePath( const tstring& sFileName ) const
{
	return m_sBaseDir + _T("\\") + to_tchar( sFileName.c_str() );
}

//�R�}���h��ǉ�����
int CPlugin::AddCommand( const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister )
{
	if( !handler ){ handler = L""; }
	if( !label ){ label = L""; }

	//�R�}���h�v���OID��1����U��
	m_nCommandCount++;
	CPlug *newPlug = CreatePlug( *this, m_nCommandCount, PP_COMMAND_STR, wstring(handler), wstring(label) );
	if( icon ){
		newPlug->m_sIcon = icon;
	}

	m_plugs.push_back( newPlug );

	if( doRegister ){
		CJackManager::Instance()->RegisterPlug( PP_COMMAND_STR, newPlug );
	}
	return newPlug->GetFunctionCode();
}
