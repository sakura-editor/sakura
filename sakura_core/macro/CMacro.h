/*!	@file
	@brief キーボードマクロ

	CMacroのインスタンスひとつが、1コマンドになる。

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, 鬼

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CMacro;
class CTextOutputStream;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include <objidl.h>  // VARIANT等
#include "func/Funccode.h"

class CEditView;

/*! @brief キーボードマクロの1コマンド

	引数をリスト構造にして、いくつでも持てるようにしてみました。
	スタックにするのが通例なのかもしれません（よくわかりません）。
	
	今後、制御構造が入っても困らないようにしようと思ったのですが、挫折しました。
	
	さて、このクラスは次のような前提で動作している。

	@li 引数のリストを、m_pParamTopからのリスト構造で保持。
	@li 引数を新たに追加するには、AddParam()を使用する。
	  AddParamにどんな値が渡されてもよいように準備するコト。
	  渡された値が数値なのか、文字列へのポインタなのかは、m_nFuncID（機能 ID）によって、このクラス内で判別し、よろしくやること。
	@li 引数は、CMacro内部ではすべて文字列で保持すること（数値97は、"97"として保持）（いまのところ）
*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro( EFunctionCode nFuncID );	//	機能IDを指定して初期化
	~CMacro();

	void SetNext(CMacro* pNext){ m_pNext = pNext; }
	CMacro* GetNext(){ return m_pNext; }
	void Exec( CEditView* pcEditView ) const; //2007.09.30 kobake const追加
	void Save( HINSTANCE hInstance, CTextOutputStream& out ) const; //2007.09.30 kobake const追加
	
	void AddLParam( LPARAM lParam, const CEditView* pcEditView  );	//@@@ 2002.2.2 YAZAKI pcEditViewも渡す
	void AddStringParam( const WCHAR* lParam );
	void AddStringParam( const ACHAR* lParam ){ return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int lParam );

	static void HandleCommand( CEditView* pcEditView, const EFunctionCode Index,	const WCHAR* Argument[], const int ArgSize );
	static bool HandleFunction(CEditView *View, int ID, VARIANT *Arguments, int ArgSize, VARIANT &Result);
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* 機能ID→関数名，機能名日本語 */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* 関数名→機能ID，機能名日本語 */
	static BOOL CanFuncIsKeyMacro( int );	/* キーマクロに記録可能な機能かどうかを調べる */
#endif

protected:
	struct CMacroParam{
		WCHAR*			m_pData;
		CMacroParam*	m_pNext;
	};

	/*
	||  実装ヘルパ関数
	*/
	EFunctionCode	m_nFuncID;		//	機能ID
	CMacroParam*	m_pParamTop;	//	パラメータ
	CMacroParam*	m_pParamBot;
	CMacro*			m_pNext;		//	次のマクロへのポインタ
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */



