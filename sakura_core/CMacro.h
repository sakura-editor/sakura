//	$Id$
/*!	@file
	@brief キーボードマクロ
	CMacroのインスタンスひとつが、1コマンドになる。

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CMacro;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include "CEditView.h"

/*! @brief キーボードマクロの1コマンド

	引数をリスト構造にして、いくつでも持てるようにしてみました。
	スタックにするのが通例なのかもしれません（よくわかりません）。
	
	今後、制御構造が入っても困らないようにしようと思ったのですが、挫折しました。
	
	さて、このクラスは次のような前提で動作している。

	@li 引数のリストを、m_pParamTopからのリスト構造で保持。
	@li 引数を新たに追加するには、AddParam()を使用する。
	  AddParamにどんな値が渡されてもよいように準備するコト。
	  渡された値が数値なのか、文字列へのポインタなのかは、m_nFuncID（機能ID）によって、このクラス内で判別し、よろしくやること。
	@li 引数は、CMacro内部ではすべて文字列で保持すること（数値97は、"97"として保持）（いまのところ）
*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro( int nFuncID );	//	機能IDを指定して初期化
	~CMacro();

	void SetNext(CMacro* pNext){ m_pNext = pNext; };
	CMacro* GetNext(){ return m_pNext;};
	void Exec( CEditView* pcEditView );
	void Save( HINSTANCE hInstance, HFILE hFile );
	
	void AddLParam( LPARAM lParam );
	void AddParam( const char* lParam );
	void AddParam( const int lParam );

	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* 機能ID→関数名，機能名日本語 */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* 関数名→機能ID，機能名日本語 */
	static BOOL CanFuncIsKeyMacro( int );	/* キーマクロに記録可能な機能かどうかを調べる */


protected:
	/*
	||  実装ヘルパ関数
	*/
	int		m_nFuncID;	//	機能ID
	struct CMacroParam{
		char* m_pData;
		CMacroParam* m_pNext;
	} *m_pParamTop, *m_pParamBot;			//	パラメータ
	CMacro* m_pNext;	//	次のマクロへのポインタ
//	CMacro* m_pPrev;	前のマクロに戻ることは無い？
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */

/*[EOF]*/

