/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#define SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <thread>
#include "doc/CDocListener.h"
#include "mem/CNativeW.h"
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

namespace grep {

/*!
	@brief ワーカーのスレッドで使う、中断の確認と進み具合の表示

	中断は、メインスレッドが立てるフラグで受け取る。確かめるたびにフラグを見る。
	画面に触るのはメインスレッドだけなので、表示はしない。
*/
class CGrepWorkerProgress : public IGrepProgress {
public:
	/*!
		@param[in]	bCancel	中断のフラグ(メインスレッドが立てる。このオブジェクトより長く残す)
	*/
	explicit CGrepWorkerProgress( const std::atomic<bool>& bCancel )
		: m_bCancel( bCancel )
	{
	}

	bool IsCheckTime() override
	{
		return true;
	}

	bool IsFileNameTime() override
	{
		return false;
	}

	bool IsCancelRequested() override
	{
		return m_bCancel;
	}

	void ApplyRealTimeView() override
	{
		// ワーカーは画面に触らない
	}

	void SetHitCount( int /*nHitCount*/ ) override
	{
		// ワーカーは表示しない(ヒット数はメインスレッドが結果を受け取ったときに表示する)
	}

	void SetFileName( std::wstring_view /*fileName*/ ) override
	{
		// ワーカーは表示しない(ファイル名はメインスレッドが表示する)
	}

	void SetFolder( std::wstring_view /*folder*/ ) override
	{
		// ワーカーは表示しない(フォルダーはメインスレッドが表示する)
	}

private:
	const std::atomic<bool>&	m_bCancel;	//!< 中断のフラグ(メインスレッドが立てる)
};

/*!
	@brief ワーカーの検索が終わったことを、メインスレッドに知らせる

	メインスレッドは先頭のファイルの結果を待つ間、ほかのワーカーが終わったら、すぐに次のファイルを渡す。
	知らせは数えておくので、WaitFor() を呼ぶ前の知らせも取りこぼさない。
*/
class CGrepWorkerSignal {
public:
	//! 検索が終わったことを知らせる(ワーカーのスレッドから呼ぶ)
	void Notify()
	{
		{
			std::scoped_lock lock( m_mutex );
			++m_nNotified;
		}
		m_cv.notify_one();
	}

	/*!
		前回の WaitFor() の後に知らせがあるまで、timeout を限りに待つ

		@param[in]	timeout	待つ時間の上限
		@retval true	知らせがあった
	*/
	bool WaitFor( std::chrono::milliseconds timeout )
	{
		std::unique_lock lock( m_mutex );
		const bool bNotified = m_cv.wait_for( lock, timeout, [this]{ return m_nSeen != m_nNotified; } );
		m_nSeen = m_nNotified;
		return bNotified;
	}

private:
	std::mutex				m_mutex;
	std::condition_variable	m_cv;
	size_t					m_nNotified = 0;	//!< 知らせた回数
	size_t					m_nSeen = 0;		//!< WaitFor() が見た回数
};

// 1 つのフォルダーの中のファイルを並列に検索するときの、ワーカーごとの正規表現を作る
std::vector<std::unique_ptr<CBregexp>> CreateWorkerRegexps( const CSearchStringPattern& pattern, const GrepInfo& gi, bool bHwnd, size_t nMaxThreads );

/*!
	@brief 検索したファイルの合計(KB)に、ファイル 1 つの大きさを足した結果
 */
struct SAddFileSizeResult {
	int		nWorkKB = 0;		//!< 足した後の合計(KB)。上限を超えるときは足す前のまま
	bool	bOverLimit = false;	//!< 足すと上限を超える(早めに出力する時機)

	//! 上限を超えるとき true
	explicit operator bool() const noexcept { return bOverLimit; }
};

// 検索したファイルの合計(KB)に、ファイル 1 つの大きさを足す(上限を超えるなら足さない)
SAddFileSizeResult AddFileSizeKB( int nWorkKB, ULONGLONG nFileSize );

} // namespace grep

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
		ULONGLONG		nFileSize = 0;		//!< ファイルの大きさ(バイト。リアルタイム表示を早めるかと、並列に検索するかの目安)
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
		std::span<CBregexp* const>	workerRegexps;	//!< ワーカーごとの正規表現(数が並列に検索するファイルの数の上限。空なら直列)
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
		bool			bWorker = false;			//!< ワーカーのスレッドで検索しているか(ファイルの途中で出力先に足さない。pOutput は使わない)
	};

	//! 検索するファイル 1 つの名前とパス
	struct SGrepFilePath {
		const WCHAR*	pszFile;		//!< 処理対象ファイル名(表示用)
		const WCHAR*	pszFullPath;	//!< 処理対象ファイルパス C:\Folder\SubFolder\File.ext
		const WCHAR*	pszBaseFolder;	//!< 検索フォルダー C:\Folder
		const WCHAR*	pszFolder;		//!< 結果に表示するフォルダー
		const WCHAR*	pszRelPath;		//!< 結果に表示する相対パス
	};

	//! ワーカーが検索したファイル 1 つぶんの結果
	struct SGrepFileResult {
		CNativeW		cmemMessage;		//!< 結果の文字列。見出しを出していれば先頭にある
		int				nHitCount = 0;		//!< ヒット数
		int				nRet = 0;			//!< DoGrepFile() の戻り値(中断したら -1)
		SGrepHeading	heading;			//!< 見出しを出したか(見出しを出していない状態から検索する)
	};

	//! ファイル 1 つを検索するワーカー。消すときに、スレッドの終わりを待つ
	struct SGrepFileWorker {
		std::future<SGrepFileResult>	result;		//!< 結果を受け取る
		std::jthread					thread;		//!< 検索するスレッド
		size_t							nRegexp = 0;		//!< 使っている正規表現(SGrepSearchCond::workerRegexps の添字)
		bool							bReleased = false;	//!< 検索が終わり、正規表現を空けたか
	};

	//! 並列に検索している間の、ワーカーの列と空いている正規表現
	struct SGrepWorkerQueue {
		grep::CGrepWorkerSignal		done;			//!< ワーカーの検索が終わったことの知らせ(ワーカーより長く残すので、先に置く)
		std::deque<SGrepFileWorker>	workers;		//!< 結果を受け取っていないワーカー(列挙の順)
		std::vector<size_t>			freeRegexps;	//!< 空いている正規表現(SGrepSearchCond::workerRegexps の添字)
		size_t						nMaxPending = 0;	//!< 結果を受け取っていないワーカーの数の上限
		size_t						nNext = 0;		//!< 次にワーカーに渡すファイル
	};

	//! 検索するフォルダーのパス一式(ワーカーに渡すときは、指す文字列は結果を受け取るまで残す)
	struct SGrepFolderPath {
		const WCHAR*	pszPath;		//!< 検索するフォルダー
		const WCHAR*	pszBasePath;	//!< 検索対象パス(ベースフォルダー)
		const WCHAR*	pszFolder;		//!< 結果に表示するフォルダー
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

	// 対象ファイル 1 つを、このスレッドで検索する(Grep置換なら置換する)
	int DoGrepTargetFile(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const SGrepTargetFile&	target,
		const SGrepFolderPath&	folder
	);

	// ファイル 1 つを検索した後、たまった結果を出力する時機なら出力する
	void AppendResultIfDue( SGrepOutput& out, const wchar_t* pszKey, ULONGLONG nFileSize, int& nWork, int& nHitCountOld );

	// フォルダー 1 つぶんの対象ファイルを並列に検索し、列挙した順に出力する
	int DoGrepFilesParallel(
		const SGrepSearchCond&	cond,
		CBregexp*				pRegexp,
		SGrepOutput&			out,
		const std::vector<SGrepTargetFile>& vFiles,
		const SGrepFolderPath&	folder
	);

	// ファイル 1 つの検索を、ワーカーのスレッドで始める(ワーカーの列の末尾に足す)
	bool LaunchGrepFile(
		SGrepWorkerQueue&		queue,
		const SGrepSearchCond&	cond,
		size_t					nRegexp,
		const std::atomic<bool>&	bCancel,
		const SGrepTargetFile&	target,
		const SGrepFolderPath&	folder
	);

	// 先頭のワーカーの検索が終わったかを返す(終わっていなければ、どれかのワーカーが終わるまで少し待つ)
	static bool WaitFrontWorker( SGrepWorkerQueue& queue );

	// 検索が終わったワーカーの正規表現を空け、空いた数だけ次のファイルをワーカーに渡す
	void LaunchGrepFiles(
		SGrepWorkerQueue&		queue,
		const SGrepSearchCond&	cond,
		const std::atomic<bool>&	bCancel,
		const std::vector<SGrepTargetFile>& vFiles,
		const SGrepFolderPath&	folder
	);

	// ワーカーが検索したファイル 1 つぶんの結果を、出力していない結果の末尾に足す
	static void MergeGrepFileResult(
		SGrepOutput&			out,
		const GrepInfo&			gi,
		std::wstring_view		baseFolder,
		std::wstring_view		folder,
		const SGrepFileResult&	result
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
	size_t	m_nMaxGrepThreads = 8;		//!< 1 つのフォルダーの中のファイルを並列に検索するスレッドの数の上限(コア数とファイルの数でも抑える。1 なら直列)
	ULONGLONG	m_nMaxParallelFileSize = 64 * 1024 * 1024;	//!< 並列に検索するファイルの大きさの上限(バイト)。超えるファイルはメインスレッドで検索する

};
#endif /* SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_ */
