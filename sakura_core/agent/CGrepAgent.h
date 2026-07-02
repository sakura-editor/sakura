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
#include <string>
#include <vector>
#include "doc/CDocListener.h"
#include "extmodule/CBregexp.h"
class CDlgCancel;
class CEditView;
class CSearchStringPattern;
class CGrepEnumKeys;
class CGrepEnumFiles;
class CGrepEnumFolders;

//! 並列Grep処理で使用するファイルタスク情報
// 1 ファイルを 1 タスクとして扱い、検索対象の実体情報と表示用文字列を分けて保持する。
struct SGrepFileTask {
	std::wstring fullPath;    //!< 処理対象ファイルのフルパス
	std::wstring fileName;    //!< ファイル名（タイプ別設定取得用）
	std::wstring baseFolder;  //!< ベースフォルダー
	std::wstring folder;      //!< 表示用フォルダー（bGrepSeparateFolder時）
	std::wstring relPath;     //!< 相対パス（bGrepSeparateFolder時はファイル名のみ）
};

struct SGrepOption{
	bool		bGrepReplace;			//!< Grep置換
	bool		bGrepSubFolder;			//!< サブフォルダーからも検索する
	bool		bGrepStdout;			//!< 標準出力モード
	bool		bGrepHeader;			//!< ヘッダー・フッター表示
	ECodeType	nGrepCharSet;			//!< 文字コードセット選択
	int			nGrepOutputLineType;	//!< 0:ヒット部分を出力, 1: ヒット行を出力, 2: 否ヒット行を出力
	int			nGrepOutputStyle;		//!< 出力形式 1: Normal, 2: WZ風(ファイル単位) 3: 結果のみ
	bool		bGrepOutputFileOnly;	//!< ファイル毎最初のみ検索
	bool		bGrepOutputBaseFolder;	//!< ベースフォルダー表示
	bool		bGrepSeparateFolder;	//!< フォルダー毎に表示
	bool		bGrepPaste;				//!< Grep置換：クリップボードから貼り付ける
	bool		bGrepBackup;			//!< Grep置換：バックアップ

	SGrepOption() :
		 bGrepReplace(false)
		,bGrepSubFolder(true)
		,bGrepStdout(false)
		,bGrepHeader(true)
		,nGrepCharSet(CODE_AUTODETECT)
		,nGrepOutputLineType(1)
		,nGrepOutputStyle(1)
		,bGrepOutputFileOnly(false)
		,bGrepOutputBaseFolder(false)
		,bGrepSeparateFolder(false)
		,bGrepPaste(false)
		,bGrepBackup(false)
	{}
};

//! Grep の検索対象・除外をまとめるオブジェクト群（非所有参照）
struct SGrepEnumContext {
	CGrepEnumKeys&		keys;			//!< [in] 検索対象/除外ファイルパターン(!で除外)
	CGrepEnumFiles&		exceptFiles;	//!< [in] 除外ファイル（絶対パス）
	CGrepEnumFolders&	exceptFolders;	//!< [in] 除外フォルダー（絶対パス）
};

//! 並列 Grep の検索条件（非所有参照）
struct SGrepSearchParams {
	const wchar_t*			pszKey;			//!< [in] 検索パターン（c_str）
	const SSearchOption&	sSearchOption;	//!< [in] 検索オプション
	const SGrepOption&		sGrepOption;	//!< [in] Grep オプション
};

//! 検索結果 1 件のヒット位置情報
struct SGrepMatchInfo {
	LONGLONG		nLine;			//!< [in] ヒットした行番号
	int				nColumn;		//!< [in] ヒットした桁番号
	const wchar_t*	pCompareData;	//!< [in] 行の文字列
	int				nLineLen;		//!< [in] 行の文字列長
	int				nEolCodeLen;	//!< [in] EOL の長さ
	const wchar_t*	pMatchData;		//!< [in] ヒットした文字列
	int				nMatchLen;		//!< [in] ヒットした文字列長
};

//! DoGrep の検索入力文字列（非所有）
struct SGrepInput {
	const CNativeW*	pcmGrepKey;		//!< [in] 検索キー
	const CNativeW*	pcmGrepReplace;	//!< [in] 置換文字列
	const CNativeW*	pcmGrepFile;	//!< [in] 対象ファイルパターン
	const CNativeW*	pcmGrepFolder;	//!< [in] 検索フォルダー
};

//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
//	Mar. 28, 2004 genta DoGrepFileから不要な引数を削除
class CGrepAgent : public CDocListenerEx{
public:
	CGrepAgent();

	// イベント
	ECallbackResult OnBeforeClose() override;
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

	//! 検索結果 1 件分のフォーマット生成（HWND 非依存）
	static void FormatGrepResultLine(
		CNativeW&				cmemMessage,	//!< [out] 整形結果の追記先
		const WCHAR*			pszFilePath,	//!< [in]  表示用ファイルパス
		const WCHAR*			pszCodeName,	//!< [in]  文字コード表記（"[SJIS]" 等）
		const SGrepMatchInfo&	mi,				//!< [in]  ヒット位置情報
		const SGrepOption&		sGrepOption		//!< [in]  Grep オプション
	);

	//! 結果ヘッダ生成（"検索条件 ..."）
	static CNativeW BuildGrepHeader(
		const wchar_t* pszKey,
		const wchar_t* pszFile,
		const wchar_t* pszFolder,
		const SSearchOption& sSearchOption,
		const SGrepOption& sGrepOption,
		const wchar_t* pszReplace = nullptr
	);

	//! 結果フッタ生成（"該当 N 件" / "N 件を置換"）
	static CNativeW BuildGrepFooter(int nHitCount, bool bGrepReplace = false);

	static void CreateFolders( const WCHAR* pszPath, std::vector<std::wstring>& vPaths );
	static std::wstring ChopYen( const std::wstring& str );
	void AddTail( CEditView* pcEditView, const CNativeW& cmem, bool bAddStdout );

	// Grep実行
	DWORD DoGrep(
		CEditView*				pcViewDst,				//!< [in]  出力先ビュー
		const SGrepInput&		grepInput,				//!< [in]  検索入力（キー/置換/ファイル/フォルダー）
		const SSearchOption&	sSearchOption,			//!< [in]  検索オプション
		SGrepOption				sGrepOption,			//!< [in]  Grep オプション（内部で正規化するため値渡し）
		bool					bGrepCurFolder,			//!< [in]  カレントフォルダーを変更しない
		bool					bGrepExcludeFileRegexp	//!< [in]  除外ファイルを正規表現として扱う
	);

	// 並列 Grep のワーカー本体。UI に触れず、1 ファイル分の検索だけを担当する。
	// テストから直接呼び出せるように public 化。
	int DoGrepFileWorker(
		const SGrepSearchParams&	searchParams,	//!< [in]  検索条件（キー/検索/Grep オプション）
		const SGrepFileTask&		task,			//!< [in]  検索対象 1 ファイル
		CBregexp*					pLocalRegexp,	//!< [in]  スレッド専用正規表現（コンパイル済み）
		const CSearchStringPattern&	localPattern,	//!< [in]  スレッド専用検索パターン
		const std::atomic<bool>&	bCancelled,		//!< [in]  キャンセル要求フラグ
		CNativeW&					cmemMessage,	//!< [out] 結果メッセージ追記先
		CNativeW&					cUnicodeBuffer	//!< [i/o] 変換用テンポラリ
	);

	// 並列 Grep のスレッドプール管理・バッチ駆動コア。
	// pcViewDst / pcDlgCancel は nullptr 可（nullptr 時は UI 操作をスキップ）。
	// @retval -1 キャンセル   @retval 0 完了
	// @param[out] nHitCountOut 合計ヒット数（キャンセル時も部分カウントを返す）
	int RunParallelGrep(
		CEditView*							pcViewDst,		//!< [in]  出力先ビュー（nullptr 可）
		CDlgCancel*							pcDlgCancel,	//!< [in]  Cancelダイアログ（nullptr 可）
		const SGrepSearchParams&			searchParams,	//!< [in]  検索条件（キー/検索/Grep オプション）
		const SGrepEnumContext&				enumCtx,		//!< [in]  列挙・除外オブジェクト群
		const std::vector<std::wstring>&	vPaths,			//!< [in]  検索対象パス群
		CNativeW&							cmemMessage,	//!< [out] 結果メッセージ追記先
		int&								nHitCountOut	//!< [out] 合計ヒット数（キャンセル時も部分値）
	);

private:
	// Grep実行
	int DoGrepTree(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,		//!< [in] Cancelダイアログへのポインタ
		const wchar_t*			pszKey,				//!< [in] 検索パターン
		const CNativeW&			cmGrepReplace,
		CGrepEnumKeys&			cGrepEnumKeys,		//!< [in] 検索対象ファイルパターン(!で除外指定)
		CGrepEnumFiles&			cGrepExceptAbsFiles,
		CGrepEnumFolders&		cGrepExceptAbsFolders,
		const WCHAR*			pszPath,			//!< [in] 検索対象パス
		const WCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベース)
		const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
		const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
		const CSearchStringPattern& pattern,		//!< [in] 検索パターン
		CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
		bool&					bOutputBaseFolder,
		int*					pnHitCount,			//!< [i/o] ヒット数の合計
		CNativeW&				cmemMessage,
		CNativeW&				cUnicodeBuffer,
		std::vector<CBregexp>* pExclRegexps = nullptr //!< [in] コンパイル済み除外正規表現（再帰呼び出し用・nullptrなら内部でコンパイル）
	);

	// Grep実行
	int DoGrepFile(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,
		HWND					hWndTarget,
		const wchar_t*			pszKey,
		const WCHAR*			pszFile,
		const SSearchOption&	sSearchOption,
		const SGrepOption&		sGrepOption,
		const CSearchStringPattern& pattern,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		int*					pnHitCount,
		const WCHAR*			pszFullPath,
		const WCHAR*			pszBaseFolder,
		const WCHAR*			pszFolder,
		const WCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		CNativeW&				cmemMessage,
		CNativeW&				cUnicodeBuffer
	);

	int DoGrepReplaceFile(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,
		const wchar_t*			pszKey,
		const CNativeW&			cmGrepReplace,
		const WCHAR*			pszFile,
		const SSearchOption&	sSearchOption,
		const SGrepOption&		sGrepOption,
		const CSearchStringPattern& pattern,
		CBregexp*				pRegexp,
		int*					pnHitCount,
		const WCHAR*			pszFullPath,
		const WCHAR*			pszBaseFolder,
		const WCHAR*			pszFolder,
		const WCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		CNativeW&				cmemMessage,
		CNativeW&				cUnicodeBuffer
	);

	// Grep結果をpszWorkに格納
	void SetGrepResult(
		CNativeW&				cmemMessage,	//!< [out] 整形結果の追記先
		const WCHAR*			pszFilePath,	//!< [in]  フルパス or 相対パス
		const WCHAR*			pszCodeName,	//!< [in]  文字コード情報"[SJIS]"とか
		const SGrepMatchInfo&	mi,				//!< [in]  ヒット位置情報
		const SGrepOption&		sGrepOption		//!< [in]  Grep オプション
	);

	// ファイル列挙（メインスレッド用・キャンセル対応）
	// 検索は行わず、ワーカーへ渡すタスクだけを作る。
	void DoGrepTreeEnumerate(
		CDlgCancel*					pcDlgCancel,	//!< [in]  Cancelダイアログ（nullptr 可）
		const SGrepOption&			sGrepOption,	//!< [in]  Grep オプション（パス整形フラグ用）
		const SGrepEnumContext&		enumCtx,		//!< [in]  列挙・除外オブジェクト群
		const WCHAR*				pszPath,		//!< [in]  列挙対象パス
		const WCHAR*				pszBasePath,	//!< [in]  ベースパス
		std::vector<SGrepFileTask>&	vecTasks,		//!< [out] 生成されたファイルタスク
		bool&						bCancelled		//!< [out] キャンセルされたら true
	);

	// UICHECK 間隔ごとのキャンセル確認・表示設定更新（DoGrepTreeEnumerate 用）
	// @retval 0 続行 / -1 キャンセル（BlockingHook 失敗 or 中断ボタン）
	int CheckGrepCancelUI( CDlgCancel* pcDlgCancel, DWORD dwNow );

	DWORD m_dwTickAddTail = 0;	// AddTail() を呼び出した時間
	DWORD m_dwTickUICheck = 0;	// 処理中にユーザーによるUI操作が行われていないか確認した時間
	DWORD m_dwTickUIFileName = 0;	// Cancelダイアログのファイル名表示更新を行った時間

public: //$$ 仮
	bool	m_bGrepMode = false;		//!< Grepモードか
	bool	m_bGrepRunning = false;		//!< Grep処理中
};
#endif /* SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_ */
