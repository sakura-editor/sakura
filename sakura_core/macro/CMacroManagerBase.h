/*!	@file
	@brief マクロエンジン

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_
#define SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_
#pragma once

#include <Windows.h>
class CEditView;

class CMacroBeforeAfter {
public:
	CMacroBeforeAfter() : m_nOpeBlkCount(0), m_bDrawSwitchOld(true){};
	virtual ~CMacroBeforeAfter(){};
	virtual void ExecKeyMacroBefore( class CEditView* pcEditView, int flags );
	virtual void ExecKeyMacroAfter( class CEditView* pcEditView, int flags, bool bRet );
private:
	int m_nOpeBlkCount;
	bool m_bDrawSwitchOld;
};

/*!
	@brief マクロを処理するエンジン部分の基底クラス

*/
class CMacroManagerBase : CMacroBeforeAfter {
public:

	/*! キーボードマクロの実行
	
		@param[in] pcEditView マクロ実行対象の編集ウィンドウ
		@param[in] flags マクロ実行属性．
		
		@date 2007.07.20 genta マクロ実行属性を渡すためにflagsを追加
	*/
	virtual bool ExecKeyMacro( class CEditView* pcEditView, int flags ) const = 0;
	virtual void ExecKeyMacro2( class CEditView* pcEditView, int flags );
	
	/*! キーボードマクロをファイルから読み込む

		@param hInstance [in]
		@param pszPath [in] ファイル名
	*/
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const WCHAR* pszPath) = 0;

	/*! キーボードマクロを文字列から読み込む

		@param hInstance [in]
		@param pszCode [in] マクロコード
	*/
	virtual BOOL LoadKeyMacroStr( HINSTANCE hInstance, const WCHAR* pszCode ) = 0;

	//static CMacroManagerBase* Creator( const char* str );
	//純粋仮想クラスは実体化できないのでFactoryは不要。
	//継承先クラスでは必要。
	
	//	デストラクタのvirtualを忘れずに
	virtual ~CMacroManagerBase();
	
protected:
	//!	Load済みかどうかを表すフラグ true...Load済み、false...未Load。
	bool m_nReady;

public:
	/*!	Load済みかどうか

		@retval true Load済み
		@retval false 未Load
	*/
	bool IsReady(){ return m_nReady; }

	// Constructor
	CMacroManagerBase();
};
#endif /* SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_ */
