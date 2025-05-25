/*!	@file
	@brief ジャック管理クラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CJackManager.h"
#include "CPropertyManager.h"
#include "typeprop/CPropTypes.h"

//コンストラクタ
CJackManager::CJackManager()
{
	int i;

	//ジャック定義一覧
	//添え字がEJackの値と同じであること。
	struct tagJackEntry {
		EJack id;
		const WCHAR* name;
	} jackNames[] = {
		{ PP_COMMAND				, PP_COMMAND_STR		},
//		{ PP_INSTALL				, L"Install"			},
//		{ PP_UNINSTALL				, L"Uninstall"			},
//		{ PP_APP_START				, L"AppStart"			},
//		{ PP_APP_END				, L"AppEnd"				},
		{ PP_EDITOR_START			, L"EditorStart"		},
		{ PP_EDITOR_END				, L"EditorEnd"			},
		{ PP_DOCUMENT_OPEN			, L"DocumentOpen"		},
		{ PP_DOCUMENT_CLOSE			, L"DocumentClose"		},
		{ PP_DOCUMENT_BEFORE_SAVE	, L"DocumentBeforeSave"	},
		{ PP_DOCUMENT_AFTER_SAVE	, L"DocumentAfterSave"	},
		{ PP_OUTLINE				, L"Outline"			},
		{ PP_SMARTINDENT			, L"SmartIndent"		},
		{ PP_COMPLEMENT				, L"Complement"			},
		{ PP_COMPLEMENTGLOBAL		, L"ComplementGlobal"	},
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

//ジャック定義一覧を返す
std::vector<JackDef> CJackManager::GetJackDef() const
{
	return m_Jacks;
}

//プラグをジャックに関連付ける
ERegisterPlugResult CJackManager::RegisterPlug( wstring pszJack, CPlug* plug )
{
	EJack ppId = GetJackFromName( pszJack );
	if( ppId == PP_NONE ){
		return PPMGR_INVALID_NAME;
	}

	//機能IDの昇順になるようにプラグを登録する
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
	case PP_OUTLINE:					//アウトライン解析方法を追加
		{
			int nMethod = CPlug::GetOutlineType( plug->GetFunctionCode() );	// 2011/8/20 syat プラグ複数化のためGetOutlineType仕様変更// 2010/5/1 Uchi 関数化
			CPropTypesScreen::AddOutlineMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	case PP_SMARTINDENT:				//スマートインデント方法を追加
		{
			int nMethod = CPlug::GetSmartIndentType( plug->GetFunctionCode() );	// 2011/8/20 syat プラグ複数化のためGetOutlineType仕様変更// 2010/5/1 Uchi 関数化
			CPropTypesScreen::AddSIndentMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	case PP_COMPLEMENT:
		{
			int nMethod = CPlug::GetPluginFunctionCode( plug->m_cPlugin.m_id, 0 );
			CPropTypesSupport::AddHokanMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	default:
		break;
	}
	return PPMGR_REG_OK;
}

//プラグの関連付けを解除する
bool CJackManager::UnRegisterPlug( wstring pszJack, CPlug* plug )
{
	EJack ppId = GetJackFromName( pszJack );

	switch( ppId ){
	case PP_OUTLINE:					//アウトライン解析方法を追加
		{
			int nMethod = CPlug::GetOutlineType( plug->GetFunctionCode() );
			CPropTypesScreen::RemoveOutlineMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	case PP_SMARTINDENT:				//スマートインデント方法を追加
		{
			int nMethod = CPlug::GetSmartIndentType( plug->GetFunctionCode() );
			CPropTypesScreen::RemoveSIndentMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	case PP_COMPLEMENT:
		{
			int nMethod = CPlug::GetPluginFunctionCode( plug->m_cPlugin.m_id, 0 );
			CPropTypesSupport::RemoveHokanMethod( nMethod, plug->m_sLabel.c_str() );
		}
		break;
	default:
		break;
	}

	for( unsigned int index=0; index<m_Jacks[ ppId ].plugs.size(); index++ ){
		if( m_Jacks[ ppId ].plugs[index] == plug ){
			m_Jacks[ ppId ].plugs.erase( m_Jacks[ ppId ].plugs.begin() + index );
			break;
		}
	}

	return true;
}

//ジャック名をジャック番号に変換する
EJack CJackManager::GetJackFromName( wstring sName )
{
	unsigned int i;
	const WCHAR* szName = sName.c_str();

	for( i=0; i < m_Jacks.size(); i++ ){
		if( wcscmp( m_Jacks[i].szName, szName ) == 0 ){
			return m_Jacks[i].ppId;
		}
	}

	//見つからない
	return PP_NONE;
}

//利用可能なプラグを検索する
bool CJackManager::GetUsablePlug(
	EJack			jack,		//!< [in] ジャック番号
	PlugId			plugId,		//!< [in] プラグID
	CPlug::Array*	plugs		//!< [out] 利用可能プラグのリスト
)
{
	for( auto it = m_Jacks[jack].plugs.begin(); it != m_Jacks[jack].plugs.end(); it++ ){
		if( plugId == 0 || plugId == (*it)->GetFunctionCode() ){
			plugs->push_back( *it );
		}
	}
	return true;
}

//プラグインを列挙して呼び出し
void CJackManager::InvokePlugins(EJack jack, CEditView* view)
{
	CPlug::Array plugs;
	CWSHIfObj::List params;
	GetUsablePlug( jack, 0, &plugs );
	for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
		(*it)->Invoke(view, params);
	}
}

//プラグインコマンドの機能番号を返す
EFunctionCode CJackManager::GetCommandCode( int index ) const
{
	CPlug::Array commands = m_Jacks[ PP_COMMAND ].plugs;

	if( (unsigned int)index < commands.size() ){
		return ( commands[index] )->GetFunctionCode();
	}else{
		return F_INVALID;
	}
}

//プラグインコマンドの名前を返す
int CJackManager::GetCommandName( int funccode, WCHAR* buf, int size ) const
{
	for( CPlug::ArrayIter it = m_Jacks[ PP_COMMAND ].plugs.cbegin(); it != m_Jacks[ PP_COMMAND ].plugs.cend(); it++ ){
		if( ((CPlug*)(*it))->GetFunctionCode() == funccode ){
			wcsncpy( buf, ((CPlug*)(*it))->m_sLabel.c_str(), size );
			buf[ size-1 ] = L'\0';
			return 1;
		}
	}
	return -1;
}

//プラグインコマンドの数を返す
int CJackManager::GetCommandCount() const
{
	return m_Jacks[ PP_COMMAND ].plugs.size();
}

//IDに合致するコマンドプラグを返す
CPlug* CJackManager::GetCommandById( int id ) const
{
	const CPlug::Array& plugs = GetPlugs( PP_COMMAND );
	for( CPlug::ArrayIter it = plugs.cbegin(); it != plugs.cend(); it++ ){
		if( (*it)->GetFunctionCode() == id ){
			return (*it);
		}
	}
	assert_warning(false);	//IDに合致するプラグが登録されていない
	return NULL;
}

//プラグを返す
const CPlug::Array& CJackManager::GetPlugs( EJack jack ) const
{
	return m_Jacks[ jack ].plugs;	
}
