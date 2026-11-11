/*!	@file
	@brief マクロエンジン

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_
#define SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_
#pragma once

#include "basis/CMyString.h" //SFilePath

#include <Windows.h>

class CEditView;

class CMacroBeforeAfter {
private:
	using Me = CMacroBeforeAfter;

public:
	CMacroBeforeAfter() = default;

	CMacroBeforeAfter(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	virtual ~CMacroBeforeAfter() = default;

	virtual void ExecKeyMacroBefore(
		CEditView* pcEditView,
		int flags [[maybe_unused]]
	);

	virtual void ExecKeyMacroAfter(
		CEditView* pcEditView,
		int flags [[maybe_unused]],
		bool bRet [[maybe_unused]]
	);

private:
	int		m_nOpeBlkCount = 0;
	bool	m_bDrawSwitchOld = true;
};

/*!
	@brief マクロを処理するエンジン部分の基底クラス

*/
class CMacroManagerBase : protected CMacroBeforeAfter {
private:
	using Base = CMacroBeforeAfter;
	using Me = CMacroManagerBase;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	/*!
	 * 仮想メソッドを持つクラスなので、virtualなデストラクタが必要。
	 * 継承クラスなので、overrideにする。
	 * 独自の処理が何もないので = defaultにする。
	 */
	~CMacroManagerBase() override = default;

	/*! キーボードマクロの実行
	
		@param[in] pcEditView マクロ実行対象の編集ウィンドウ
		@param[in] flags マクロ実行属性．
		
		@date 2007.07.20 genta マクロ実行属性を渡すためにflagsを追加
	*/
	virtual bool ExecKeyMacro(
		CEditView* pcEditView,
		int flags
	) const = 0;
	
	virtual void ExecKeyMacro2(
		CEditView* pcEditView,
		int flags
	);
	
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
	
private:
	//!	Load済みかどうかを表すフラグ true...Load済み、false...未Load。
	bool m_nReady = false;

public:
	/*!	Load済みかどうか

		@retval true Load済み
		@retval false 未Load
	*/
	bool IsReady() const noexcept { return m_nReady; }

protected:
	void SetReady() noexcept { m_nReady = true; }

	void ResetReady() noexcept { m_nReady = false; }
};

#endif /* SAKURA_CMACROMANAGERBASE_40AD4654_61DF_45E5_BC2F_7C702E530B5D_H_ */
