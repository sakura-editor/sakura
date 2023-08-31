/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_
#define SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_
#pragma once

#include "apiwrap/CClipboardApi.hpp"

#include "mem/CNativeW.h"

#include <shellapi.h>

class CEol;

/*!
 * クリップボード所有型のスマートポインタを実現するためのdeleterクラス
 */
struct ClipboardCloser : private User32DllClient
{
	explicit ClipboardCloser(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>()) noexcept
		: User32DllClient(std::move(User32Dll_))
	{
	}

	void operator()(HWND) const
	{
		CloseClipboard();
	}

	BOOL CloseClipboard() const
	{
		return GetUser32Dll()->CloseClipboard();
	}
};

//! クリップボード所有型のスマートポインタ
using ClipboardHolder = std::unique_ptr<std::remove_pointer_t<HWND>, ClipboardCloser>;

/*!
 * サクラエディタ用クリップボードクラス
 *
 * 当初は、「後々はこの中で全てのクリップボードAPIを呼ばせた」かったらしい。
 * エディタとスクリプト関数の仕様がごちゃまぜになったスパゲッティコードになっている。
 * （仕様の分離を試みたが無理だった。）
 */
class CClipboard : public apiwrap::CClipboardApi, private ShareDataAccessorClient
{
private:
	ClipboardHolder m_bOpenResult;

	using Me = CClipboard;

public:
	static constexpr int   MAX_RETRY_FOR_OPEN = 2;
	static constexpr UINT  CF_ANY             = -1;
	static constexpr auto& CFN_HTML_FORMAT_   = L"HTML Format";
	static constexpr auto& CFN_SAKURA_CLIP2   = L"SAKURAClipW";
	static constexpr auto& CFN_MSDEV_COLUMN   = L"MSDEVColumnSelect";
	static constexpr auto& CFN_MSDEV_LINE01   = L"MSDEVLineSelect";
	static constexpr auto& CFN_MSDEV_LINE02   = L"VisualStudioEditorOperationsLineCutCopyClipboardTag";

	/*!
	 * スクリプト関数のモード値
	 */
	enum EModeConstants {
		/*!
		 * サクラエディタと同じ
		 */
		MODE_SAKURA = -2,

		/*!
		 * バイナリモード
		 *
		 * [U+00, U+ff] を [0x00, 0xff] にマッピングする
		 */
		MODE_BINARY = -1,
	};

	//コンストラクタ・デストラクタ
	explicit CClipboard(HWND hWnd, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>(), std::shared_ptr<Kernel32Dll> Kernel32Dll_ = std::make_shared<Kernel32Dll>(), std::shared_ptr<Shell32Dll> Shell32Dll_ = std::make_shared<Shell32Dll>(), std::shared_ptr<ShareDataAccessor> ShareDataAccessor_ = std::make_shared<ShareDataAccessor>()); //!< コンストラクタ内でクリップボードが開かれる
	CClipboard(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	static bool HasValidData(std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());    //!< クリップボード内に、サクラエディタで扱えるデータがあればtrue
	static CLIPFORMAT GetSakuraFormat(std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>()); //!< サクラエディタ独自のクリップボードデータ形式

	//インターフェース
	void Empty() const; //!< クリップボードを空にする
	void Close(); //!< クリップボードを閉じる
	bool SetText(const CStringRef& cstr, bool bColumnSelect, bool bLineSelect) const;
	bool SetHtmlText(const CNativeW& cmemBUf) const;
	bool GetText(CNativeW& cmemBuf, _Out_opt_ bool* pbColumnSelect = nullptr, _Out_opt_ bool* pbLineSelect = nullptr) const; //!< テキストを取得する
	bool GetDropFiles(CNativeW& cmemBuf, const CEol& cEol) const;
	bool IsIncludeClipboardFormat(std::wstring_view name) const;
	bool SetClipboardByFormat(const CStringRef& cstr, std::wstring_view name, int nMode, int nEndMode) const;
	bool GetClipboardByFormat(CNativeW& mem, std::wstring_view name, int nMode, int nEndMode, const CEol& cEol) const;

	CLIPFORMAT GetClipFormat(std::wstring_view formatName) const;
	int GetDataType() const; //!< クリップボードデータ形式(CF_UNICODETEXT等)の取得

	//演算子
	operator bool() const{ return m_bOpenResult!=FALSE; } //!< クリップボードを開けたならtrue

	/*!
	 * SetText既存コード互換バージョン
	 *
	 * サクラエディタ内からは指定しない引数 uFormat がある。
	 */
	bool SetText(
		const wchar_t* pData,                 //!< コピーするUNICODE文字列
		int            nDataLen,              //!< pDataの長さ（文字単位）
		bool           bColumnSelect = false,
		bool           bLineSelect   = false,
		UINT           uFormat       = CF_ANY) const
	{
		// 既存コードで-1以外を指定されるパスはない
		UNREFERENCED_PARAMETER(uFormat);

		return SetText({ pData, static_cast<size_t>(nDataLen) }, bColumnSelect, bLineSelect);
	}

	/*!
	 * GetText既存コード互換バージョン
	 *
	 * サクラエディタ内からは指定しない引数 uGetFormat がある。
	 */
	bool GetText(CNativeW* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat = CF_ANY) const
	{
		// 既存コードで-1以外を指定されるパスはない
		UNREFERENCED_PARAMETER(uGetFormat);

		if (GetText(*cmemBuf, pbColumnSelect, pbLineSelect))
		{
			return true;
		}

		if (GetDropFiles(*cmemBuf, cEol))
		{
			return true;
		}

		return false;
	}

	//! IsIncludeClipboardFormatの誤記バージョン
	bool IsIncludeClipboradFormat(const wchar_t* pFormatName) const {
		return IsIncludeClipboardFormat(pFormatName);
	}

	//! SetClipboardByFormatの誤記バージョン
	bool SetClipboradByFormat(const CStringRef& cstr, const wchar_t* pFormatName, int nMode, int nEndMode) const {
		return SetClipboardByFormat(cstr, pFormatName, nMode, nEndMode);
	}

	//! GetClipboradByFormatの誤記バージョン
	bool GetClipboradByFormat(CNativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const CEol& cEol) const {
		return GetClipboardByFormat(mem, pFormatName, nMode, nEndMode, cEol);
	}
};

#endif /* SAKURA_CCLIPBOARD_4E783022_214C_4E51_A2E0_54EC343500F6_H_ */
