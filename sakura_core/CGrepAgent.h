﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
		CEditView*				pcViewDst,
		bool					bGrepReplace,
		const CNativeW*			pcmGrepKey,
		const CNativeW*			pcmGrepReplace,
		const CNativeW*			pcmGrepFile,
		const CNativeW*			pcmGrepFolder,
		bool					bGrepCurFolder,
		BOOL					bGrepSubFolder,
		bool					bGrepStdout,
		bool					bGrepHeader,
		const SSearchOption&	sSearchOption,
		ECodeType				nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
		int						nGrepOutputLineType,
		int						nGrepOutputStyle,
		bool					bGrepOutputFileOnly,	//!< [in] ファイル毎最初のみ出力
		bool					bGrepOutputBaseFolder,	//!< [in] ベースフォルダー表示
		bool					bGrepSeparateFolder,	//!< [in] フォルダー毎に表示
		bool					bGrepPaste,
		bool					bGrepBackup
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
		int						nNest,				//!< [in] ネストレベル
		bool&					bOutputBaseFolder,
		int*					pnHitCount,			//!< [i/o] ヒット数の合計
		CNativeW&				cmemMessage,
		CNativeW&				cUnicodeBuffer
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
		// データ格納先
		CNativeW&		cmemMessage,
		// マッチしたファイルの情報
		const WCHAR*	pszFilePath,	//	フルパス or 相対パス
		const WCHAR*	pszCodeName,	//	文字コード情報"[SJIS]"とか
		// マッチした行の情報
		LONGLONG		nLine,			//	マッチした行番号
		int				nColumn,		//	マッチした桁番号
		const wchar_t*	pCompareData,	//	行の文字列
		int				nLineLen,		//	行の文字列の長さ
		int				nEolCodeLen,	//	EOLの長さ
		// マッチした文字列の情報
		const wchar_t*	pMatchData,		//	マッチした文字列
		int				nMatchLen,		//	マッチした文字列の長さ
		// オプション
		const SGrepOption&	sGrepOption
	);

	DWORD m_dwTickAddTail = 0;	// AddTail() を呼び出した時間
	DWORD m_dwTickUICheck = 0;	// 処理中にユーザーによるUI操作が行われていないか確認した時間
	DWORD m_dwTickUIFileName = 0;	// Cancelダイアログのファイル名表示更新を行った時間

public: //$$ 仮
	bool	m_bGrepMode = false;		//!< Grepモードか
	bool	m_bGrepRunning = false;		//!< Grep処理中
};
#endif /* SAKURA_CGREPAGENT_97F2B632_71C8_4E4A_AC42_13A6098B248F_H_ */
