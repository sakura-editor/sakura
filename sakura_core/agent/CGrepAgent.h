/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#define SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#pragma once

#include <optional>
#include <string_view>
#include "doc/CDocListener.h"
class CEditView;
class CSearchStringPattern;
class CGrepEnumKeys;
class CGrepEnumFiles;
class CGrepEnumFolders;
struct GrepInfo;

/*!
	@brief Grep の中断の確認と、進み具合の表示

	CGrepAgent::DoGrep() が、検索・置換の途中で呼ぶ。
	画面では「Grep実行中」のダイアログを使う。テストでは差し替えて、中断する位置を決めたり、表示した内容を確かめたりする。

	中断の確認と表示をどれだけ間引くかは、実装が決める(前回の時刻は実装が持つ)。
	CGrepAgent は時刻を持たない。
*/
struct IGrepProgress {
	virtual ~IGrepProgress() = default;

	//! 中断の確認をする時機かを返す(false なら IsCancelRequested() と表示を飛ばす)
	virtual bool IsCheckTime() = 0;
	//! 処理中のファイル名を表示する時機かを返す
	virtual bool IsFileNameTime() = 0;
	//! 処理中のユーザー操作を受け付け、中断するかを返す
	virtual bool IsCancelRequested() = 0;
	//! リアルタイム表示の指定を、すべてのビューの表示に反映する
	virtual void ApplyRealTimeView() = 0;
	//! ヒット数を表示する
	virtual void SetHitCount( int nHitCount ) = 0;
	//! 処理中のファイル名を表示する
	virtual void SetFileName( std::wstring_view fileName ) = 0;
	//! 処理中のフォルダーを表示する
	virtual void SetFolder( std::wstring_view folder ) = 0;
};

/*!
	@brief Grep の結果の出力先

	CGrepAgent::DoGrep() が、検索条件の見出し・結果・件数を順に足す。
	画面では Grep の結果の文書(-GOPT=U なら標準出力)に足す。テストでは差し替えて、足された文字列を確かめる。
*/
struct IGrepOutputTarget {
	virtual ~IGrepOutputTarget() = default;

	//! 結果を出力先の末尾に足す
	virtual void Append( const CNativeW& cmem ) = 0;
	//! 足した結果をすぐに表示しているか(リアルタイム表示)。途中の結果を早めに足すかを決めるのに使う
	virtual bool IsDrawing() const = 0;
};

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

	// Grep実行
	DWORD DoGrep(
		CEditView*				pcViewDst,	//!< [in] Grep結果の文書のビュー
		const GrepInfo&			gi,			//!< [in] Grep実行の入力一式(検索キー・対象・オプション)
		const std::optional<IGrepProgress*>&	optProgress = std::nullopt,	//!< [in] 中断の確認と進み具合の表示。指定が無ければ「Grep実行中」のダイアログを出す
		const std::optional<IGrepOutputTarget*>&	optOutput = std::nullopt	//!< [in] 結果の出力先。指定が無ければ pcViewDst の文書(-GOPT=U なら標準出力)に足す
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

	//! 見出し(ベースフォルダー名・フォルダー名)を出したか
	struct SGrepHeading {
		bool	bBaseFolder = false;	//!< ベースフォルダー名を出したか(検索するフォルダーごとに戻す)
		bool	bFolderName = false;	//!< フォルダー名を出したか(フォルダーごとに戻す)
	};

	//! 結果の出力先と、出力の途中の状態
	struct SGrepOutput {
		IGrepOutputTarget*	pOutput;				//!< 結果の出力先
		IGrepProgress*	pProgress;					//!< 中断の確認と進み具合の表示
		int*			pnHitCount;					//!< ヒット数の合計
		CNativeW&		cmemMessage;				//!< まだ出力していない結果
		CNativeW&		cUnicodeBuffer;				//!< 1 行の読み込みに使うバッファー
		SGrepHeading	heading;					//!< 見出しを出したか
	};

	//! 検索するファイル 1 つの名前とパス
	struct SGrepFilePath {
		const WCHAR*	pszFile;		//!< 処理対象ファイル名(表示用)
		const WCHAR*	pszFullPath;	//!< 処理対象ファイルパス C:\Folder\SubFolder\File.ext
		const WCHAR*	pszBaseFolder;	//!< 検索フォルダー C:\Folder
		const WCHAR*	pszFolder;		//!< 結果に表示するフォルダー
		const WCHAR*	pszRelPath;		//!< 結果に表示する相対パス
	};

	// 結果を出力先に足す(足す直前の時刻を覚える)
	void AppendResult( IGrepOutputTarget& output, const CNativeW& cmem );

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

	// Grep実行
	int DoGrepFile(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		SGrepOutput&			out,
		HWND					hWndTarget,
		const SGrepFilePath&	path
	);

	int DoGrepReplaceFile(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const SGrepFilePath&	path
	);

	ULONGLONG m_lastAddTailTime = 0;	// AppendResult() を呼び出した時間

public: //$$ 仮
	bool	m_bGrepMode = false;		//!< Grepモードか
	bool	m_bGrepRunning = false;		//!< Grep処理中
};
#endif /* SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_ */
