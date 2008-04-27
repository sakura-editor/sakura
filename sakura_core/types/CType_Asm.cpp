#include "stdafx.h"
#include "types/CType.h"

/* アセンブラ */
//	2004.05.01 MIK/genta
//Mar. 10, 2001 JEPRO	半角数値を色分け表示
void CType_Asm::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("アセンブラ") );
	_tcscpy( pType->m_szTypeExts, _T("asm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L";", -1 );			/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_ASM;					/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;
}
