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
#include "StdAfx.h"
#include "CEditorProcessForTest.hpp"

static constexpr auto& EMPTY_STRING = L"";

CEditorProcessForTest::CEditorProcessForTest()
	: CProcess(::GetModuleHandle(nullptr), EMPTY_STRING)
{
	// 共有メモリを初期化するにはコマンドラインのインスタンスが必要
	m_cCommandLine.ParseCommandLine(EMPTY_STRING, false);
}

/*!
	デストラクタ
 */
CEditorProcessForTest::~CEditorProcessForTest()
{
	// エディタウインドウを解放する
	m_pcEditWnd = nullptr;

	// エディタアプリを解放する
	m_pcEditApp = nullptr;

	// ドキュメントは最後に解放する
	m_pcEditDoc = nullptr;

	if (SUCCEEDED(m_hrCoInit))
	{
		::OleUninitialize();
	}
}

/*!
	@brief エディタプロセスを初期化する
	
	CEditWndを作成する。
 */
bool CEditorProcessForTest::InitializeProcess()
{
	if (m_hrCoInit = ::OleInitialize(nullptr);
		FAILED(m_hrCoInit))
	{
		return false;
	}

	/* 共有メモリを初期化する */
	if (!CProcess::InitializeProcess()) {
		return false;
	}

	// ドキュメントのインスタンスを生成する
	if (m_pcEditDoc = std::make_unique<CEditDoc>(nullptr);
		m_pcEditDoc == nullptr)
	{
		return false;
	}

	// エディタアプリのインスタンスを生成
	if (m_pcEditApp = std::make_unique<CEditApp>();
		m_pcEditApp == nullptr)
	{
		return false;
	}

	// 編集ウインドウのインスタンスを生成する
	if (m_pcEditWnd = std::make_unique<CEditWnd>();
		m_pcEditWnd == nullptr)
	{
		return false;
	}

	return true;
}
