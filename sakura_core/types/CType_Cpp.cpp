#include "stdafx.h"
#include "CType.h"

/* C/C++ */
// Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読めるようにする
// Jan. 24, 2004 genta 関連づけ上好ましくないのでdsw,dsp,dep,makははずす
//	2003.06.23 Moca ファイル内からの入力補完機能
void CType_Cpp::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("C/C++") );
	_tcscpy( pType->m_szTypeExts, _T("c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );							/* 行コメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );			/* ブロックコメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 1, L"#if 0", L"#endif" );	/* ブロックコメントデリミタ2 */	//Jul. 11, 2001 JEPRO
	pType->m_nKeyWordSetIdx[0] = 0;											/* キーワードセット */
	pType->m_nDefaultOutline = OUTLINE_CPP;									/* アウトライン解析方法 */
	pType->m_nSmartIndent = SMARTINDENT_CPP;								/* スマートインデント種別 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;					//半角数値を色分け表示	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;			//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pType->m_bUseHokanByFile = TRUE;										/*! 入力補完 開いているファイル内から候補を探す */
}
