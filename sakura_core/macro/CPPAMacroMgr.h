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

#include <Windows.h>
#include "CKeyMacroMgr.h"
#include "CPPA.h"

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
	virtual bool ExecKeyMacro( class CEditView* pcEditView, int flags ) const;	/* PPAマクロの実行 */
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath);		/* キーボードマクロをファイルから読み込み、CMacroの列に変換 */
	virtual BOOL LoadKeyMacroStr( HINSTANCE hInstance, const TCHAR* pszCode);	/* キーボードマクロを文字列から読み込み、CMacroの列に変換 */

	static class CPPA m_cPPA;

	// Apr. 29, 2002 genta
	static CMacroManagerBase* Creator(const TCHAR* ext);
	static void declare(void);

protected:
	CNativeW m_cBuffer;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPPAMACROMGR_H_ */



	
