/*!	@file
	@brief �W���b�N�Ǘ��N���X

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
#include "CJackManager.h"
#include "CPropertyManager.h"
#include "typeprop/CPropTypes.h"

//�R���X�g���N�^
CJackManager::CJackManager()
{
	int i;

	//�W���b�N��`�ꗗ
	//�Y������EJack�̒l�Ɠ����ł��邱�ƁB
	struct tagJackEntry {
		EJack id;
		const WCHAR* name;
	} jackNames[] = {
		{ PP_COMMAND				, PP_COMMAND_STR		},
		{ PP_INSTALL				, L"Install"			},
		{ PP_UNINSTALL				, L"Uninstall"			},
		{ PP_APP_START				, L"AppStart"			},
		{ PP_APP_END				, L"AppEnd"				},
		{ PP_EDITOR_START			, L"EditorStart"		},
		{ PP_EDITOR_END				, L"EditorEnd"			},
		{ PP_OUTLINE				, L"Outline"			},
		{ PP_SMARTINDENT			, L"SmartIndent"		},
	};

	m_pShareData = &GetDllShareData();

	m_Jacks.reserve( PP_BUILTIN_JACK_COUNT );
	for( i=0; i<PP_BUILTIN_JACK_COUNT; i++ ){
		assert( i == jackNames[i].id );

		JackDef jack;
		jack.ppId = jackNames[i].id;
		jack.szName = jackNames[i].name;

		m_Jacks.push_back( jack );
	}
	
}

//�W���b�N��`�ꗗ��Ԃ�
std::vector<JackDef> CJackManager::GetJackDef() const
{
	return m_Jacks;
}

//�v���O���W���b�N�Ɋ֘A�t����
ERegisterPlugResult CJackManager::RegisterPlug( wstring pszJack, CPlug* plug )
{
	EJack ppId = GetJackFromName( pszJack );
	if( ppId == PP_NONE ){
		return PPMGR_INVALID_NAME;
	}

	//�@�\ID�̏����ɂȂ�悤�Ƀv���O��o�^����
	CPlug::Array& plugs = m_Jacks[ ppId ].plugs;
	int plugid = plug->GetFunctionCode();
	if( plugs.empty()  ||  (*(plugs.end() - 1))->GetFunctionCode() < plugid ){
		plugs.push_back( plug );
	} else {
		for( unsigned int index=0; index<plugs.size(); index++ ){
			if( plugid < plugs[index]->GetFunctionCode() ){
				plugs.insert( plugs.begin() + index, plug );
				break;
			}
		}
	}

	switch( ppId ){
	case PP_OUTLINE:					//�A�E�g���C����͕��@��ǉ�
		{
//			int nMethod = (EOutlineType)( plug->m_cPlugin.m_id * 100 + F_PLUGCOMMAND_FIRST );
			int nMethod = (EOutlineType)GetPluginFunctionCode( plug->m_cPlugin.m_id, 0 );	// 2010/5/1 Uchi �֐���
			CPropScreen::AddOutlineMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	case PP_SMARTINDENT:				//�X�}�[�g�C���f���g���@��ǉ�
		{
//			int nMethod = (ESmartIndentType)( plug->m_cPlugin.m_id * 100 + F_PLUGCOMMAND_FIRST );
			int nMethod = (ESmartIndentType)GetPluginFunctionCode( plug->m_cPlugin.m_id, 0 );	// 2010/5/1 Uchi �֐���
			CPropScreen::AddSIndentMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	}
	return PPMGR_REG_OK;
}

//�v���O�̊֘A�t������������
bool CJackManager::UnRegisterPlug( wstring pszJack, CPlug* plug )
{
	EJack ppId = GetJackFromName( pszJack );

	for( unsigned int index=0; index<m_Jacks[ ppId ].plugs.size(); index++ ){
		if( m_Jacks[ ppId ].plugs[index] == plug ){
			m_Jacks[ ppId ].plugs.erase( m_Jacks[ ppId ].plugs.begin() + index );
		}
	}

	return true;
}

//�W���b�N�����W���b�N�ԍ��ɕϊ�����
EJack CJackManager::GetJackFromName( wstring sName )
{
	unsigned int i;
	const WCHAR* szName = sName.c_str();

	for( i=0; i < m_Jacks.size(); i++ ){
		if( wcscmp( m_Jacks[i].szName, szName ) == 0 ){
			return m_Jacks[i].ppId;
		}
	}

	//������Ȃ�
	return PP_NONE;
}

//���p�\�ȃv���O����������
bool CJackManager::GetUsablePlug(
	EJack			jack,		//!< [in] �W���b�N�ԍ�
	PlugId			plugId,		//!< [in] �v���OID
	CPlug::Array*	plugs		//!< [out] ���p�\�v���O�̃��X�g
)
{
	for( CPlug::Array::iterator it = m_Jacks[jack].plugs.begin(); it != m_Jacks[jack].plugs.end(); it++ ){
		if( plugId == 0 || plugId == (*it)->GetFunctionCode() ){
			plugs->push_back( *it );
		}
	}
	return true;
}

//�v���O�C���R�}���h�̋@�\�ԍ���Ԃ�
EFunctionCode CJackManager::GetCommandCode( int index ) const
{
	CPlug::Array commands = m_Jacks[ PP_COMMAND ].plugs;

	if( (unsigned int)index < commands.size() ){
		return ( commands[index] )->GetFunctionCode();
	}else{
		return F_INVALID;
	}
}

//�v���O�C���R�}���h�̖��O��Ԃ�
int CJackManager::GetCommandName( int funccode, WCHAR* buf, int size ) const
{
	for( CPlug::ArrayIter it = m_Jacks[ PP_COMMAND ].plugs.begin(); it != m_Jacks[ PP_COMMAND ].plugs.end(); it++ ){
		if( ((CPlug*)(*it))->GetFunctionCode() == funccode ){
			wcsncpy( buf, ((CPlug*)(*it))->m_sLabel.c_str(), size );
			buf[ size-1 ] = L'\0';
			return 1;
		}
	}
	return -1;
}

//�v���O�C���R�}���h�̐���Ԃ�
int CJackManager::GetCommandCount() const
{
	return m_Jacks[ PP_COMMAND ].plugs.size();
}

//ID�ɍ��v����R�}���h�v���O��Ԃ�
CPlug* CJackManager::GetCommandById( int id ) const
{
	const CPlug::Array& plugs = GetPlugs( PP_COMMAND );
	for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
		if( (*it)->GetFunctionCode() == id ){
			return (*it);
		}
	}
	assert_warning(false);	//ID�ɍ��v����v���O���o�^����Ă��Ȃ�
	return NULL;
}

//�v���O��Ԃ�
const CPlug::Array& CJackManager::GetPlugs( EJack jack ) const
{
	return m_Jacks[ jack ].plugs;	
}
