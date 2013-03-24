/*!	@file
	@brief DLL�v���O�C���N���X

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
#include "plugin/CDllPlugin.h"
#include "view/CEditView.h"

// �f�X�g���N�^
CDllPlugin::~CDllPlugin(void)
{
	for( CPlug::ArrayIter it = m_plugs.begin(); it != m_plugs.end(); it++ ){
		delete (CDllPlug*)(*it);
	}
}

// �v���O�̐���
// CPlug�̑����CDllPlug���쐬����
CPlug* CDllPlugin::CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
{
	CDllPlug *newPlug =  new CDllPlug( plugin, id, sJack, sHandler, sLabel );
	return newPlug;
}

// �v���O�C����`�t�@�C���̓ǂݍ���
bool CDllPlugin::ReadPluginDef( CDataProfile *cProfile )
{
	ReadPluginDefCommon( cProfile );

	//DLL���̓ǂݍ���
	cProfile->IOProfileData( PII_DLL, PII_DLL_NAME, m_sDllName );

	//�v���O�̓ǂݍ���
	ReadPluginDefPlug( cProfile );

	//�R�}���h�̓ǂݍ���
	ReadPluginDefCommand( cProfile );

	//�I�v�V������`�̓ǂݍ���	// 2010/3/24 Uchi
	ReadPluginDefOption( cProfile );

	return true;
}

// �v���O���s
bool CDllPlugin::InvokePlug( CEditView* view, CPlug& plug_raw, CWSHIfObj::List& params )
{
	tstring dllPath = GetFilePath( to_tchar(m_sDllName.c_str()) );
	EDllResult resInit = InitDll( to_tchar( dllPath.c_str() ) );
	if( resInit != DLL_SUCCESS ){
		::MYMESSAGEBOX( view->m_hwndParent, MB_OK, _T("DLL�v���O�C��"), _T("DLL�̓ǂݍ��݂Ɏ��s���܂���\n%ts\n%ls"), dllPath.c_str(), m_sName.c_str() );
		return false;
	}

	CDllPlug& plug = *(static_cast<CDllPlug*>(&plug_raw));
	if( ! plug.m_handler ){
		//DLL�֐��̎擾
		ImportTable imp[2] = {
			{ &plug.m_handler, to_achar( plug.m_sHandler.c_str() ) },
			{ NULL, 0 }
		};
		if( ! RegisterEntries( imp ) ){
//			DWORD err = GetLastError();
			::MYMESSAGEBOX( NULL, MB_OK, _T("DLL�v���O�C��"), _T("DLL�̓ǂݍ��݂Ɏ��s���܂���") );
			return false;
		}
	}

	//DLL�֐��̌Ăяo��
	plug.m_handler();
	
	return true;
}
