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
#include "StdAfx.h"
#include "CClipboard.h"

#include "doc/CEditDoc.h"
#include "charset/CCodeMediator.h"
#include "charset/CCodeFactory.h"
#include "charset/CUtf8.h"
#include "CEol.h"
#include "mem/CNativeA.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CClipboard::CClipboard(HWND hWnd, std::shared_ptr<User32Dll> User32Dll_, std::shared_ptr<Kernel32Dll> Kernel32Dll_, std::shared_ptr<Shell32Dll> Shell32Dll_, std::shared_ptr<ShareDataAccessor> ShareDataAccessor_)
	: CClipboardApi(std::move(User32Dll_), std::move(Kernel32Dll_), std::move(Shell32Dll_))
	, ShareDataAccessorClient(std::move(ShareDataAccessor_))
	, m_hwnd(hWnd)
	, m_bOpenResult(OpenClipboard(hWnd, MAX_RETRY_FOR_OPEN))
{
}

CClipboard::~CClipboard()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CClipboard::Empty() const
{
	if (m_bOpenResult)
	{
		EmptyClipboard();
	}
}

void CClipboard::Close()
{
	if (m_bOpenResult)
	{
		CloseClipboard();

		m_bOpenResult = FALSE;
	}
}

bool CClipboard::SetText(
	const CStringRef& cstr,
	bool			  bColumnSelect,
	bool			  bLineSelect) const
{
	if (!m_bOpenResult)
	{
		return false;
	}

	// UNICODE形式のデータ (CF_UNICODETEXT)
	if (!SetClipboardData(CF_UNICODETEXT, cstr.GetPtr(), cstr.GetLength(), cstr.GetLength() + 1))
	{
		return false;
	}

	// サクラエディタ専用フォーマット
	// バイナリ形式のデータ
	//	(int) 「データ」の長さ
	//	「データ」
	if (const auto uClipFormat = GetClipFormat(CFN_SAKURA_CLIP2))
	{
		int cchClipSakura = cstr.GetLength();
		size_t cbClipSakura = sizeof(int) + (cstr.GetLength() + 1) * sizeof(wchar_t);
		BinarySequence bin(cbClipSakura, std::byte());
		bin.assign(std::bit_cast<const std::byte*>(&cchClipSakura), sizeof(int));
		bin.append(std::bit_cast<const std::byte*>(cstr.GetPtr()), cchClipSakura * sizeof(wchar_t));
		bin.resize(cbClipSakura);
		if (!SetClipboardData(uClipFormat, bin.data(), bin.length(), bin.length()))
		{
			return false;
		}
	}

	// 矩形選択を示すダミーデータ
	if (bColumnSelect)
	{
		if (const auto uClipFormat = GetClipFormat(CFN_MSDEV_COLUMN))
		{
			const auto msDevColumn = std::byte(0);
			if (!SetClipboardData(uClipFormat, &msDevColumn, 1, 1))
			{
				return false;
			}
		}
	}

	/* 行選択を示すダミーデータ */
	if (bLineSelect)
	{
		if (const auto uClipFormat = GetClipFormat(CFN_MSDEV_LINE01))
		{
			const auto msDevLine = std::byte(1);
			if (!SetClipboardData(uClipFormat, &msDevLine, 1, 1))
			{
				return false;
			}
		}

		if (const auto uClipFormat = GetClipFormat(CFN_MSDEV_LINE02))
		{
			const auto msDevLine2 = std::byte(1); // ※ ClipSpy で調べるとデータはこれとは違うが内容には無関係に動くっぽい
			if (!SetClipboardData(uClipFormat, &msDevLine2, 1, 1))
			{
				return false;
			}
		}
	}
	return true;
}

bool CClipboard::SetHtmlText(const CNativeW& cmemBUf) const
{
	if( !m_bOpenResult ){
		return false;
	}

	CNativeA cmemUtf8;
	CUtf8().UnicodeToCode(cmemBUf, cmemUtf8._GetMemory());

	CNativeA cmemHeader;
	size_t size = cmemUtf8.GetStringLength() + 134;
	cmemHeader.AppendString("Version:0.9\r\n");
	cmemHeader.AppendString("StartHTML:00000097\r\n");
	cmemHeader.AppendStringF("EndHTML:%08Id\r\n", size + 36);
	cmemHeader.AppendString("StartFragment:00000134\r\n");
	cmemHeader.AppendStringF("EndFragment:%08Id\r\n", size);
	cmemHeader.AppendString("<html><body>\r\n<!--StartFragment -->\r\n");
	CNativeA cmemFooter;
	cmemFooter.AppendString("\r\n<!--EndFragment-->\r\n</body></html>\r\n");

	BinarySequence text;
	text.append(std::bit_cast<std::byte*>(cmemHeader.GetStringPtr()), cmemHeader.GetStringLength());
	text.append(std::bit_cast<std::byte*>(cmemUtf8.GetStringPtr()), cmemUtf8.GetStringLength());
	text.append(std::bit_cast<std::byte*>(cmemFooter.GetStringPtr()), cmemFooter.GetStringLength());

	//クリップボードに設定
	const auto uFormat = GetClipboardFormatW(CFN_HTML_FORMAT_, GetUser32Dll());
	return SetClipboardData(uFormat, text.data(), text.size(), text.size() + 1);
}

/*!
 * テキストを取得する
 *
 * @param [out] cmemBuf 取得したテキストの格納先
 * @param [in,out] pbColumnSelect 矩形選択形式
 * @param [in,out] pbLineSelect 行選択形式
 */
bool CClipboard::GetText(
	CNativeW&       cmemBuf,
	_Out_opt_ bool* pbColumnSelect,
	_Out_opt_ bool* pbLineSelect) const
{
	cmemBuf.SetString(L"");

	if (pbColumnSelect)
	{
		*pbColumnSelect = false;
	}

	if (pbLineSelect)
	{
		*pbLineSelect = false;
	}

	if (!m_bOpenResult)
	{
		return false;
	}

	//矩形選択や行選択のデータがあれば取得
	bool done = false;
	UINT uFormat = 0;
	do
	{
		uFormat = EnumClipboardFormats(uFormat);

		if (pbColumnSelect && uFormat == GetClipFormat(CFN_MSDEV_COLUMN)) {
			done = *pbColumnSelect = true;
		}
		if (pbLineSelect && (uFormat == GetClipFormat(CFN_MSDEV_LINE01) || uFormat == GetClipFormat(CFN_MSDEV_LINE02))) {
			done = *pbLineSelect = true;
		}
	}
	while (!done && uFormat);

	//サクラ形式のデータがあれば取得
	if (const auto uClipFormat = GetClipFormat(CFN_SAKURA_CLIP2);
		IsClipboardFormatAvailable(uClipFormat))
	{
		if (BinarySequence buffer;
			GetClipboardString(uClipFormat, buffer))
		{
			const auto nLength = *std::bit_cast<int*>(buffer.data());
			cmemBuf.SetString(std::bit_cast<const wchar_t*>(&buffer[sizeof(int)]), nLength);
			return true;
		}
	}

	//UNICODE形式のデータがあれば取得
	// From Here 2005/05/29 novice UNICODE TEXT 対応処理を追加
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (std::wstring buffer;
			GetClipboardString(CF_UNICODETEXT, buffer))
		{
			if (const auto length = ::wcsnlen_s(buffer.data(), buffer.length());
				length < buffer.length())
			{
				buffer.resize(length);
			}
			cmemBuf.SetString(buffer.c_str(), buffer.length());
			return true;
		}
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT形式のデータがあれば取得
	if (IsClipboardFormatAvailable(CF_OEMTEXT))
	{
		if (BinarySequence buffer;
			GetClipboardString(CF_OEMTEXT, buffer))
		{
			if (const auto length = ::strnlen_s(std::bit_cast<const char*>(buffer.data()), buffer.length());
				length < buffer.length())
			{
				buffer.resize(length);
			}
			//SJIS→UNICODE
			auto converter = CCodeFactory::CreateCodeBase(ECodeType::CODE_SJIS);
			cmemBuf = converter->CodeToUnicode(BinarySequenceView(buffer.data(), buffer.length()));
			return true;
		}
	}

	return false;
}

/*!
 * ドロップされたファイルを取得します。
 *
 * @date 2008/09/10 bosagami パス貼り付け対応
 * @date 2012/10/05 Moca ANSI版に合わせて最終行にも改行コードをつける
 */
bool CClipboard::GetDropFiles(
	CNativeW&   cmemBuf,
	const CEol& cEol) const
{
	cmemBuf.SetString(L"");

	if (!m_bOpenResult)
	{
		return false;
	}

	//HDROP形式のデータがあれば取得
	if (IsClipboardFormatAvailable(CF_HDROP))
	{
		HDROP hDrop   = NULL;
		UINT  nMaxCnt = 0;

		if (BinarySequence buffer;
			GetClipboardString(CF_HDROP, buffer))
		{
			hDrop   = std::bit_cast<HDROP>(buffer.data());
			nMaxCnt = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
		}

		std::array<WCHAR, _MAX_PATH + 1> path = {};
		for (UINT nLoop = 0; hDrop && nLoop < nMaxCnt; nLoop++) {
			DragQueryFileW(hDrop, nLoop, path.data(), static_cast<int>(path.size()));
			cmemBuf.AppendString(path.data());
			if (nMaxCnt > 1) {
				cmemBuf.AppendString(cEol.GetValue2());
			}
		}

		return true;
	}

	return false;
}

CLIPFORMAT CClipboard::GetClipFormat(std::wstring_view name) const
{
	if (name.length() > 0 && std::ranges::all_of(name, WCODE::Is09))
	{
		return static_cast<CLIPFORMAT>(_wtoi(name.data()));
	}

	return static_cast<CLIPFORMAT>(GetClipboardFormatW(name, GetUser32Dll()));
}

bool CClipboard::IsIncludeClipboardFormat(std::wstring_view name) const
{
	if (const auto uFormat = GetClipFormat(name);
		uFormat != CF_INVALID)
	{
		return IsClipboardFormatAvailable(uFormat);
	}
	return false;
}

static int GetEndModeByMode(int nMode, int nEndMode)
{
	if( nEndMode == -1 ){
		switch(nMode){
			case -1: nEndMode = 0; break;
			case CODE_AUTODETECT: nEndMode = 0; break;
			case CODE_SJIS: nEndMode = 1; break;
			case CODE_EUC: nEndMode = 1; break;
			case CODE_LATIN1: nEndMode = 1; break;
			case CODE_UNICODE: nEndMode = 2; break;
			case CODE_UNICODEBE: nEndMode = 2; break;
			case CODE_UTF8: nEndMode = 1; break;
			case CODE_CESU8: nEndMode = 1; break;
			case CODE_UTF7: nEndMode = 1; break;
			default: nEndMode = 0; break;
		}
	}
	return nEndMode;
}

/*!
 * 指定のクリップボード形式で設定
 * (スクリプト用関数)
 *
 * @param nMode -2:通常のサクラの処理, -1:バイナリモード, それ以外:文字コード
 * @param nEndMode -1:文字コードに依存 0:GlobalSize 1:strlen 2:wcslen 4:wchar32_tの文字列
 */
bool CClipboard::SetClipboardByFormat(const CStringRef& cstr, std::wstring_view name, int nMode, int nEndMode) const
{
	if (nMode == MODE_SAKURA) {
		return SetText(cstr, false, false);
	}

	const auto uFormat = GetClipFormat(name);
	if( uFormat == CF_INVALID ){
		return false;
	}
	CMemory cmemBuf;
	char* pBuf = NULL;
	size_t nTextByteLen = 0;
	if( nMode == MODE_BINARY ){
		cmemBuf.AllocBuffer(cstr.GetLength());
		cmemBuf._SetRawLength(cstr.GetLength());
		pBuf = std::bit_cast<char*>(cmemBuf.GetRawPtr());
		const auto len = cstr.GetLength();
		const auto mem = cstr.GetPtr();
		for(size_t i = 0; i < len; i++){
			pBuf[i] = static_cast<uint8_t>(mem[i]);
			if( 0xff < mem[i] ){
				return false;
			}
		}
		nTextByteLen = len;
 	}else{
		const auto eMode = static_cast<ECodeType>(nMode);
		if( !IsValidCodeType(eMode) ){
			return false;
		}
		if (eMode == CODE_UNICODE) {
			pBuf = std::bit_cast<char*>(cstr.GetPtr());
			nTextByteLen = cstr.GetLength() * sizeof(wchar_t);
		}
		else {
			if (auto converter = std::unique_ptr<CCodeBase>(CCodeFactory::CreateCodeBase(eMode, GetShareData()->m_Common.m_sFile.GetAutoMIMEdecode()));
				RESULT_FAILURE == converter->UnicodeToCode(cstr, &cmemBuf)) {
				return false;
			}
			pBuf = std::bit_cast<char*>(cmemBuf.GetRawPtr());
			nTextByteLen = cmemBuf.GetRawLength();
		}
	}
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nulLen = 0;
	switch( nEndMode ){
	case 1: nulLen = 1; break;
	case 2: nulLen = 2; break;
	case 4: nulLen = 4; break;
	case 0: nulLen = 0; break;
	default: nulLen = 0; break;
	}

	return SetClipboardData(uFormat, std::bit_cast<const std::byte*>(pBuf), nTextByteLen, nTextByteLen + nulLen);
}

static size_t GetLengthByMode(BinarySequenceView data, int nMode, int nEndMode)
{
	const auto pData  = data.data();
	const auto cbData = data.size();
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t cbLength = cbData;
	if( nEndMode == 1 ) {
		cbLength = strnlen(std::bit_cast<const char*>(pData), cbData);
	}else if( nEndMode == 2 ){
		cbLength = wcsnlen(std::bit_cast<const wchar_t*>(pData), cbData / 2) * 2;
	}else if( nEndMode == 4 ){
		const auto pData32   = std::bit_cast<const char32_t*>(pData);
		const auto cchData32 = cbData / 4;
		for (cbLength = 0; cbLength < cchData32 && pData32[cbLength]; ++cbLength);
		cbLength *= 4;
	}
	return cbLength;
}

/*!
	指定のクリップボード形式で取得
	
	@param nMode -2:通常のサクラの処理, -1:バイナリモード, それ以外:文字コード
	@param nEndMode -1:文字コードに依存 0:GlobalSize 1:strlen 2:wcslen 4:wchar32_tの文字列
	@date 2013.06.12 Moca 新規作成
 */
bool CClipboard::GetClipboardByFormat(CNativeW& mem, std::wstring_view name, int nMode, int nEndMode, const CEol& cEol) const
{
	mem.SetString(L"");

	if( nMode == MODE_SAKURA ){
		return GetText(&mem, nullptr, nullptr, cEol, CF_ANY);
	}

	const auto uFormat = GetClipFormat(name);
	if( uFormat == CF_INVALID ){
		return false;
	}
	if( !IsClipboardFormatAvailable(uFormat) ){
		return false;
	}

	BinarySequence data;
	if (!GetClipboardString(uFormat, data))
	{
		return false;
	}

	// 長さオプションの解釈
	size_t nLength = GetLengthByMode(data, nMode, nEndMode);

	// バイナリモード。1byteをU+00-U+ffにマッピング
	if (nMode == MODE_BINARY)
	{
		data.resize(nLength);
		mem.AllocStringBuffer(nLength);
		mem._SetStringLength(nLength);
		auto pBuf = mem.GetStringPtr();
		for( size_t i = 0; i < data.length(); ++i ){
			pBuf[i] = static_cast<wchar_t>(data[i]);
		}

		return true;
	}

	auto eMode = static_cast<ECodeType>(nMode);
	if (!IsValidCodeType(eMode)) {
		const auto& type = CEditDoc::getInstance()->m_cDocType.GetDocumentAttribute();
		CCodeMediator mediator(type.m_encoding);
		eMode = mediator.CheckKanjiCode(std::bit_cast<const char*>(data.data()), nLength);

		if (!IsValidCodeType(eMode)) {
			eMode = CODE_DEFAULT;
		}
		if (-1 == nEndMode) {
			// nLength 再設定
			nLength = GetLengthByMode(data, eMode, nEndMode);
		}
	}
	if (eMode == CODE_UNICODE) {
		mem.SetString((wchar_t*)data.data(), nLength / sizeof(wchar_t));
		return true;
	}

	auto converter = std::unique_ptr<CCodeBase>(CCodeFactory::CreateCodeBase(eMode, GetShareData()->m_Common.m_sFile.GetAutoMIMEdecode()));

	bool bResult = true;
	mem = converter->CodeToUnicode(data, &bResult);
	if (bResult) {
		return true;
	}

	mem.SetString(L"");
	return false;
}

//! クリップボード内に、サクラエディタで扱えるデータがあればtrue
bool CClipboard::HasValidData(std::shared_ptr<User32Dll> _User32Dll)
{
	//扱える形式が１つでもあればtrue
	if (const auto uFormat = GetSakuraFormat(_User32Dll);
		_User32Dll->IsClipboardFormatAvailable(uFormat))
	{
		return true;
	}
	if (_User32Dll->IsClipboardFormatAvailable(CF_UNICODETEXT))return true;
	if (_User32Dll->IsClipboardFormatAvailable(CF_OEMTEXT))return true;
	/* 2008.09.10 bosagami パス貼り付け対応 */
	if(_User32Dll->IsClipboardFormatAvailable(CF_HDROP))return true;
	return false;
}

//!< サクラエディタ独自のクリップボードデータ形式
CLIPFORMAT CClipboard::GetSakuraFormat(std::shared_ptr<User32Dll> _User32Dll)
{
	/*
		2007.09.30 kobake

		UNICODE形式でクリップボードデータを保持するよう変更したため、
		以前のバージョンのクリップボードデータと競合しないように
		フォーマット名を変更
	*/
	return static_cast<CLIPFORMAT>(GetClipboardFormatW(CFN_SAKURA_CLIP2, _User32Dll));
}

//!< クリップボードデータ形式(CF_UNICODETEXT等)の取得
int CClipboard::GetDataType() const
{
	//扱える形式が１つでもあればtrue
	// 2013.06.11 GetTextの取得順に変更
	if (const auto uFormat = GetSakuraFormat(GetUser32Dll());
		IsClipboardFormatAvailable(uFormat))
	{
		return uFormat;
	}
	if(IsClipboardFormatAvailable(CF_UNICODETEXT))return CF_UNICODETEXT;
	if(IsClipboardFormatAvailable(CF_OEMTEXT))return CF_OEMTEXT;
	if(IsClipboardFormatAvailable(CF_HDROP))return CF_HDROP;
	return -1;
}
