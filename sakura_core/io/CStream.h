/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_
#define SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_
#pragma once

class CFileAttribute;

//! 例外：ファイルオープンに失敗
class CError_FileOpen {
public:
	enum EReason {
		UNKNOWN,
		TOO_BIG
	};
public:
	CError_FileOpen() : m_reason(UNKNOWN) {}
	CError_FileOpen(EReason reason) : m_reason(reason) {}
	EReason Reason() const { return m_reason; }
private:
	EReason m_reason;
};

class CError_FileWrite{};	//!< 例外：ファイル書き込み失敗
class CError_FileRead{};	//!< 例外：ファイル読み込み失敗

//ストリーム基底クラス
class CStream{
	using Me = CStream;

public:
	//コンストラクタ・デストラクタ
	CStream(const WCHAR* pszPath, const WCHAR* pszMode, bool bExceptionMode = false);
	CStream(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CStream(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CStream();

	//演算子
	operator bool() const{ return Good(); }

	//オープン・クローズ
	void Open(const WCHAR* pszPath, const WCHAR* pszMode);
	void Close();

	//操作
	void SeekSet(	//!< シーク
		long offset	//!< ストリーム先頭からのオフセット 
	);
	void SeekEnd(   //!< シーク
		long offset //!< ストリーム終端からのオフセット
	);

	//状態
	virtual bool Good() const{ return m_fp!=NULL && !Eof(); }
	bool Eof() const{ return m_fp==NULL || feof(m_fp); }

	//ファイルハンドル
	FILE* GetFp() const{ return m_fp; }

	//モード
	bool IsExceptionMode() const{ return m_bExceptionMode; }
private:
	FILE*			m_fp;
	CFileAttribute*	m_pcFileAttribute;
	bool			m_bExceptionMode;
};

class COutputStream : public CStream{
public:
	COutputStream(const WCHAR* pszPath, const WCHAR* pszMode, bool bExceptionMode = false)
	: CStream(pszPath, pszMode, bExceptionMode)
	{
	}

	//! データを無変換で書き込む。戻り値は書き込んだバイト数。
	int Write(const void* pBuffer, size_t nSizeInBytes)
	{
		size_t nRet = ::fwrite(pBuffer, 1, nSizeInBytes, GetFp());
		if(nRet!=nSizeInBytes && IsExceptionMode())throw CError_FileWrite();
		return static_cast<int>(nRet);
	}
};
#endif /* SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_ */
