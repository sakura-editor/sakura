/*!	@file
	@brief DLLのロード、アンロード

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, genta

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

#include <Windows.h>
#include <string>
#include "_main/global.h"

/*! CDllImp をラップ
	CDllImp::DeinitDll を呼び忘れないためのヘルパ的クラス。
	今のところDeinitDllが使われている箇所が無いので、このクラスの出番はありませんが。
	2008.05.10 kobake 作成
*/
template <class DLLIMP> class CDllHandler{
public:
	//コンストラクタ・デストラクタ
	CDllHandler()
	{
		m_pcDllImp = new DLLIMP();
		m_pcDllImp->InitDll();
	}
	~CDllHandler()
	{
		m_pcDllImp->DeinitDll(true); //※終了処理に失敗しても強制的にDLL解放
		delete m_pcDllImp;
	}

	//アクセサ
	DLLIMP* operator->(){ return m_pcDllImp; }

	//! 利用状態のチェック（operator版）
	bool operator!() const { return m_pcDllImp->IsAvailable(); }

private:
	DLLIMP*	m_pcDllImp;
};


//!結果定数
enum EDllResult{
	DLL_SUCCESS,		//成功
	DLL_LOADFAILURE,	//DLLロード失敗
	DLL_INITFAILURE,	//初期処理に失敗
};

//! DLLの動的なLoad/Unloadを行うためのクラス
/*!
	@author genta
	@date Jun. 10, 2001 genta
	@date 2001.07.05 genta InitDll: 引数追加。パスの指定などに使える
	@date Apr. 15, 2002 genta RegisterEntriesの追加。
	@date 2007.06.25 genta InitDll: GetDllNameImpを使うように実装を変更．
	@date 2001.07.05 genta GetDllName: 引数追加。パスの指定などに使える
	@date 2007.06.25 genta GetDllName: GetDllNameImpを使用する場合は必須ではないので，
										純粋仮想関数はやめてプレースホルダーを用意する．
	@date 2008.05.10 kobake 整理。派生クラスは、～Impをオーバーロードすれば良いという方式です。
*/
class CDllImp{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                            型                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
		アドレスとエントリ名の対応表。RegisterEntriesで使われる。
		@author YAZAKI
		@date 2002.01.26
	*/
	struct ImportTable{
		void*		proc;
		const char*	name;
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        生成と破棄                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//コンストラクタ・デストラクタ
	CDllImp();
	virtual ~CDllImp();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         DLLロード                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//! DLLの関数を呼び出せるか状態どうか
	virtual bool IsAvailable() const { return m_hInstance != NULL; }

	//! DLLロードと初期処理
	EDllResult InitDll(
		LPCTSTR pszSpecifiedDllName = NULL	//!< [in] クラスが定義しているDLL名以外のDLLを読み込みたいときに、そのDLL名を指定。
	);

	//! 終了処理とDLLアンロード
	bool DeinitDll(
		bool force = false	//!< [in] 終了処理に失敗してもDLLを解放するかどうか
	);

	//! インスタンスハンドルの取得
	HINSTANCE GetInstance() const { return m_hInstance; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           属性                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//! ロード済みDLLファイル名の取得。ロードされていない (またはロードに失敗した) 場合は NULL を返す。
	LPCTSTR GetLoadedDllName() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  オーバーロード可能実装                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	//!	DLLの初期化
	/*!
		DLLのロードに成功した直後に呼び出される．エントリポイントの
		確認などを行う．

		@retval true 正常終了
		@retval false 異常終了

		@note falseを返した場合は、読み込んだDLLを解放する．
	*/
	virtual bool InitDllImp() = 0;

	//!	関数の初期化
	/*!
		DLLのアンロードを行う直前に呼び出される．メモリの解放などを
		行う．

		@retval true 正常終了
		@retval false 異常終了

		@note falseを返したときはDLLのUnloadは行われない．
		@par 注意
		デストラクタからDeinitDll及びDeinitDllImpが呼び出されたときは
		ポリモーフィズムが行われないためにサブクラスのDeinitDllImpが呼び出されない。
		そのため、サブクラスのデストラクタではDeinitDllImpを明示的に呼び出す必要がある。
		
		DeinitDllがデストラクタ以外から呼び出される場合はDeinitDllImpは仮想関数として
		サブクラスのものが呼び出され、デストラクタは当然呼び出されないので
		DeinitDllImpそのものは必要である。
		
		デストラクタからDeinitDllImpを呼ぶときは、初期化されているという保証がないので
		呼び出し前にIsAvailableによる確認を必ず行うこと。
		
		@date 2002.04.15 genta 注意書き追加
	*/
	virtual bool DeinitDllImp();

	//! DLLファイル名の取得(複数を順次)
	/*!
		DLLファイル名として複数の可能性があり，そのうちの一つでも
		見つかったものを使用する場合に対応する．
		
		番号に応じてそれぞれ異なるファイル名を返すことができる．
		LoadLibrary()からはcounterを0から1ずつ増加させて順に呼びだされる．
		それはDLLのロードに成功する(成功)か，戻り値としてNULLを返す(失敗)
		まで続けられる．

		@param[in] nIndex インデックス．(0～)
		
		@return 引数に応じてDLL名(LoadLibraryに渡す文字列)，またはNULL．
	*/
	virtual LPCTSTR GetDllNameImp(int nIndex) = 0;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	bool RegisterEntries(const ImportTable table[]);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	HINSTANCE		m_hInstance;
	std::tstring	m_strLoadedDllName;
};

#endif



