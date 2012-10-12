#include "StdAfx.h"
#include "CType.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/Colors/CColorStrategy.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

void _DefaultConfig(STypeConfig* pType);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CTypeConfig                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
STypeConfig* CTypeConfig::GetTypeConfig()
{
	return &CDocTypeManager().GetTypeSetting(*this);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();

	//規定値をコピー
	static STypeConfig sDefault;
	static bool bLoadedDefault = false;
	if(!bLoadedDefault){
		_DefaultConfig(&sDefault);
		bLoadedDefault=true;
	}
	CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx)) = sDefault;

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
	};
	assert( 1 <= MAX_TYPES );
	for(int i=0;i<_countof(table) && i < MAX_TYPES;i++){
		table[i]->InitTypeConfig(i);
		SAFE_DELETE(table[i]);
	}

	// 設定17-30(MAX_TYPES)
	if( _countof(table) < MAX_TYPES ){
		CType_Other other;
		for( int i = _countof(table); i < MAX_TYPES; i++ ){
			other.InitTypeConfig(i);
		}
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
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname );
	
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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        デフォルト                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DefaultConfig(STypeConfig* pType)
{
//キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/

	pType->m_nTextWrapMethod = WRAP_SETTING_WIDTH;	// テキストの折り返し方法		// 2008.05.30 nasukoji
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);	/* 折り返し桁数 */
	pType->m_nColmSpace = 0;					/* 文字と文字の隙間 */
	pType->m_nLineSpace = 1;					/* 行間のすきま */
	pType->m_nTabSpace = CLayoutInt(4);					/* TABの文字数 */
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		pType->m_nKeyWordSetIdx[i] = -1;
	}
	wcscpy( pType->m_szTabViewString, _EDITL("^       ") );	/* TAB表示文字列 */
	pType->m_bTabArrow = false;				/* タブ矢印表示 */	// 2001.12.03 hor
	pType->m_bInsSpace = FALSE;				/* スペースの挿入 */	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI 以下、m_cLineCommentとm_cBlockCommentsを使うように修正
	pType->m_cLineComment.CopyTo(0, L"", -1);	/* 行コメントデリミタ */
	pType->m_cLineComment.CopyTo(1, L"", -1);	/* 行コメントデリミタ2 */
	pType->m_cLineComment.CopyTo(2, L"", -1);	/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	pType->m_cBlockComments[0].SetBlockCommentRule(L"", L"");	/* ブロックコメントデリミタ */
	pType->m_cBlockComments[1].SetBlockCommentRule(L"", L"");	/* ブロックコメントデリミタ2 */

	pType->m_nStringType = 0;					/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"" );		/* その他のインデント対象文字 */

	pType->m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pType->m_szHokanFile, _T("") );		/* 入力補完 単語ファイル */
	// 2001/06/14 End

	pType->m_nHokanType = 0;

	// 2001/06/19 asa-o
	pType->m_bHokanLoHiCase = FALSE;			/* 入力補完機能：英大文字小文字を同一視する */

	//	2003.06.23 Moca ファイル内からの入力補完機能
	pType->m_bUseHokanByFile = FALSE;			/*! 入力補完 開いているファイル内から候補を探す */

	// 文字コード設定
	pType->m_encoding.m_bPriorCesu8 = FALSE;
	pType->m_encoding.m_eDefaultCodetype = CODE_SJIS;
	pType->m_encoding.m_eDefaultEoltype = EOL_CRLF;
	pType->m_encoding.m_bDefaultBom = FALSE;

	//@@@2002.2.4 YAZAKI
	pType->m_szExtHelp[0] = L'\0';
	pType->m_szExtHtmlHelp[0] = L'\0';
	pType->m_bHtmlHelpIsSingle = TRUE;

	pType->m_bAutoIndent = true;			/* オートインデント */
	pType->m_bAutoIndent_ZENSPACE = true;	/* 日本語空白もインデント */
	pType->m_bRTrimPrevLine = FALSE;			/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */

	pType->m_nIndentLayout = 0;	/* 折り返しは2行目以降を字下げ表示 */


	for( int i = 0; i < COLORIDX_LAST; ++i ){
		GetDefaultColorInfo(&pType->m_ColorInfoArr[i],i);
	}
	pType->m_szBackImgPath[0] = '\0';
	pType->m_backImgPos = BGIMAGE_TOP_LEFT;
	pType->m_backImgRepeatX = true;
	pType->m_backImgRepeatY = true;
	pType->m_backImgScrollX = true;
	pType->m_backImgScrollY = true;
	{
		POINT pt ={0,0};
		pType->m_backImgPosOffset = pt;
	}
	pType->m_bLineNumIsCRLF = true;				/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	pType->m_nLineTermType = 1;					/* 行番号区切り 0=なし 1=縦線 2=任意 */
	pType->m_cLineTermChar = L':';					/* 行番号区切り文字 */
	pType->m_bWordWrap = FALSE;					/* 英文ワードラップをする */
	pType->m_nCurrentPrintSetting = 0;				/* 現在選択している印刷設定 */
	pType->m_bOutlineDockDisp = FALSE;				/* アウトライン解析表示の有無 */
	pType->m_eOutlineDockSide = DOCKSIDE_FLOAT;		/* アウトライン解析ドッキング配置 */
	pType->m_cxOutlineDockLeft = 0;					// アウトラインの左ドッキング幅
	pType->m_cyOutlineDockTop = 0;					// アウトラインの上ドッキング高
	pType->m_cxOutlineDockRight = 0;				// アウトラインの右ドッキング幅
	pType->m_cyOutlineDockBottom = 0;				// アウトラインの下ドッキング高
	pType->m_eDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pType->m_nOutlineSortCol = 0;					/* アウトライン解析ソート列番号 */
	pType->m_nOutlineSortType = 0;					/* アウトライン解析ソート基準 */
	pType->m_eSmartIndent = SMARTINDENT_NONE;		/* スマートインデント種別 */
	pType->m_nImeState = IME_CMODE_NOCONVERSION;	/* IME入力 */

	pType->m_szOutlineRuleFilename[0] = L'\0';	//Dec. 4, 2000 MIK
	pType->m_bKinsokuHead = FALSE;				/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;				/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;				/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;				/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"" );		/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuTail, L"" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuKuto, L"、。，．､｡,." );	/* 句読点ぶら下げ文字 */	// 2009.08.07 ryoji

	pType->m_bUseDocumentIcon = FALSE;			/* 文書に関連づけられたアイコンを使う */

//@@@ 2001.11.17 add start MIK
	for(int i = 0; i < _countof(pType->m_RegexKeywordArr); i++)
	{
		pType->m_RegexKeywordArr[i].m_szKeyword[0] = L'\0';
		pType->m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pType->m_bUseRegexKeyword = false;
//		pType->m_nRegexKeyMagicNumber = 1;
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	for(int i = 0; i < MAX_KEYHELP_FILE; i++){
		pType->m_KeyHelpArr[i].m_bUse = false;
		pType->m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		pType->m_KeyHelpArr[i].m_szPath[0] = _T('\0');
	}
	pType->m_bUseKeyWordHelp = FALSE;	/* 辞書選択機能の使用可否 */
	pType->m_nKeyHelpNum = 0;			/* 登録辞書数 */
	pType->m_bUseKeyHelpAllSearch = FALSE;	/* ヒットした次の辞書も検索(&A) */
	pType->m_bUseKeyHelpKeyDisp = FALSE;		/* 1行目にキーワードも表示する(&W) */
	pType->m_bUseKeyHelpPrefix = FALSE;		/* 選択範囲で前方一致検索(&P) */
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca 指定位置縦線の設定
	for(int i = 0; i < MAX_VERTLINES; i++ ){
		pType->m_nVertLineIdx[i] = CLayoutInt(0);
	}
}
