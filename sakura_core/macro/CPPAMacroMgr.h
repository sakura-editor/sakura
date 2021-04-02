﻿/*!	@file
	@brief PPA.DLLマクロ

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPPAMACROMGR_C2A77DE3_E368_4D65_AC46_7E743718D84A_H_
#define SAKURA_CPPAMACROMGR_C2A77DE3_E368_4D65_AC46_7E743718D84A_H_
#pragma once

#include <Windows.h>
#include "CKeyMacroMgr.h"
#include "CPPA.h"
#include "mem/CNativeW.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! PPAマクロ
class CPPAMacroMgr final : public CMacroManagerBase
{
public:
	/*
	||  Constructors
	*/
	CPPAMacroMgr();
	~CPPAMacroMgr();

	/*
	||	PPA.DLLに委譲する部分
	*/
	bool ExecKeyMacro( class CEditView* pcEditView, int flags ) const override;	/* PPAマクロの実行 */
	BOOL LoadKeyMacro( HINSTANCE hInstance, const WCHAR* pszPath) override;		/* キーボードマクロをファイルから読み込み、CMacroの列に変換 */
	BOOL LoadKeyMacroStr( HINSTANCE hInstance, const WCHAR* pszCode) override;	/* キーボードマクロを文字列から読み込み、CMacroの列に変換 */

	static class CPPA m_cPPA;

	// Apr. 29, 2002 genta
	static CMacroManagerBase* Creator(const WCHAR* ext);
	static void declare(void);

protected:
	CNativeW m_cBuffer;
};
#endif /* SAKURA_CPPAMACROMGR_C2A77DE3_E368_4D65_AC46_7E743718D84A_H_ */
