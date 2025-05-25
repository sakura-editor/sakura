/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "view/colors/EColorIndexType.h"
#include "config/app_constants.h"

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, L"基本" );
	pType->m_szTypeExts[0] = L'\0';

	//設定
	pType->m_nMaxLineKetas = CKetaXInt(MAXLINEKETAS);			// 折り返し桁数
	pType->m_eDefaultOutline = OUTLINE_TEXT;					// アウトライン解析方法
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// シングルクォーテーション文字列を色分け表示しない	//Oct. 17, 2000 JEPRO
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない	//Sept. 4, 2000 JEPRO
}
