#include "stdafx.h"
#include "types/CType.h"

/* COBOL */
void CType_Cobol::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("COBOL") );
	_tcscpy( pType->m_szTypeExts, _T("cbl,cpy,pco,cob") );	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加

	//設定
	pType->m_cLineComment.CopyTo( 0, L"*", 6 );			//Jun. 02, 2001 JEPRO 修正
	pType->m_cLineComment.CopyTo( 1, L"D", 6 );			//Jun. 04, 2001 JEPRO 追加
	pType->m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"*" );				/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 3;						/* キーワードセット */		//Jul. 10, 2001 JEPRO
	pType->m_nDefaultOutline = OUTLINE_COBOL;			/* アウトライン解析方法 */
	// 指定桁縦線	//2005.11.08 Moca
	pType->m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = true;
	pType->m_nVertLineIdx[0] = CLayoutInt(7);
	pType->m_nVertLineIdx[1] = CLayoutInt(8);
	pType->m_nVertLineIdx[2] = CLayoutInt(12);
	pType->m_nVertLineIdx[3] = CLayoutInt(73);
}
