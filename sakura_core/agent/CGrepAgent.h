/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#define SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#pragma once

#include "doc/CDocListener.h"
class CDlgCancel;
class CEditView;
class CSearchStringPattern;
class CGrepEnumKeys;
class CGrepEnumFiles;
class CGrepEnumFolders;
struct GrepInfo;

//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
//	Mar. 28, 2004 genta DoGrepFileから不要な引数を削除
class CGrepAgent : public CDocListenerEx{
public:
	CGrepAgent();

	// イベント
	ECallbackResult OnBeforeClose() override;
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

	static void CreateFolders( const WCHAR* pszPath, std::vector<std::wstring>& vPaths );
	static std::wstring ChopYen( const std::wstring& str );
	void AddTail( CEditView* pcEditView, const CNativeW& cmem, bool bAddStdout );

	// Grep実行
	DWORD DoGrep(
		CEditView*				pcViewDst,	//!< [in] Grep結果の出力先
		const GrepInfo&			gi			//!< [in] Grep実行の入力一式(検索キー・対象・オプション)
	);

private:
	//! Grep の対象ファイル 1 つぶん(フォルダー 1 つの列挙結果の要素)
	struct SGrepTargetFile {
		std::wstring	strFileName;		//!< 検索するフォルダーからの相対のファイル名(検索キーにフォルダーを含むと sub\a.txt の形)
		DWORD			dwFileSizeLow = 0;	//!< ファイルサイズの下位 32 ビット(リアルタイム表示を早めるかの目安)
	};

	// フォルダー 1 つの中から、Grep の対象ファイルを列挙する
	static std::vector<SGrepTargetFile> EnumerateGrepTargetFiles(
		const WCHAR*			pszPath,
		CGrepEnumKeys&			cGrepEnumKeys,
		CGrepEnumFiles&			cGrepExceptAbsFiles
	);

	//! Grep 1 回ぶんの検索の条件(全ファイルで共通。読み取り専用)
	struct SGrepSearchCond {
		const wchar_t*				pszKey;			//!< 検索キー
		const CNativeW&				cmGrepReplace;	//!< 置換後文字列(置換のときだけ使う)
		const SSearchOption&		sSearchOption;	//!< 検索オプション
		const GrepInfo&				sGrepOption;	//!< Grep実行の入力一式(正規化済み)
		const CSearchStringPattern&	pattern;		//!< 検索パターン
	};

	//! フォルダーの列挙の条件(全フォルダーで共通)
	struct SGrepEnumCond {
		CGrepEnumKeys&		cGrepEnumKeys;			//!< 検索対象ファイルパターン
		CGrepEnumFiles&		cGrepExceptAbsFiles;	//!< 除外ファイル絶対パス
		CGrepEnumFolders&	cGrepExceptAbsFolders;	//!< 除外フォルダー絶対パス
	};

	//! 結果の出力先と、出力の途中の状態
	struct SGrepOutput {
		CEditView*		pcViewDst;					//!< 結果の出力先
		CDlgCancel*		pcDlgCancel;				//!< Cancelダイアログ
		int*			pnHitCount;					//!< ヒット数の合計
		CNativeW&		cmemMessage;				//!< まだ出力していない結果
		CNativeW&		cUnicodeBuffer;				//!< 1 行の読み込みに使うバッファー
		bool			bOutputBaseFolder = false;	//!< ベースフォルダー名を出力したか
	};

	//! 検索するファイル 1 つの名前とパス
	struct SGrepFilePath {
		const WCHAR*	pszFile;		//!< 処理対象ファイル名(表示用)
		const WCHAR*	pszFullPath;	//!< 処理対象ファイルパス C:\Folder\SubFolder\File.ext
		const WCHAR*	pszBaseFolder;	//!< 検索フォルダー C:\Folder
		const WCHAR*	pszFolder;		//!< 結果に表示するフォルダー
		const WCHAR*	pszRelPath;		//!< 結果に表示する相対パス
	};

	// Grep実行
	int DoGrepTree(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const SGrepEnumCond&	enumCond,
		const WCHAR*			pszPath,			//!< [in] 検索対象パス
		const WCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベースフォルダー)
		int						nNest				//!< [in] ネストレベル
	);

	// フォルダー 1 つぶんの対象ファイルを、列挙した順に検索する
	int DoGrepFiles(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const std::vector<SGrepTargetFile>& vFiles,
		const WCHAR*			pszPath,
		const WCHAR*			pszBasePath
	);

	// 処理中のユーザー操作を受け付け、中断されたかを返す
	bool IsGrepCanceled( CDlgCancel* pcDlgCancel, DWORD dwNow );

	// Grep実行
	int DoGrepFile(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		SGrepOutput&			out,
		HWND					hWndTarget,
		const SGrepFilePath&	path,
		bool&					bOutputFolderName
	);

	int DoGrepReplaceFile(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const SGrepFilePath&	path,
		bool&					bOutputFolderName
	);

	DWORD m_dwTickAddTail = 0;	// AddTail() を呼び出した時間
	DWORD m_dwTickUICheck = 0;	// 処理中にユーザーによるUI操作が行われていないか確認した時間
	DWORD m_dwTickUIFileName = 0;	// Cancelダイアログのファイル名表示更新を行った時間

public: //$$ 仮
	bool	m_bGrepMode = false;		//!< Grepモードか
	bool	m_bGrepRunning = false;		//!< Grep処理中
};
#endif /* SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_ */
