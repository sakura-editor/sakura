#include "stdafx.h"
#include "types/CType.h"

void _DefaultConfig(STypeConfig* pType)
{
//キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/

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
	
	//@@@ 2002.09.22 YAZAKI 以下、m_cLineCommentとm_cBlockCommentを使うように修正
	pType->m_cLineComment.CopyTo(0, L"", -1);	/* 行コメントデリミタ */
	pType->m_cLineComment.CopyTo(1, L"", -1);	/* 行コメントデリミタ2 */
	pType->m_cLineComment.CopyTo(2, L"", -1);	/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	pType->m_cBlockComment.SetBlockCommentRule(0, L"", L"");	/* ブロックコメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule(1, L"", L"");	/* ブロックコメントデリミタ2 */

	pType->m_nStringType = 0;					/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"" );		/* その他のインデント対象文字 */

	pType->m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pType->m_szHokanFile, _T("") );		/* 入力補完 単語ファイル */
	// 2001/06/14 End

	// 2001/06/19 asa-o
	pType->m_bHokanLoHiCase = FALSE;			/* 入力補完機能：英大文字小文字を同一視する */

	//	2003.06.23 Moca ファイル内からの入力補完機能
	pType->m_bUseHokanByFile = FALSE;			/*! 入力補完 開いているファイル内から候補を探す */

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
	pType->m_bLineNumIsCRLF = TRUE;				/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	pType->m_nLineTermType = 1;					/* 行番号区切り 0=なし 1=縦線 2=任意 */
	pType->m_cLineTermChar = L':';					/* 行番号区切り文字 */
	pType->m_bWordWrap = FALSE;					/* 英文ワードラップをする */
	pType->m_nCurrentPrintSetting = 0;				/* 現在選択している印刷設定 */
	pType->m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pType->m_nSmartIndent = SMARTINDENT_NONE;		/* スマートインデント種別 */
	pType->m_nImeState = IME_CMODE_NOCONVERSION;	/* IME入力 */

	pType->m_szOutlineRuleFilename[0] = L'\0';	//Dec. 4, 2000 MIK
	pType->m_bKinsokuHead = FALSE;				/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;				/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;				/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;				/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"" );		/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuTail, L"" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK

	pType->m_bUseDocumentIcon = FALSE;			/* 文書に関連づけられたアイコンを使う */

//@@@ 2001.11.17 add start MIK
	for(int i = 0; i < 100; i++)
	{
		pType->m_RegexKeywordArr[i].m_szKeyword[0] = L'\0';
		pType->m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pType->m_bUseRegexKeyword = FALSE;
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

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//規定値の構築
	_DefaultConfig(pType);

	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("基本") );
	_tcscpy( pType->m_szTypeExts, _T("") );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);			// 折り返し桁数
	pType->m_nDefaultOutline = OUTLINE_TEXT;					// アウトライン解析方法
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// シングルクォーテーション文字列を色分け表示しない	//Oct. 17, 2000 JEPRO
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない	//Sept. 4, 2000 JEPRO
}
