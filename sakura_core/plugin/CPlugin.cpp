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
#include "StdAfx.h"
#include <vector>		// wstring_split�p 2010/4/4 Uchi
#include "CPlugin.h"
#include "CJackManager.h"

/////////////////////////////////////////////
// CPlug �����o�֐�
bool CPlug::Invoke( CEditView* view, CWSHIfObj::List& params ){
	return m_cPlugin.InvokePlug( view, *this, params );
}

EFunctionCode CPlug::GetFunctionCode() const{
	return GetPluginFunctionCode(m_cPlugin.m_id, m_id);
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
	for( CPluginOption::ArrayIter it = m_options.begin(); it != m_options.end(); it++ ){
		delete *it;
	}
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

#ifdef _UNICODE
	DEBUG_TRACE(_T("    Name:%ls\n"), m_sName.c_str());
	DEBUG_TRACE(_T("    Description:%ls\n"), m_sDescription.c_str());
	DEBUG_TRACE(_T("    Author:%ls\n"), m_sAuthor.c_str());
	DEBUG_TRACE(_T("    Version:%ls\n"), m_sVersion.c_str());
	DEBUG_TRACE(_T("    Url:%ls\n"), m_sUrl.c_str());
#endif

	return true;
}

//�v���O�C����`�t�@�C����Plug�Z�N�V������ǂݍ���
// @date 2011.08.20 syat Plug�Z�N�V������������`�\�Ƃ���
bool CPlugin::ReadPluginDefPlug( CDataProfile *cProfile )
{
	unsigned int i;
	std::vector<JackDef> jacks = CJackManager::getInstance()->GetJackDef();
	wstring sKey;
	wstring sHandler;
	wstring sLabel;
	wchar_t szIndex[8];

	for( i=0; i<jacks.size(); i++ ){
		sKey = jacks[i].szName;
		for( int nCount = 0; nCount < MAX_PLUG_CMD; nCount++ ){
			if( nCount == 0 ){
				szIndex[0] = L'\0';
			}else{
				swprintf(szIndex, L"[%d]", nCount);
			}
			if( cProfile->IOProfileData( PII_PLUG, (sKey + szIndex).c_str(), sHandler ) ){
				//���x���̎擾
				cProfile->IOProfileData( PII_PLUG, (sKey + szIndex + L".Label").c_str(), sLabel );
				if (sLabel == L"") {
					sLabel = sHandler;		// Label��������΃n���h�����ő�p
				}

				CPlug *newPlug = CreatePlug( *this, nCount, jacks[i].szName, sHandler, sLabel );
				m_plugs.push_back( newPlug );
			}else{
				break;		//��`���Ȃ���Γǂݍ��݂��I��
			}
		}
	}

	return true;
}

//�v���O�C����`�t�@�C����Command�Z�N�V������ǂݍ���
bool CPlugin::ReadPluginDefCommand( CDataProfile *cProfile )
{
	wstring sHandler;
	WCHAR bufKey[64];

	for( int nCount = 1; nCount < MAX_PLUG_CMD; nCount++ ){	//�Y�����͂P����n�߂�
		swprintf( bufKey, L"C[%d]", nCount );
		if( cProfile->IOProfileData( PII_COMMAND, bufKey, sHandler ) ){
			wstring sLabel;
			wstring sIcon;

			//���x���̎擾
			swprintf( bufKey, L"C[%d].Label", nCount );
			cProfile->IOProfileData( PII_COMMAND, bufKey, sLabel );
			if (sLabel == L"") {
				sLabel = sHandler;		// Label��������΃n���h�����ő�p
			}
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

//�v���O�C����`�t�@�C����Option�Z�N�V������ǂݍ���	// 2010/3/24 Uchi
bool CPlugin::ReadPluginDefOption( CDataProfile *cProfile )
{
	wstring sLabel;
	wstring sSection;
	wstring sSection_wk;
	wstring sKey;
	wstring sType;
	wstring sSelect;
	wstring sDefaultVal;
	WCHAR bufKey[64];

	sSection = L"";
	for( int nCount = 1; nCount < MAX_PLUG_CMD; nCount++ ){	//�Y�����͂P����n�߂�
		sKey = sLabel = sType = sDefaultVal= L"";
		//Key�̎擾
		swprintf( bufKey, L"O[%d].Key", nCount );
		if( cProfile->IOProfileData( PII_OPTION, bufKey, sKey ) ){
			//Section�̎擾
			swprintf( bufKey, L"O[%d].Section", nCount );
			cProfile->IOProfileData( PII_OPTION, bufKey, sSection_wk );
			if (!sSection_wk.empty()) {		// �w�肪������ΑO�������p��
				sSection = sSection_wk;
			}
			//���x���̎擾
			swprintf( bufKey, L"O[%d].Label", nCount );
			cProfile->IOProfileData( PII_OPTION, bufKey, sLabel );
			//Type�̎擾
			swprintf( bufKey, L"O[%d].Type", nCount );
			cProfile->IOProfileData( PII_OPTION, bufKey, sType );
			// ���ڑI�����
			swprintf( bufKey, L"O[%d].Select", nCount );
			cProfile->IOProfileData( PII_OPTION, bufKey, sSelect );
			// �f�t�H���g�l
			swprintf( bufKey, L"O[%d].Default", nCount );
			cProfile->IOProfileData( PII_OPTION, bufKey, sDefaultVal );

			if (sSection.empty() || sKey.empty()) {
				// �ݒ肪���������疳��
				continue;
			}
			if (sLabel.empty()) {
				// Label�w�肪������΁AKey�ő�p
				sLabel = sKey;
			}

			m_options.push_back( new CPluginOption( this, sLabel, sSection, sKey, sType, sSelect, sDefaultVal, nCount ) );
		}
	}

	return true;
}

//�v���O�C���t�H���_��̑��΃p�X���t���p�X�ɕϊ�
CPlugin::tstring CPlugin::GetFilePath( const tstring& sFileName ) const
{
	return m_sBaseDir + _T("\\") + to_tchar( sFileName.c_str() );
}

CPlugin::tstring CPlugin::GetFolderName() const
{
	return tstring(GetFileTitlePointer(m_sBaseDir.c_str()));
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
		CJackManager::getInstance()->RegisterPlug( PP_COMMAND_STR, newPlug );
	}
	return newPlug->GetFunctionCode();
}

// �����񕪊�	2010/4/4 Uchi
//	�Ɨ��������ق��������̂���
std::vector<std::wstring> wstring_split( std::wstring sTrg, wchar_t cSep )
{
    std::vector<std::wstring>	splitVec;
    int 	idx;

    while ((idx = sTrg.find( cSep )) != std::wstring::npos) {
        splitVec.push_back( sTrg.substr( 0, idx ) );
        sTrg = sTrg.substr( ++idx );
    }
	if (sTrg != L"") {
		splitVec.push_back( sTrg );
	}

    return splitVec;
}
