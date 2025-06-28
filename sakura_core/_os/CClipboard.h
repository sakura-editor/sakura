/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_
#define SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_
#pragma once

#include "mem/CNativeW.h"

class CEol;
class CStringRef;

struct IWBuffer {
	virtual void Clear() = 0;
	virtual void Reserve(size_t nDataLen) = 0;
	virtual size_t Capacity() const = 0;
	virtual void Append(const wchar_t* pszData, size_t nDataLen) = 0;
};

struct CNativeWBuffer : public IWBuffer {
	CNativeWBuffer(CNativeW* cmembuf) : cmembuf(cmembuf) {}
	void Clear() override { cmembuf->Clear(); }
	void Reserve(size_t nDataLen) override { cmembuf->AllocStringBuffer(nDataLen); }
	size_t Capacity() const override { return (size_t)cmembuf->capacity(); }
	void Append(const wchar_t* pszData, size_t nDataLen) override { cmembuf->AppendString(pszData, nDataLen); }
	CNativeW* cmembuf;
};

struct StdWStringBuffer : public IWBuffer {
	StdWStringBuffer(std::wstring* wstr) : wstr(wstr) {}
	void Clear() override { wstr->clear(); }
	void Reserve(size_t nDataLen) override {
		try {
			wstr->reserve(nDataLen);
		}
		catch (std::bad_alloc&) {
			;
		}
	}
	size_t Capacity() const override { return wstr->capacity(); }
	void Append(const wchar_t* pszData, size_t nDataLen) override { wstr->append(pszData, nDataLen); }
	std::wstring* wstr;
};

//!サクラエディタ用クリップボードクラス。後々はこの中で全てのクリップボードAPIを呼ばせたい。
class CClipboard{
	using Me = CClipboard;

public:
	//コンストラクタ・デストラクタ
	CClipboard(HWND hwnd); //!< コンストラクタ内でクリップボードが開かれる
	CClipboard(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CClipboard(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CClipboard(); //!< デストラクタ内でCloseが呼ばれる

	//インターフェース
	void Empty(); //!< クリップボードを空にする
	void Close(); //!< クリップボードを閉じる
	bool SetText(const wchar_t* pData, size_t nDataLen, bool bColumnSelect, bool bLineSelect, UINT uFormat = (UINT)-1);   //!< テキストを設定する
	bool SetHtmlText(const CNativeW& cmemBUf);
	bool GetText(IWBuffer* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat = (UINT)-1); //!< テキストを取得する
	bool GetText(CNativeW* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat = (UINT)-1); //!< テキストを取得する
	bool GetText(std::wstring* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat = (UINT)-1); //!< テキストを取得する
	bool IsIncludeClipboardFormat(const wchar_t* pFormatName);
	bool SetClipboardByFormat(const CStringRef& cstr, const wchar_t* pFormatName, int nMode, int nEndMode);
	bool GetClipboardByFormat(CNativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const CEol& cEol);

	//演算子
	operator bool() const{ return m_bOpenResult!=FALSE; } //!< クリップボードを開けたならtrue

	int GetDataType() const; //!< クリップボードデータ形式(CF_UNICODETEXT等)の取得

private:
	HWND m_hwnd;
	BOOL m_bOpenResult;

	// -- -- staticインターフェース -- -- //
public:
	static bool HasValidData();    //!< クリップボード内に、サクラエディタで扱えるデータがあればtrue
	static CLIPFORMAT GetSakuraFormat(); //!< サクラエディタ独自のクリップボードデータ形式

protected:
	// 単体テスト用コンストラクタ
	explicit CClipboard(bool openStatus) : m_bOpenResult(openStatus) {}

	// 同名の Windows API に引数を転送する仮想メンバ関数。
	// 単体テスト内でオーバーライドすることで副作用のないテストを実施するのが目的。
	virtual HANDLE SetClipboardData(UINT uFormat, HANDLE hMem) const;
	virtual HANDLE GetClipboardData(UINT uFormat) const;
	virtual BOOL EmptyClipboard() const;
	virtual BOOL IsClipboardFormatAvailable(UINT format) const;
	virtual UINT EnumClipboardFormats(UINT format) const;
	virtual HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) const;
	virtual LPVOID GlobalLock(HGLOBAL hMem) const;
};
#endif /* SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_ */
