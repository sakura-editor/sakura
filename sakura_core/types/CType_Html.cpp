#include "stdafx.h"
#include "types/CType.h"

/* HTML */
//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
//Feb. 2, 2005 genta 苦情が多いのでシングルクォートの色分けはHTMLでは行わない
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	_tcscpy( pType->m_szTypeName, _T("HTML") );
	_tcscpy( pType->m_szTypeExts, _T("html,htm,shtml,plg") );

	//設定
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"<!--", L"-->" );	/* ブロックコメントデリミタ */
	pType->m_nStringType = 0;											/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pType->m_nKeyWordSetIdx[0] = 1;										/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;			//シングルクォートの色分けOFF
}
