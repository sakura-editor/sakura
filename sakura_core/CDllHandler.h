//	$Id$
/*!	@file
	DLLのロード、アンロード

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _LOAD_LIBRARY_H_
#define _LOAD_LIBRARY_H_

#include <windows.h>
#include "global.h"

//! DLLの動的なLoad/Unloadを行うためのクラス
class SAKURA_CORE_API CDllHandler {
public:
	CDllHandler();
	virtual ~CDllHandler();
	
	//! 利用状態のチェック
	/*!
		DLLの関数を呼び出せるか状態どうか
	
		@retval true 利用可能
		@retval false 利用不能
	*/
	virtual bool IsAvailable(void) const { return m_hInstance != NULL; }
	//! DLLのロード
	/*!
		@retval 0 正常終了。DLLがロードされた。
		@retval other 異常終了。DLLはロードされなかった。
	*/
	int LoadLibrary(void);
	//! DLLのアンロード
	/*!
		@param force [in] 終了処理に失敗してもDLLを解放するかどうか
	*/
	int FreeLibrary(bool force = false);

	//! 利用状態のチェック（operator版）
	bool operator!(void) const { return IsAvailable(); }

	//! インスタンスハンドルの取得
	HINSTANCE GetInstance() const { return m_hInstance; }
protected:
	//!	DLLの初期化
	/*!
		DLLのロードに成功した直後に呼び出される．エントリポイントの
		確認などを行う．
		
		@retval 0 正常終了
		@retval other 異常終了．値の意味は自由に設定して良い．
		
		@note 0以外の値を返した場合は、読み込んだDLLを解放する．
	*/
	virtual int InitDll(void) = 0;
	//!	関数の初期化
	/*!
		DLLのアンロードを行う直前に呼び出される．メモリの解放などを
		行う．
		
		@retval 0 正常終了
		@retval other 異常終了．値の意味は自由に設定して良い．
		
		@note 0以外を返したときはDLLのUnloadは行われない．
	*/
	virtual int DeinitDll(void);
	//! DLLファイル名の取得
	virtual char* GetDllName(void) = 0;

private:
	HINSTANCE m_hInstance;
};

#endif
