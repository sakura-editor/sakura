/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "CMacroManagerBase.h"
#include "Funccode_enum.h"

class CMacro;

//#define MAX_STRLEN			70
//#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! キーボードマクロ
/*!
	キーボードマクロクラス
*/
class CKeyMacroMgr : public CMacroManagerBase
{
  public:
    /*
	||  Constructors
	*/
    CKeyMacroMgr();
    ~CKeyMacroMgr();

    /*
	||  Attributes & Operations
	*/
    void ClearAll(void); /* キーマクロのバッファをクリアする */
    void Append(EFunctionCode nFuncID, const LPARAM *lParams, class CEditView *pcEditView); /* キーマクロのバッファにデータ追加 */
    void Append(class CMacro *macro); /* キーマクロのバッファにデータ追加 */

    /* キーボードマクロをまとめて取り扱う */
    BOOL SaveKeyMacro(HINSTANCE hInstance, const WCHAR *pszPath) const; /* CMacroの列を、キーボードマクロに保存 */
    //@@@2002.2.2 YAZAKI PPA.DLLアリ/ナシ共存のためvirtualに。
    //	2007.07.20 genta flags追加
    bool ExecKeyMacro(class CEditView *pcEditView, int flags) const override; /* キーボードマクロの実行 */
    BOOL LoadKeyMacro(HINSTANCE hInstance, const WCHAR *pszPath) override; /* キーボードマクロをファイルから読み込む */
    BOOL LoadKeyMacroStr(HINSTANCE hInstance, const WCHAR *pszCode) override; /* キーボードマクロを文字列から読み込む */

    // Apr. 29, 2002 genta
    static CMacroManagerBase *Creator(const WCHAR *ext);
    static void declare(void);

  protected:
    CMacro *m_pTop; //	先頭と終端を保持
    CMacro *m_pBot;
};
