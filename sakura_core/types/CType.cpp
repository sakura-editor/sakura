/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CType.h"
#include "view/colors/EColorIndexType.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "config/app_constants.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx, STypeConfig& type)
{
	//規定値をコピー
	static STypeConfig sDefault;
	static bool bLoadedDefault = false;
	if(!bLoadedDefault){
		bLoadedDefault=true;
	}
	type = sDefault;

	//インデックスを設定
	type.m_nIdx = nIdx;
	type.m_id = nIdx;

	//個別設定
	InitTypeConfigImp(&type);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 共有メモリ初期化/タイプ別設定

	タイプ別設定の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
void DLLSHAREDATA::InitTypeConfigs(std::vector<STypeConfig*>& types)
{
	const std::array<std::unique_ptr<CType>, 17> table = {{
		std::make_unique<CType_Basis>(),
		std::make_unique<CType_Text>(),
		std::make_unique<CType_Cpp>(),
		std::make_unique<CType_Html>(),
		std::make_unique<CType_Sql>(),
		std::make_unique<CType_Cobol>(),
		std::make_unique<CType_Java>(),
		std::make_unique<CType_Asm>(),
		std::make_unique<CType_Awk>(),
		std::make_unique<CType_Dos>(),
		std::make_unique<CType_Pascal>(),
		std::make_unique<CType_Tex>(),
		std::make_unique<CType_Perl>(),
		std::make_unique<CType_Python>(),
		std::make_unique<CType_Vb>(),
		std::make_unique<CType_Rich>(),
		std::make_unique<CType_Ini>()
	}};

	types.clear();

	static_assert(std::size(table) <= MAX_TYPES);

	for (int i = 0; i < int(std::size(table)); ++i) {
		STypeConfig* type = new STypeConfig;
		types.push_back(type);
		table[i]->InitTypeConfig(i, *type);
		wcscpy_s(m_TypeMini[i].m_szTypeName, type->m_szTypeName);
		wcscpy_s(m_TypeMini[i].m_szTypeExts, type->m_szTypeExts);
		m_TypeMini[i].m_encoding = type->m_encoding;
		m_TypeMini[i].m_id = type->m_id;
	}

	m_TypeBasis = *types[0];
	m_nTypesCount = int(std::size(table));
}

/*!	@brief 共有メモリ初期化/強調キーワード

	強調キーワード関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
		キーワード定義を関数の外に出し，登録をマクロ化して簡潔に．
*/
CommonSetting_SpecialKeyword::CommonSetting_SpecialKeyword() noexcept
{
	/* 強調キーワードのテストデータ */
	m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname );
	
	PopulateKeyword( L"C/C++",			true,	CPP );			/* セット 0の追加 */
	PopulateKeyword( L"HTML",			false,	HTML );			/* セット 1の追加 */
	PopulateKeyword( L"PL/SQL",			false,	PLSQL );		/* セット 2の追加 */
	PopulateKeyword( L"COBOL",			true,	COBOL );		/* セット 3の追加 */
	PopulateKeyword( L"Java",			true,	JAVA );			/* セット 4の追加 */
	PopulateKeyword( L"CORBA IDL",		true,	CORBA_IDL );	/* セット 5の追加 */
	PopulateKeyword( L"AWK",			true,	AWK );			/* セット 6の追加 */
	PopulateKeyword( L"MS-DOS batch",	false,	BAT );			/* セット 7の追加 */	//Oct. 31, 2000 JEPRO 'バッチファイル'→'batch' に短縮
	PopulateKeyword( L"Pascal",			false,	PASCAL );		/* セット 8の追加 */	//Nov. 5, 2000 JEPRO 大・小文字の区別を'しない'に変更
	PopulateKeyword( L"TeX",			true,	TEX );			/* セット 9の追加 */	//Sept. 2, 2000 jepro Tex →TeX に修正 Bool値は大・小文字の区別
	PopulateKeyword( L"TeX2",			true,	TEX2 );			/* セット10の追加 */	//Jan. 19, 2001 JEPRO 追加
	PopulateKeyword( L"Perl",			true,	PERL );			/* セット11の追加 */
	PopulateKeyword( L"Perl2",			true,	PERL2 );		/* セット12の追加 */	//Jul. 10, 2001 JEPRO Perlから変数を分離・独立
	PopulateKeyword( L"Visual Basic",	false,	VB );			/* セット13の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Visual Basic2",	false,	VB2 );			/* セット14の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Rich Text",		true,	RTF );			/* セット15の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Python",			true,	Python);		/* セット16の追加 */

#undef PopulateKeyword
}

STypeConfig::STypeConfig() noexcept
{
	/************************/
	/* タイプ別設定の規定値 */
	/************************/

	m_nMaxLineKetas = CKetaXInt(MAXLINEKETAS);

	std::fill(std::begin(m_nKeyWordSetIdx), std::end(m_nKeyWordSetIdx), -1);

	wcscpy_s(m_szTabViewString, L"^       ");

	static_assert( COLORIDX_LAST <= _countof(m_ColorInfoArr) );
	for (int i = 0; i < COLORIDX_LAST; ++i) {
		m_ColorInfoArr[i].SetDefault(i);
	}

	wcscpy_s(m_szKinsokuKuto, L"、。，．､｡,.");
}
