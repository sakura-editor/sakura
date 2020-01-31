/*! @file */
#include "StdAfx.h"
#include <stdexcept>
#include "mem/CNativeW.h"
#include "CEol.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CNativeW::CNativeW() noexcept
    : CNative()
{
}

CNativeW::CNativeW(const CNativeW &rhs)
    : CNative(rhs)
{
}

CNativeW::CNativeW(CNativeW &&other) noexcept
    : CNative(std::forward<CNativeW>(other))
{
}

//! nDataLenは文字単位。
CNativeW::CNativeW(const wchar_t *pData, int nDataLen)
    : CNative()
{
    SetString(pData, nDataLen);
}

CNativeW::CNativeW(const wchar_t *pData)
    : CNative()
{
    SetString(pData);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeW::SetString(const wchar_t *pData, int nDataLen)
{
    CNative::SetRawData(pData, nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void CNativeW::SetString(const wchar_t *pszData)
{
    if (pszData)
        CNative::SetRawData(pszData, wcslen(pszData) * sizeof(wchar_t));
    else
        CMemory::Clear();
}

void CNativeW::SetStringHoldBuffer(const wchar_t *pData, int nDataLen)
{
    CNative::SetRawDataHoldBuffer(pData, nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void CNativeW::SetNativeData(const CNativeW &pcNative)
{
    CNative::SetRawData(pcNative);
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeW::AllocStringBuffer(int nDataLen)
{
    CNative::AllocBuffer(nDataLen * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する
void CNativeW::AppendString(const wchar_t *pszData)
{
    CNative::AppendRawData(pszData, wcslen(pszData) * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeW::AppendString(const wchar_t *pszData, int nLength)
{
    CNative::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

/*!
 * バッファの最後にデータを追加する (フォーマット機能付き)
 *
 * @param pszFormat フォーマット書式文字列
 * @param va_args C-style の可変長引数
 * @throws std::invalid_argument pszFormatがNULL
 * @throws std::bad_alloc メモリ確保に失敗
 * @remark 不正なフォーマットを指定すると無効なパラメータ例外で即死します。
 */
void CNativeW::AppendStringF(const wchar_t *pszFormat, ...)
{
    // _vscwprintf に NULL を渡してはならないので除外する
    if (!pszFormat)
    {
        throw std::invalid_argument("pszFormat can't be nullptr");
    }

    // 可変長引数のポインタを取得
    va_list v;
    va_start(v, pszFormat);

    // 整形によって追加される文字数をカウント
    const int additional = ::_vscwprintf(pszFormat, v);

    // 現在の文字列長を取得
    const auto currentLength = GetStringLength();

    // 現在の文字数 + 追加文字数が収まるようにバッファを拡張する
    const auto newCapacity = currentLength + additional;
    AllocStringBuffer(newCapacity);

    int added = 0;
    if (additional > 0)
    {
        // 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
        added = ::_vsnwprintf_s(&GetStringPtr()[currentLength], additional + 1, _TRUNCATE, pszFormat, v);
    }

    // 可変長引数のポインタを解放
    va_end(v);

    // 文字列終端を再設定する
    _SetStringLength(currentLength + added);
}

//! バッファの最後にデータを追加する
void CNativeW::AppendNativeData(const CNativeW &cmemData)
{
    CNative::AppendRawData(cmemData.GetStringPtr(), cmemData.GetRawLength());
}

/*!
 * 指定した文字列を連結した文字列バッファを作成する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字列ポインタ(C string)
 * @return 新しい文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW operator+(const CNativeW &lhs, const wchar_t *rhs) noexcept(false)
{
    CNativeW tmp(lhs);
    tmp.AppendString(rhs);
    return tmp;
}

/*!
 * 指定した文字列を連結した文字列バッファを作成する
 *
 * @param lhs 文字列ポインタ(C string)
 * @param rhs 文字列バッファ(CNativeW)
 * @return 新しい文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW operator+(const wchar_t *lhs, const CNativeW &rhs) noexcept(false)
{
    CNativeW tmp(lhs);
    return tmp + rhs;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()と同機能
wchar_t CNativeW::operator[](int nIndex) const
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

/*!
 * 同型との比較
 *
 * @param rhs 比較対象
 * @retval < 0 自身がメモリ未確保、かつ、比較対象はメモリ確保済み
 * @retval < 0 データ値が比較対象より小さい
 * @retval < 0 データが比較対象の先頭部分と一致する、かつ、データ長が比較対象より小さい
 * @retval == 0 比較対象が自分自身の参照
 * @retval == 0 比較対象と等しい
 * @retval == 0 自身がメモリ未確保、かつ、比較対象がメモリ未確保
 * @retval > 0 自身が確保済み、かつ、比較対象がメモリ未確保
 * @retval > 0 データ値が比較対象より大きい
 * @retval > 0 データの先頭部分が比較対象と一致する、かつ、データ長が比較対象より大きい
 */
int CNativeW::Compare(const CNativeW &rhs) const noexcept
{
    if (this == &rhs)
        return 0;
    const int lhsIsValid = static_cast<int>(IsValid());
    const int rhsIsValid = static_cast<int>(rhs.IsValid());
    if (!rhsIsValid || !lhsIsValid)
        return lhsIsValid - rhsIsValid;
    // データ長が短い方を基準に比較を行う
    const int lhsLength = static_cast<int>(GetStringLength());
    const int rhsLength = static_cast<int>(rhs.GetStringLength());
    const int minLength = std::min(lhsLength, rhsLength);
    // データ長の範囲で文字列を比較する
    auto cmp = wmemcmp(GetStringPtr(), rhs.GetStringPtr(), minLength);
    if (!cmp)
        cmp = lhsLength - rhsLength;
    return cmp;
}

/*!
 * 文字列ポインタ型との比較
 *
 * @param rhs 比較対象(C string)
 * @retval < 0 自身がメモリ未確保、かつ、比較対象がnullptr以外
 * @retval < 0 文字列値が比較対象より小さい
 * @retval == 0 比較対象と等しい
 * @retval == 0 自身がメモリ未確保、かつ、比較対象がnullptr
 * @retval > 0 自身がメモリ確保済み、かつ、比較対象がnullptr
 * @retval > 0 文字列値が比較対象より大きい
 */
int CNativeW::Compare(const wchar_t *rhs) const noexcept
{
    const int lhsIsValid = static_cast<int>(IsValid());
    const int rhsIsValid = rhs ? 1 : 0;
    if (!rhsIsValid || !lhsIsValid)
        return lhsIsValid - rhsIsValid;
    const wchar_t *lhs  = GetStringPtr();
    const int lhsLength = GetStringLength();
    // NUL終端考慮のために終端を拡張し、比較自体はCRTに丸投げする
    return wcsncmp(lhs, rhs, lhsLength + 1);
}

/* 等しい内容か */
bool CNativeW::IsEqual(const CNativeW &cmem1, const CNativeW &cmem2)
{
    if (&cmem1 == &cmem2)
        return true;

    const int nLen1 = cmem1.GetStringLength();
    const int nLen2 = cmem2.GetStringLength();
    if (nLen1 == nLen2)
    {
        const wchar_t *psz1 = cmem1.GetStringPtr();
        const wchar_t *psz2 = cmem2.GetStringPtr();
        if (0 == wmemcmp(psz1, psz2, nLen1))
        {
            return true;
        }
    }
    return false;
}

/*!
 * 文字列ポインタ型との等価比較
 *
 * @param lhs 比較対象(CNativeW)
 * @param rhs 比較対象(C string)
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator==(const CNativeW &lhs, const wchar_t *rhs) noexcept
{
    return lhs.Equals(rhs);
}

/*!
 * 文字列ポインタ型との否定の等価比較
 *
 * @param lhs 比較対象(CNativeW)
 * @param rhs 比較対象(C string)
 * @retval true 等しくない
 * @retval false 等しい
 */
bool operator!=(const CNativeW &lhs, const wchar_t *rhs) noexcept
{
    return !(lhs == rhs);
}

/*!
 * 文字列ポインタ型との等価比較(引数逆転版)
 *
 * @param lhs 比較対象(C string)
 * @param rhs 比較対象(CNativeW)
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator==(const wchar_t *lhs, const CNativeW &rhs) noexcept
{
    return rhs.Equals(lhs);
}

/*!
 * 文字列ポインタ型との否定の等価比較(引数逆転版)
 *
 * @param lhs 比較対象(C string)
 * @param rhs 比較対象(CNativeW)
 * @retval true 等しくない
 * @retval false 等しい
 */
bool operator!=(const wchar_t *lhs, const CNativeW &rhs) noexcept
{
    return !(lhs == rhs);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 文字列置換
void CNativeW::Replace(const wchar_t *pszFrom, const wchar_t *pszTo)
{
    int nFromLen = wcslen(pszFrom);
    int nToLen   = wcslen(pszTo);
    Replace(pszFrom, nFromLen, pszTo, nToLen);
}

void CNativeW::Replace(const wchar_t *pszFrom, int nFromLen, const wchar_t *pszTo, int nToLen)
{
    CNativeW cmemWork;
    int nBgnOld = 0;
    int nBgn    = 0;
    while (nBgn <= GetStringLength() - nFromLen)
    {
        if (0 == wmemcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen))
        {
            if (nBgnOld == 0 && nFromLen <= nToLen)
            {
                cmemWork.AllocStringBuffer(GetStringLength());
            }
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
            nBgn++;
        }
    }
    if (nBgnOld != 0)
    {
        if (0 < GetStringLength() - nBgnOld)
        {
            cmemWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
        }
        SetNativeData(cmemWork);
    }
    else
    {
        if (this->GetStringPtr() == NULL)
        {
            this->SetString(L"");
        }
    }
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字がwchar_t何個分かを返す
CLogicInt CNativeW::GetSizeOfChar(const wchar_t *pData, int nDataLen, int nIdx)
{
    if (nIdx >= nDataLen)
        return CLogicInt(0);

    // サロゲートチェック					2008/7/5 Uchi
    if (IsUTF16High(pData[nIdx]))
    {
        if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1]))
        {
            // サロゲートペア 2個分
            return CLogicInt(2);
        }
    }

    return CLogicInt(1);
}

//! 指定した位置の文字が半角何個分かを返す
CKetaXInt CNativeW::GetKetaOfChar(const wchar_t *pData, int nDataLen, int nIdx)
{
    //文字列範囲外なら 0
    if (nIdx >= nDataLen)
        return CKetaXInt(0);

    // サロゲートチェック BMP 以外は全角扱い		2008/7/5 Uchi
    if (IsUTF16High(pData[nIdx]))
    {
        return CKetaXInt(2); // 仮
    }
    if (IsUTF16Low(pData[nIdx]))
    {
        if (nIdx > 0 && IsUTF16High(pData[nIdx - 1]))
        {
            // サロゲートペア（下位）
            return CKetaXInt(0);
        }
        // 単独（ブロークンペア）
        // return CKetaXInt(2);
        if (IsBinaryOnSurrogate(pData[nIdx]))
            return CKetaXInt(1);
        else
            return CKetaXInt(2);
    }

    //半角文字なら 1
    if (WCODE::IsHankaku(pData[nIdx]))
        return CKetaXInt(1);

    //全角文字なら 2
    else
        return CKetaXInt(2);
}

//! 指定した位置の文字の文字幅を返す
CHabaXInt CNativeW::GetHabaOfChar(const wchar_t *pData, int nDataLen, int nIdx)
{
    //文字列範囲外なら 0
    if (nIdx >= nDataLen)
    {
        return CHabaXInt(0);
    }
    // HACK:改行コードに対して1を返す
    if (WCODE::IsLineDelimiter(pData[nIdx], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol))
    {
        return CHabaXInt(1);
    }

    // サロゲートチェック
    if (IsUTF16High(pData[nIdx]) && nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1]))
    {
        return CHabaXInt(WCODE::CalcPxWidthByFont2(pData + nIdx));
    }
    else if (IsUTF16Low(pData[nIdx]) && 0 < nIdx && IsUTF16High(pData[nIdx - 1]))
    {
        // サロゲートペア（下位）
        return CHabaXInt(0); // 不正位置
    }
    return CHabaXInt(WCODE::CalcPxWidthByFont(pData[nIdx]));
}

/* ポインタで示した文字の次にある文字の位置を返します */
/* 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します */
const wchar_t *CNativeW::GetCharNext(const wchar_t *pData, int nDataLen, const wchar_t *pDataCurrent)
{
    const wchar_t *pNext = pDataCurrent + 1;

    if (pNext >= &pData[nDataLen])
    {
        return &pData[nDataLen];
    }

    // サロゲートペア対応	2008/7/6 Uchi
    if (IsUTF16High(*pDataCurrent))
    {
        if (IsUTF16Low(*pNext))
        {
            pNext += 1;
        }
    }

    return pNext;
}

/* ポインタで示した文字の直前にある文字の位置を返します */
/* 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します */
const wchar_t *CNativeW::GetCharPrev(const wchar_t *pData, int nDataLen, const wchar_t *pDataCurrent)
{
    const wchar_t *pPrev = pDataCurrent - 1;
    if (pPrev <= pData)
    {
        return pData;
    }

    // サロゲートペア対応	2008/7/6 Uchi
    if (IsUTF16Low(*pPrev))
    {
        if (IsUTF16High(*(pPrev - 1)))
        {
            pPrev -= 1;
        }
    }

    return pPrev;
    //	return ::CharPrev( pData, pDataCurrent );
}
