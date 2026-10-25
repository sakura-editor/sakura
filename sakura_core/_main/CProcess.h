/*!	@file
	@brief プロセス基底クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#define SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "global.h"
#include "util/design_template.h"
#include "env/CShareData.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
 * @brief アプリ終了コード
 *
 * サクラエディタの終了コードを定義する。
 * 当初は「常にゼロを返す」な実装になっていた。
 * 実際は他の値を返すことがあり微妙になっていた。
 * 制御できる範囲を増やすため、終了コードを導入する。
 *
 * @note 仮の終了コード。
 * @note 将来的に拡張する可能性あり。
 */
enum class EAppExitCode : int {
	Exited,			//!< 終了した
	AbEnd,			//!< 異常終了した
	BadAlloc,		//!< メモリ不足で終了した
	Terminated,		//!< 強制終了された
	UnExpected,		//!< 予期せぬエラーで終了した
	Crashed,		//!< クラッシュした
	AppInitFailed,	//!< アプリ初期化が失敗した
};

/*!
	@brief プロセス基底クラス
*/
class CProcess : public TSingleInstance<CProcess> {
public:
	/*!
	 * @brief C++例外ハンドラー
	 */
	[[noreturn]] static void OnTerminated() noexcept;

	/*!
	 * @brief スタックトレースを出力する
	 *
	 * @param[in] context スタックトレースを取得するコンテキスト情報
	 * @param[in] out 出力先のストリーム
	 */
	static void WriteStackTraceIgnoringExceptions(
		CONTEXT& context,
		std::ostream* out
	) noexcept;

	CProcess( HINSTANCE hInstance, LPCWSTR lpCmdLine );

	~CProcess() override = default;

	/*!
	 * @brief プロセス実行
	 *
	 * @param[in] nCmdShow コマンドラインから渡されたウィンドウ表示フラグ
	 * @returns アプリ終了コード
	 */
	EAppExitCode Run(
		int nCmdShow
	);

	virtual void RefreshString();

	virtual std::filesystem::path GetIniFileName() const;

protected:
	virtual bool InitializeProcess(
		int nCmdShow [[maybe_unused]]
	);
	virtual bool MainLoop() = 0;

	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CShareData&		GetShareData()   { return m_cShareData; }
	HWND			GetMainWindow() const{ return m_hWnd; }

	[[nodiscard]] const CShareData* GetShareDataPtr() const { return &m_cShareData; }

private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd = nullptr;
	CShareData		m_cShareData;
};

#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
