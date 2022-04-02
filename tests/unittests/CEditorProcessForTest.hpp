/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <Windows.h>
#include <CommCtrl.h>

#include "_main/CCommandLine.h"
#include "_main/CNormalProcess.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CGrepAgent.h"

/*!
	テストコード用エディタプロセスクラス

	エディタプロセスはCEditWndクラスのインスタンスを作る。
 */
class CEditorProcessForTest final : public CProcess {
private:
	using CEditDocPtr = std::unique_ptr<CEditDoc>;
	using CEditWndPtr = std::unique_ptr<CEditWnd>;
	using CEditAppPtr = std::unique_ptr<CEditApp>;

	CCommandLine	m_cCommandLine;
	HRESULT			m_hrCoInit = E_UNEXPECTED;
	CEditDocPtr		m_pcEditDoc;
	CEditWndPtr		m_pcEditWnd;
	CEditAppPtr		m_pcEditApp;
	CMigemo			m_cMigemo;

public:
	CEditorProcessForTest();
	~CEditorProcessForTest();

	bool InitializeProcess() override;

protected:
	bool MainLoop() override { return false; }
	void OnExitProcess() override {}
};

void CControlProcess_Start(std::wstring_view profileName);
void CControlProcess_Terminate(std::wstring_view profileName);

#include <gtest/gtest.h>

/*!
 * CEditDocのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
template<typename TargetClass>
class TEditorProcessTest : public ::testing::Test {
protected:
	/*!
	 * プロファイル名
	 */
	static std::wstring_view profileName;

	/*!
	 * 設定ファイルのパス
	 *
	 * GetIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	static std::filesystem::path iniPath;

	/*!
	 * 最初のテストが起動される前に呼ばれる関数
	 */
	static void SetUpTestCase() {
		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}

		// コントロールプロセスを起動する
		CControlProcess_Start(profileName.data());
	}

	/*!
	 * 最後のテストが完了した後に呼ばれる関数
	 */
	static void TearDownTestCase() {
		// コントロールプロセスに終了指示を出して終了を待つ
		CControlProcess_Terminate(profileName.data());

		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}
	}

	/*!
	 * テストコード用エディタプロセス
	 */
	CEditorProcessForTest process;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テストコード用エディタプロセスを初期化する
		ASSERT_TRUE(process.InitializeProcess());
	}
};

//! プロファイル名
template<typename TargetClass>
std::wstring_view TEditorProcessTest<TargetClass>::profileName = L"";

//! INIファイルのパス
template<typename TargetClass>
std::filesystem::path TEditorProcessTest<TargetClass>::iniPath;

