//	$Id$
/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CKEYMACROMGR_H_
#define _CKEYMACROMGR_H_

#include <windows.h>

/*
struct KeyMacroData {
	int		m_nFuncID;
	LPARAM	m_lParam1;
};
*/

//#define MAX_STRLEN			70
//#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! キーボードマクロ
class CKeyMacroMgr
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
	void ClearAll( void );				/* キーマクロのバッファをクリアする */
	void Append( int , LPARAM, class CEditView* pcEditView );		/* キーマクロのバッファにデータ追加 */
	void Append( class CMacro* macro );		/* キーマクロのバッファにデータ追加 */
//	int GetMacroNum() { return m_nKeyMacroDataArrNum; };
	
	/* キーボードマクロをまとめて取り扱う */
	BOOL SaveKeyMacro( HINSTANCE hInstance, const char* pszPath) const;	/* CMacroの列を、キーボードマクロに保存 */
	//@@@2002.2.2 YAZAKI PPA.DLLアリ/ナシ共存のためvirtualに。
	virtual void ExecKeyMacro( class CEditView* pcEditView ) const;				/* キーボードマクロの実行 */
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const char* pszPath);		/* キーボードマクロを読み込み、CMacroの列に変換 */
	
	/* キーボードマクロが読み込み済みか確認する */
	BOOL IsReady(){ return m_nReady; }

protected:
//	int				m_nKeyMacroDataArrNum;
	BOOL			m_nReady;	//	Load済みかどうかを表すフラグ TRUE...Load済み、FALSE...未Load。

	class CMacro*	m_pTop;	//	先頭と終端を保持
	class CMacro*	m_pBot;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYMACROMGR_H_ */


/*[EOF]*/
