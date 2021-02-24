/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "charset/CCodeMediator.h"
#include "charset/icu4c/CharsetDetector.h"
#include "charset/CESI.h"
#include "io/CBinaryStream.h"

/*
	日本語コードセット判別

	戻り値】2007.08.14 kobake 戻り値をintからECodeTypeへ変更
	SJIS		CODE_SJIS
	JIS			CODE_JIS
	EUC			CODE_EUC
	Unicode		CODE_UNICODE
	UTF-8		CODE_UTF8
	UTF-7		CODE_UTF7
	UnicodeBE	CODE_UNICODEBE
*/
ECodeType CCodeMediator::CheckKanjiCode(const char* buff, size_t size) noexcept
{
	// 0バイトならタイプ別のデフォルト設定
	if (size == 0) {
		return m_sEncodingConfig.m_eDefaultCodetype;
	}

	// ICU4CのDLL群が利用できる場合、ICU4Cによる判定を試みる
	CharsetDetector csd;
	if (csd.IsAvailable()) {
		auto code = csd.Detect(std::string_view(buff, size));
		if (code != CODE_ERROR) return code;
	}

	CESI cesi(m_sEncodingConfig);
	return cesi.CheckKanjiCode(buff, size);
}

/*
|| ファイルの日本語コードセット判別
||
|| 【戻り値】2007.08.14 kobake 戻り値をintからECodeTypeへ変更
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
||	エラー		CODE_ERROR
*/
ECodeType CCodeMediator::CheckKanjiCodeOfFile(const WCHAR* pszFile)
{
	if (!pszFile) {
		return CODE_ERROR;
	}

	// オープン
	CBinaryInputStream in(pszFile);
	if(!in){
		return CODE_ERROR;
	}

	// データ長取得
	auto size = std::min<size_t>(in.GetLength(), CheckKanjiCode_MAXREADLENGTH);

	std::unique_ptr<char[]> buff;
	if (size > 0)
	{
		// データ確保
		buff = std::make_unique<char[]>(size);

		// 読み込み
		auto ret = in.Read(buff.get(), size);
	}

	// クローズ
	in.Close();

	// 日本語コードセット判別
	return CheckKanjiCode(buff.get(), size);
}
