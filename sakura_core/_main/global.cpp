/*!	@file
	@brief 文字列共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "global.h"
#include "window/CEditWnd.h"
#include "CNormalProcess.h"

//2007.10.02 kobake CEditWndのインスタンスへのポインタをここに保存しておく
CEditWnd* g_pcEditWnd = NULL;


/*! 選択領域描画用パラメータ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;






HINSTANCE G_AppInstance()
{
	return CProcess::getInstance()->GetProcessInstance();
}
