/*!	@file
	@brief WSH�v���O�C���N���X

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
#include "plugin/CWSHPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CWSHManager.h"

// �f�X�g���N�^
CWSHPlugin::~CWSHPlugin(void)
{
	for( CPlug::ArrayIter it = m_plugs.begin(); it != m_plugs.end(); it++ ){
		delete *it;
	}
}

//�v���O�C����`�t�@�C����ǂݍ���
bool CWSHPlugin::ReadPluginDef( CDataProfile *cProfile )
{
	ReadPluginDefCommon( cProfile );

	//WSH�Z�N�V�����̓ǂݍ���
	cProfile->IOProfileData<bool>( PII_WSH, PII_WSH_USECACHE, m_bUseCache );

	//�v���O�̓ǂݍ���
	ReadPluginDefPlug( cProfile );

	//�R�}���h�̓ǂݍ���
	ReadPluginDefCommand( cProfile );

	//�I�v�V������`�̓ǂݍ���	// 2010/3/24 Uchi
	ReadPluginDefOption( cProfile );

	return true;
}

//�I�v�V�����t�@�C����ǂݍ���
bool CWSHPlugin::ReadPluginOption( CDataProfile *cProfile )
{
	return true;
}

//�v���O�����s����
bool CWSHPlugin::InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params )
{
	CWSHPlug& wshPlug = static_cast<CWSHPlug&>( plug );
	CWSHMacroManager* pWsh = NULL;

	if( !m_bUseCache || wshPlug.m_Wsh == NULL ){
		CFilePath path( plug.m_cPlugin.GetFilePath( to_tchar(plug.m_sHandler.c_str()) ).c_str() );

		pWsh = (CWSHMacroManager*)CWSHMacroManager::Creator( path.GetExt( true ) );
		if( pWsh == NULL ){ return false; }

		BOOL bLoadResult = pWsh->LoadKeyMacro( G_AppInstance(), path );
		if ( !bLoadResult ){
			ErrorMessage( NULL, _T("�}�N���̓ǂݍ��݂Ɏ��s���܂����B\n\n%ts"), static_cast<const TCHAR*>(path) );
			delete pWsh;
			return false;
		}

	}else{
		pWsh = wshPlug.m_Wsh;
	}

	CPluginIfObj cPluginIfo(*this);		//Plugin�I�u�W�F�N�g��ǉ�
	cPluginIfo.AddRef();
	cPluginIfo.SetPlugIndex( plug.m_id );	//���s���v���O�ԍ����
	pWsh->AddParam( &cPluginIfo );

	pWsh->AddParam( params );			//�p�����[�^��ǉ�

	pWsh->ExecKeyMacro( view, FA_NONRECORD | FA_FROMMACRO );

	pWsh->ClearParam();

	if( m_bUseCache ){
		wshPlug.m_Wsh = pWsh;
	}else{
		// �I�������J��
		delete pWsh;
	}

	return true;
}
