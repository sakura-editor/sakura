/*! @file */
/*
	ObserverパターンのCEditDoc特化版。
	CDocSubjectは観察され、CDocListnerは観察を行う。
	観察の開始は CDocListener::Listen で行う。

	$Note:
		Listener (Observer) と Subject のリレーション管理は
		ジェネリックな汎用モジュールに分離できる。
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCLISTENER_BEF5B814_A5B8_4D07_9B2F_009A5CB29B2F_H_
#define SAKURA_CDOCLISTENER_BEF5B814_A5B8_4D07_9B2F_009A5CB29B2F_H_
#pragma once

#include "basis/CMyString.h"
#include "charset/charset.h"
#include "basis/CEol.h"
#include "types/CType.h"
#include "util/relation_tool.h"

class CDocListener;

//###
enum ESaveResult{
	SAVED_OK,
	SAVED_FAILURE,
	SAVED_INTERRUPT,//!< 中断された
	SAVED_LOSESOME,	//!< 文字の一部が失われた
};

//###
enum ELoadResult{
	LOADED_OK,
	LOADED_FAILURE,
	LOADED_INTERRUPT,	//!< 中断された
	LOADED_LOSESOME,	//!< 文字の一部が失われた

	//特殊
	LOADED_NOIMPLEMENT,	//!< 実装無し
};

//###
enum ECallbackResult{
	CALLBACK_CONTINUE,			//!< 続ける
	CALLBACK_INTERRUPT,			//!< 中断
};

//###
struct SLoadInfo
{
	//入力
	CFilePath	cFilePath;
	ECodeType	eCharCode = CODE_AUTODETECT;
	bool		bViewMode = false;
	bool		bWritableNoMsg = false; //!< 書き込み禁止メッセージを表示しない
	CTypeConfig	nType{ -1 };

	//モード
	bool		bRequestReload = false;	//リロード要求

	//出力
	bool		bOpened = false;

	SLoadInfo() = default;
	SLoadInfo(const CFilePath& _cFilePath, ECodeType _eCodeType, bool _bReadOnly, CTypeConfig _nType = CTypeConfig(-1))
	: cFilePath(_cFilePath)
	, eCharCode(_eCodeType)
	, bViewMode(_bReadOnly)
	, nType(_nType)
	{
	}

	//! ファイルパスの比較
	bool IsSamePath(LPCWSTR pszPath) const;
};

struct SSaveInfo{
	CFilePath	cFilePath;						//!< 保存ファイル名
	ECodeType	eCharCode = CODE_AUTODETECT;	//!< 保存文字コードセット
	bool		bBomExist = false;				//!< 保存時BOM付加
	bool		bChgCodeSet = false;			//!< 文字コードセット変更	2013/5/19 Uchi
	CEol		cEol{ EEolType::none };			//!< 保存改行コード

	//モード
	bool		bOverwriteMode = false;			//!< 上書き要求

	SSaveInfo() = default;
	SSaveInfo(const CFilePath& _cFilePath, ECodeType _eCodeType, const CEol& _cEol, bool _bBomExist)
		: cFilePath(_cFilePath)
		, eCharCode(_eCodeType)
		, bBomExist(_bBomExist)
		, cEol(_cEol)
	{
	}

	//! ファイルパスの比較
	bool IsSamePath(LPCWSTR pszPath) const;
};

class CProgressListener;

//! 複数のCProgressSubjectからウォッチされる
class CProgressSubject : public CSubjectT<CProgressListener>{
public:
	~CProgressSubject() override = default;

	void NotifyProgress(int nPer);
};

//! 1つのCProgressSubjectをウォッチする
class CProgressListener : public CListenerT<CProgressSubject>{
public:
	~CProgressListener() override = default;

	virtual void OnProgress(int nPer)=0;
};

//Subjectは複数のListenerから観察される
class CDocSubject : public CSubjectT<CDocListener>{
public:
	~CDocSubject() override;

	//ロード前後
	ECallbackResult NotifyCheckLoad	(SLoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(SLoadInfo* sLoadInfo);
	ELoadResult NotifyLoad			(const SLoadInfo& sLoadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const SLoadInfo& sLoadInfo);
	void NotifyFinalLoad			(ELoadResult eLoadResult);

	//セーブ前後
	ECallbackResult NotifyCheckSave	(SSaveInfo* pSaveInfo);
	ECallbackResult NotifyPreBeforeSave(SSaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SSaveInfo& sSaveInfo);
	void NotifySave					(const SSaveInfo& sSaveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SSaveInfo& sSaveInfo);
	void NotifyFinalSave			(ESaveResult eSaveResult);

	//クローズ前後
	ECallbackResult NotifyBeforeClose();
};

//Listenerは1つのSubjectを観察する
class CDocListener : public CListenerT<CDocSubject>{
public:
	explicit CDocListener(CDocSubject* pcDoc = nullptr);
	~CDocListener() override;

	// -- -- 属性 -- -- //
	CDocSubject* GetListeningDoc() const { return GetListeningSubject(); }

	// -- -- 各種イベント -- -- //
	//ロード前後
	virtual ECallbackResult	OnCheckLoad	([[maybe_unused]] SLoadInfo* pLoadInfo)		 { return CALLBACK_CONTINUE; }	//!< 本当にロードを行うかの判定を行う
	virtual void			OnBeforeLoad([[maybe_unused]] SLoadInfo* sLoadInfo)       { return ; }					//!< ロード事前処理
	virtual ELoadResult		OnLoad		([[maybe_unused]] const SLoadInfo& sLoadInfo) { return LOADED_NOIMPLEMENT; }	//!< ロード処理
	virtual void			OnLoading	([[maybe_unused]] int nPer)					 { return ; }							//!< ロード処理の経過情報を受信
	virtual void			OnAfterLoad	([[maybe_unused]] const SLoadInfo& sLoadInfo) { return ; }					//!< ロード事後処理
	virtual void			OnFinalLoad	([[maybe_unused]] ELoadResult eLoadResult)	 { return ; }					//!< ロードフローの最後に必ず呼ばれる

	//セーブ前後
	virtual ECallbackResult OnCheckSave	([[maybe_unused]] SSaveInfo* pSaveInfo)		 { return CALLBACK_CONTINUE; }	//!< 本当にセーブを行うかの判定を行う
	virtual ECallbackResult OnPreBeforeSave	([[maybe_unused]] SSaveInfo* pSaveInfo)	 { return CALLBACK_CONTINUE; }	//!< セーブ事前おまけ処理 ($$ 仮)
	virtual void			OnBeforeSave([[maybe_unused]] const SSaveInfo& sSaveInfo) { return ; }					//!< セーブ事前処理
	virtual void			OnSave		([[maybe_unused]] const SSaveInfo& sSaveInfo) { return ; }					//!< セーブ処理
	virtual void			OnSaving	([[maybe_unused]] int nPer)					 { return ; }							//!< セーブ処理の経過情報を受信
	virtual void			OnAfterSave	([[maybe_unused]] const SSaveInfo& sSaveInfo) { return ; }					//!< セーブ事後処理
	virtual void			OnFinalSave	([[maybe_unused]] ESaveResult eSaveResult)	 { return ; }					//!< セーブフローの最後に必ず呼ばれる

	//クローズ前後
	virtual ECallbackResult OnBeforeClose()							 { return CALLBACK_CONTINUE; }
};

//GetListeningDocの利便性をアップ
class CEditDoc;
class CDocListenerEx : public CDocListener{
public:
	explicit CDocListenerEx(CDocSubject* pcDoc = nullptr)
		: CDocListener(pcDoc)
	{
	}

	CEditDoc* GetListeningDoc() const;
};

#include <exception>
class CFlowInterruption : public std::exception{
public:
	const char* what() const throw() override{ return "CFlowInterruption"; }
};

#endif /* SAKURA_CDOCLISTENER_BEF5B814_A5B8_4D07_9B2F_009A5CB29B2F_H_ */
