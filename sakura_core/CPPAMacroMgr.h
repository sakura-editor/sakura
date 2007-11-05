/*!	@file
	@brief PPA.DLLマクロ

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPPAMACROMGR_H_
#define _CPPAMACROMGR_H_

#include <windows.h>
#include "CKeyMacroMgr.h"
#include "CMemory.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! PPAマクロ
class CPPAMacroMgr: public CMacroManagerBase
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
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath);	/* キーボードマクロを読み込み、CMacroの列に変換 */

	static class CPPA m_cPPA;

	// Apr. 29, 2002 genta
	static CMacroManagerBase* Creator(const TCHAR* ext);
	static void declare(void);

protected:
	CNativeW2 m_cBuffer;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPPAMACROMGR_H_ */


/*[EOF]*/
	
