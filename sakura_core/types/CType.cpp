#include "stdafx.h"
#include "CType.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CTypeConfig                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
STypeConfig* CTypeConfig::GetTypeConfig()
{
	return &GetDllShareData().GetTypeSetting(*this);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();

	//規定値をコピー
	pShareData->GetTypeSetting(CTypeConfig(nIdx)) = pShareData->GetTypeSetting(CTypeConfig(0));

	//インデックスを設定
	CTypeConfig(nIdx)->m_nIdx = nIdx;

	//個別設定
	InitTypeConfigImp(CTypeConfig(nIdx).GetTypeConfig());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 共有メモリ初期化/タイプ別設定

	タイプ別設定の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
void CShareData::InitTypeConfigs(DLLSHAREDATA* pShareData)
{
	CType* table[] = {
		new CType_Basis(),	//基本
		new CType_Text(),	//テキスト
		new CType_Cpp(),	//C/C++
		new CType_Html(),	//HTML
		new CType_Sql(),	//PL/SQL
		new CType_Cobol(),	//COBOL
		new CType_Java(),	//Java
		new CType_Asm(),	//アセンブラ
		new CType_Awk(),	//awk
		new CType_Dos(),	//MS-DOSバッチファイル
		new CType_Pascal(),	//Pascal
		new CType_Tex(),	//TeX
		new CType_Perl(),	//Perl
		new CType_Vb(),		//Visual Basic
		new CType_Rich(),	//リッチテキスト
		new CType_Ini(),	//設定ファイル
		new CType_Other1(),	//設定17
		new CType_Other2(),	//設定18
		new CType_Other3(),	//設定19
		new CType_Other4(),	//設定20
	};
	for(int i=0;i<_countof(table);i++){
		table[i]->InitTypeConfig(i);
		SAFE_DELETE(table[i]);
	}
}


/*!	@brief 共有メモリ初期化/強調キーワード

	強調キーワード関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
		キーワード定義を関数の外に出し，登録をマクロ化して簡潔に．
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* 強調キーワードのテストデータ */
	pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	pShareData->m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname );
	
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
	PopulateKeyword( L"リッチテキスト",	true,	RTF );			/* セット15の追加 */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

