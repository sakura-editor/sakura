/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include <stdexcept>
#include "charset/codechecker.h"
#include "mem/CNativeW.h"

#include <string_view>

#include "CEol.h"

/*!
	コンストラクタ

	C文字列の先頭アドレスと有効文字数を指定してCStringRefを構築する。
 */
CStringRef::CStringRef( const wchar_t* pData, size_t nDataLen ) noexcept
	: m_pData(pData)
	, m_nDataLen(static_cast<decltype(m_nDataLen)>(nDataLen))
{
}

/*!
	コンストラクタ

	指定したCNativeWを参照するCStringRefを構築する。
 */
CStringRef::CStringRef( const CNativeW& cmem ) noexcept
	: m_pData(cmem.GetStringPtr())
	, m_nDataLen(static_cast<decltype(m_nDataLen)>(cmem.GetStringLength()))
{
}

/*!
	指定位置の文字を取得する

	標準ライブラリの実装とは異なり、範囲外を指定すると0が返る。
	サクラエディタの内部データは拡張UTF-16LEなので、
	取得した値が「1文字」であるとは限らないことに注意。
 */
[[nodiscard]] wchar_t CStringRef::At( size_t nIndex ) const noexcept
{
	if( m_pData != nullptr && nIndex < m_nDataLen ){
		return m_pData[nIndex];
	}
	return 0;
}

/*!
 * 指定した文字列を連結した文字列バッファを作成する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字列ポインタ(C string)
 * @return 新しい文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW operator + (const CNativeW& lhs, const wchar_t* rhs) noexcept(false)
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
CNativeW operator + (const wchar_t* lhs, const CNativeW& rhs) noexcept(false)
{
	CNativeW tmp(lhs);
	return tmp + rhs;
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
int CNativeW::Compare(const CNativeW& rhs) const noexcept
{
	if (this == &rhs) return 0;
	const int lhsIsValid = static_cast<int>(IsValid());
	const int rhsIsValid = static_cast<int>(rhs.IsValid());
	if (!rhsIsValid || !lhsIsValid) return lhsIsValid - rhsIsValid;
	// データ長が短い方を基準に比較を行う
	const int lhsLength = static_cast<int>(GetStringLength());
	const int rhsLength = static_cast<int>(rhs.GetStringLength());
	const int minLength = std::min(lhsLength, rhsLength);
	// データ長の範囲で文字列を比較する
	auto cmp = wmemcmp(GetStringPtr(), rhs.GetStringPtr(), minLength);
	if (!cmp) cmp = lhsLength - rhsLength;
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
int CNativeW::Compare(const wchar_t* rhs) const noexcept
{
	const int lhsIsValid = static_cast<int>(IsValid());
	const int rhsIsValid = rhs ? 1 : 0;
	if (!rhsIsValid || !lhsIsValid) return lhsIsValid - rhsIsValid;
	const wchar_t* lhs = GetStringPtr();
	const size_t lhsLength = GetStringLength();
	// NUL終端考慮のために終端を拡張し、比較自体はCRTに丸投げする
	return wcsncmp(lhs, rhs, lhsLength + 1);
}

/* 等しい内容か */
bool CNativeW::IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 )
{
	if(&cmem1==&cmem2)return true;

	const int nLen1 = cmem1.GetStringLength();
	const int nLen2 = cmem2.GetStringLength();
	if( nLen1 == nLen2 ){
		const wchar_t* psz1 = cmem1.GetStringPtr();
		const wchar_t* psz2 = cmem2.GetStringPtr();
		if( 0 == wmemcmp( psz1, psz2, nLen1 ) ){
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
bool operator == (const CNativeW& lhs, const wchar_t* rhs) noexcept
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
bool operator != (const CNativeW& lhs, const wchar_t* rhs) noexcept
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
bool operator == (const wchar_t* lhs, const CNativeW& rhs) noexcept
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
bool operator != (const wchar_t* lhs, const CNativeW& rhs) noexcept
{
	return !(lhs == rhs);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 文字列置換
void CNativeW::Replace( std::wstring_view strFrom, std::wstring_view strTo )
{
	CNativeW	cmemWork(L"");
	size_t		nBgn = 0;
	size_t		nBgnOld = 0;
	while( nBgn + strFrom.length() <= static_cast<size_t>(GetStringLength()) ){
		if( 0 == wmemcmp( &GetStringPtr()[nBgn], strFrom.data(), strFrom.length() ) ){
			if( nBgnOld  < nBgn ){
				cmemWork.AppendString( &GetStringPtr()[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( strTo.data(), strTo.length() );
			nBgn = nBgn + strFrom.length();
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( nBgnOld < static_cast<size_t>(GetStringLength()) ){
		cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
	}
	SetRawDataHoldBuffer( cmemWork );
}

void CNativeW::Replace( const wchar_t* pszFrom, size_t nFromLen, const wchar_t* pszTo, size_t nToLen )
{
	Replace( std::wstring_view( pszFrom, nFromLen ), std::wstring_view( pszTo, nToLen ) );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字がwchar_t何個分かを返す
CLogicInt CNativeW::GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen )
		return CLogicInt(0);

	// サロゲートチェック					2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])) {
			// サロゲートペア 2個分
			return CLogicInt(2);
		}
	}

	// IVSの異体字セレクタチェック
	if (IsVariationSelector(std::wstring_view(pData + nIdx + 1, nDataLen - (nIdx + 1)))) {
		// 正字 + 異体字セレクタで3個分
		return CLogicInt(3);
	}

	return CLogicInt(1);
}

//! 指定した位置の文字が半角何個分かを返す
CKetaXInt CNativeW::GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx, CCharWidthCache& cache)
{
	//文字列範囲外なら 0
	if( nIdx >= nDataLen )
		return CKetaXInt(0);

	// サロゲートチェック BMP 以外は全角扱い		2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		return CKetaXInt(2);	// 仮
	}
	if (IsUTF16Low(pData[nIdx])) {
		if (nIdx > 0 && IsUTF16High(pData[nIdx - 1])) {
			// サロゲートペア（下位）
			return CKetaXInt(0);
		}
		// 単独（ブロークンペア）
		// return CKetaXInt(2);
		 if( IsBinaryOnSurrogate(pData[nIdx]) )
			return CKetaXInt(1);
		else
			return CKetaXInt(2);
	}

	//半角文字なら 1
	if(WCODE::IsHankaku(pData[nIdx], cache))
		return CKetaXInt(1);

	//全角文字なら 2
	else
		return CKetaXInt(2);
}

//! 指定した位置の文字の文字幅を返す
CHabaXInt CNativeW::GetHabaOfChar( const wchar_t* pData, int nDataLen, int nIdx,
	bool bEnableExtEol, CCharWidthCache& cache )
{
	//文字列範囲外なら 0
	if( nIdx >= nDataLen ){
		return CHabaXInt(0);
	}
	// HACK:改行コードに対して1を返す
	if( WCODE::IsLineDelimiter(pData[nIdx], bEnableExtEol) ){
		return CHabaXInt(1);
	}

	// サロゲートチェック
	if(IsUTF16High(pData[nIdx]) && nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])){
		return CHabaXInt(cache.CalcPxWidthByFont2(pData + nIdx));
	}else if(IsUTF16Low(pData[nIdx]) && 0 < nIdx && IsUTF16High(pData[nIdx - 1])) {
		// サロゲートペア（下位）
		return CHabaXInt(0); // 不正位置
	}
	return CHabaXInt(cache.CalcPxWidthByFont(pData[nIdx]));
}

/* ポインタで示した文字の次にある文字の位置を返します */
/* 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します */
const wchar_t* CNativeW::GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pNext = pDataCurrent + 1;

	if( pNext >= &pData[nDataLen] ){
		return &pData[nDataLen];
	}

	// サロゲートペア対応	2008/7/6 Uchi
	if (IsUTF16High(*pDataCurrent)) {
		if (IsUTF16Low(*pNext)) {
			pNext += 1;
		}
	}

	return pNext;
}

/*!
	ポインタで示した文字の直前にある文字の位置を返します
	直前にある文字がバッファの先頭の位置を越える場合はpDataを返します

	@date 2008/07/06 Uchi サロゲートペア対応
 */
const wchar_t* CNativeW::GetCharPrev(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent)
{
	if (const ptrdiff_t nDataPrev = pDataCurrent - pData - 1;
		0 < nDataPrev)
	{
		std::wstring_view data(pData, nDataLen);
		if (1 < nDataPrev &&
			IsUTF16Low(data[nDataPrev]) &&
			IsUTF16High(data[nDataPrev - 1]))
		{
			return &data[nDataPrev - 1];
		}
		return &data[nDataPrev];
	}

	return pData;
}
