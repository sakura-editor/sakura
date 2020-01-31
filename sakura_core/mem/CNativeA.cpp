/*! @file */
#include "StdAfx.h"
#include "CNativeA.h"

CNativeA::CNativeA() noexcept
    : CNative()
{
}

CNativeA::CNativeA(const CNativeA &rhs)
    : CNative(rhs)
{
}

CNativeA::CNativeA(CNativeA &&other) noexcept
    : CNative(std::forward<CNativeA>(other))
{
}

CNativeA::CNativeA(const char *szData, size_t cchData)
    : CNative()
{
    SetString(szData, cchData);
}

CNativeA::CNativeA(const char *szData)
    : CNative()
{
    SetString(szData);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeA::SetString(const char *pszData)
{
    SetString(pszData, strlen(pszData));
}

// バッファの内容を置き換える。nLenは文字単位。
void CNativeA::SetString(const char *pData, int nDataLen)
{
    int nDataLenBytes = nDataLen * sizeof(char);
    CNative::SetRawData(pData, nDataLenBytes);
}

// バッファの内容を置き換える
void CNativeA::SetNativeData(const CNativeA &pcNative)
{
    CNative::SetRawData(pcNative);
}

// バッファの最後にデータを追加する
void CNativeA::AppendString(const char *pszData)
{
    AppendString(pszData, strlen(pszData));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeA::AppendString(const char *pszData, int nLength)
{
    CNative::AppendRawData(pszData, nLength * sizeof(char));
}

//! バッファの最後にデータを追加する (フォーマット機能付き)
void CNativeA::AppendStringF(const char *pszData, ...)
{
    char buf[2048];

    // 整形
    va_list v;
    va_start(v, pszData);
    int len = _vsnprintf_s(buf, _countof(buf), _TRUNCATE, pszData, v);
    int e   = errno;
    va_end(v);

    if (len == -1)
    {
        DEBUG_TRACE(L"AppendStringF error. errno = %d", e);
        throw std::exception();
    }

    // 追加
    this->AppendString(buf, len);
}

const CNativeA &CNativeA::operator=(char cChar)
{
    char pszChar[2];
    pszChar[0] = cChar;
    pszChar[1] = '\0';
    SetRawData(pszChar, 1);
    return *this;
}

//! バッファの最後にデータを追加する
void CNativeA::AppendNativeData(const CNativeA &pcNative)
{
    AppendString(pcNative.GetStringPtr(), pcNative.GetStringLength());
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeA::AllocStringBuffer(int nDataLen)
{
    CNative::AllocBuffer(nDataLen * sizeof(char));
}

const CNativeA &CNativeA::operator+=(char ch)
{
    char szChar[2] = {ch, '\0'};
    AppendString(szChar);
    return *this;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CNativeA::GetStringLength() const
{
    return CNative::GetRawLength() / sizeof(char);
}

// 任意位置の文字取得。nIndexは文字単位。
char CNativeA::operator[](int nIndex) const
{
    if (nIndex < GetStringLength())
    {
        return GetStringPtr()[nIndex];
    }
    else
    {
        return 0;
    }
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 文字列置換（日本語考慮版） */
void CNativeA::Replace_j(const char *pszFrom, const char *pszTo)
{
    CNativeA cmemWork;
    int nFromLen = strlen(pszFrom);
    int nToLen   = strlen(pszTo);
    int nBgnOld  = 0;
    int nBgn     = 0;
    while (nBgn <= GetStringLength() - nFromLen)
    {
        if (0 == memcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen))
        {
            if (0 < nBgn - nBgnOld)
            {
                cmemWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
            }
            cmemWork.AppendString(pszTo, nToLen);
            nBgn    = nBgn + nFromLen;
            nBgnOld = nBgn;
        }
        else
        {
            if (_IS_SJIS_1((unsigned char)GetStringPtr()[nBgn]))
                nBgn++;
            nBgn++;
        }
    }
    if (0 < GetStringLength() - nBgnOld)
    {
        cmemWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
    }
    SetNativeData(cmemWork);
    return;
}
