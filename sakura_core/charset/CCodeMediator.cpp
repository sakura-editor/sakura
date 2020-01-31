/*! @file */
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
ECodeType CCodeMediator::CheckKanjiCode(const char *buff, size_t size) noexcept
{
    // 0バイトならタイプ別のデフォルト設定
    if (size == 0)
    {
        return m_sEncodingConfig.m_eDefaultCodetype;
    }

    // ICU4CのDLL群が利用できる場合、ICU4Cによる判定を試みる
    CharsetDetector csd;
    if (csd.IsAvailable())
    {
        auto code = csd.Detect(std::string_view(buff, size));
        if (code != CODE_ERROR)
            return code;
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
ECodeType CCodeMediator::CheckKanjiCodeOfFile(const WCHAR *pszFile)
{
    if (!pszFile)
    {
        return CODE_ERROR;
    }

    // オープン
    CBinaryInputStream in(pszFile);
    if (!in)
    {
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
