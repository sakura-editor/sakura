//	$Id$
/*!	@file
	@brief DLLのロード、アンロード

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _LOAD_LIBRARY_H_
#define _LOAD_LIBRARY_H_

#include <windows.h>
#include "global.h"

//! DLLの動的なLoad/Unloadを行うためのクラス
/*!
	@author genta
	@date Jun. 10, 2001
	@date Apr. 15, 2002 genta RegisterEntriesの追加。
*/
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

		@date Jul. 5, 2001 genta 引数追加。パスの指定などに使える
	*/
	int LoadLibrary(char* str = NULL);
	//! DLLのアンロード
	/*!
		@param force [in] 終了処理に失敗してもDLLを解放するかどうか
	*/
	int FreeLibrary(bool force = false);

	//! 利用状態のチェック（operator版）
	bool operator!(void) const { return IsAvailable(); }

	//!	DLLのロード
	/*!
		詳細な戻り値を返さないこと以外はLoadLibrary()と同じ
	*/
	bool Init(char* str = NULL){ return LoadLibrary(str) == 0; }

	//! インスタンスハンドルの取得
	HINSTANCE GetInstance() const { return m_hInstance; }
protected:

	/*!
		アドレスとエントリ名の対応表。RegisterEntriesで使われる。
		@author YAZAKI
		@date 2002.01.26
	*/
	struct ImportTable 
	{
		void* proc;
		const char* name;
	};

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
		@par 注意
		デストラクタからFreeLibrary及びDeinitDllが呼び出されたときは
		ポリモーフィズムが行われないためにサブクラスのDeinitDllが呼び出されない。
		そのため、サブクラスのデストラクタではDeinitDllを明示的に呼び出す必要がある。
		
		FreeLibraryがデストラクタ以外から呼び出される場合はDeinitDllは仮想関数として
		サブクラスのものが呼び出され、デストラクタは当然呼び出されないので
		DeinitDllそのものは必要である。
		
		デストラクタからDeinitDllを呼ぶときは、初期化されているという保証がないので
		呼び出し前にIsAvailableによる確認を必ず行うこと。
		
		@date 2002.04.15 genta 注意書き追加
	*/
	virtual int DeinitDll(void);
	//! DLLファイル名の取得
	/*!
		@date Jul. 5, 2001 genta 引数追加。パスの指定などに使える
	*/
	virtual char* GetDllName(char*) = 0;
	
	bool RegisterEntries(const ImportTable table[]);

private:
	HINSTANCE m_hInstance;
};

#endif


/*[EOF]*/
