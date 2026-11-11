/*!	@file
	@brief プロセス生成クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_
#define SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_
#pragma once

#include "_main/CCommandLine.h"
#include "_main/CProcess.h"

#include <memory>
#include <string_view>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス生成クラス

	与えられたコマンドライン引数から生成すべきプロセスの種別を判定し，
	対応するオブジェクトを返すFactoryクラス．

	通常のエディタプロセスの起動が指定された場合には，必要に応じてコントロールプロセス
	起動の起動をエディタの起動に先立って行う．
*/
class CProcessFactory final {
private:
	using CProcessHolder = std::unique_ptr<CProcess>;

public:
	static bool	IsExistControlProcess();
	static bool	StartControlProcess();

	explicit CProcessFactory(
		_In_opt_ HINSTANCE hInstance = nullptr
	);

	CProcessHolder CreateInstance(
		std::wstring_view commandLine
	) const;

private:
	bool	IsValidVersion() const;

	bool ProfileSelect(
		CCommandLine& cCommandLine
	) const;

	bool	IsStartingControlProcess() const;

	HINSTANCE	m_hInstance;
};

#endif /* SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_ */
