/*!	@file
	@brief PPA.DLLマクロ

	@author YAZAKI
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPPAMACROMGR_H_
#define _CPPAMACROMGR_H_

#include <windows.h>
#include "CKeyMacroMgr.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! キーボードマクロ
class CPPAMacroMgr : public CKeyMacroMgr
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
	virtual void ExecKeyMacro( class CEditView* pcEditView ) const;				/* キーボードマクロの実行 */
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const char* pszPath);	/* キーボードマクロを読み込み、CMacroの列に変換 */

	static class CPPA m_cPPA;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPPAMACROMGR_H_ */


/*[EOF]*/
	