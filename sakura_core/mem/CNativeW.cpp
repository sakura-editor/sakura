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

#include "basis/CEol.h"

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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! nDataLenは文字単位。
CNativeW::CNativeW( const wchar_t* pData, size_t nDataLen )
{
	SetString( pData, nDataLen );
}

CNativeW::CNativeW(std::wstring_view text)
{
	SetString(std::data(text), std::size(text));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeW::SetString( const wchar_t* pData, size_t nDataLen )
{
	SetRawData( pData,nDataLen * sizeof(wchar_t) );
}

// バッファの内容を置き換える
void CNativeW::SetString( std::wstring_view data )
{
	SetString( data.data(), data.length() );
}

void CNativeW::SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen )
{
	SetRawDataHoldBuffer( pData, nDataLen * sizeof(wchar_t) );
}

// バッファの内容を置き換える
void CNativeW::SetNativeData( const CNativeW& cNative )
{
	SetRawData( cNative );
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeW::AllocStringBuffer( size_t nDataLen )
{
	AllocBuffer( nDataLen * sizeof(wchar_t) );
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeW::AppendString( const wchar_t* pszData, size_t nDataLen )
{
	AppendRawData( pszData, nDataLen * sizeof(wchar_t) );
}

//! バッファの最後にデータを追加する
void CNativeW::AppendString( std::wstring_view data )
{
	AppendString( data.data(), data.length() );
}

/*!
 * バッファの最後にデータを追加する (フォーマット機能付き)
 *
 * @param format フォーマット書式文字列
 * @param va_args C-style の可変長引数
 * @throws std::invalid_argument formatが無効値
 * @throws std::bad_alloc メモリ確保に失敗
 * @remark 不正なフォーマットを指定すると無効なパラメータ例外で即死します。
 */
void CNativeW::AppendStringF( std::wstring_view format, ... )
{
	// _vscwprintf に NULL を渡してはならないので除外する
	if( format.empty() ){
		throw std::invalid_argument( "format can't be empty" );
	}

	// 可変長引数のポインタを取得
	va_list v;
	va_start( v, format );

	// 整形によって追加される文字数をカウント
	const int additional = ::_vscwprintf( format.data(), v );

	// 現在の文字列長を取得
	const auto currentLength = GetStringLength();

	// 現在の文字数 + 追加文字数が収まるようにバッファを拡張する
	const auto newCapacity = currentLength + additional;
	AllocStringBuffer( newCapacity );

	int added = 0;
	if( additional > 0 ){
		// 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
		added = ::_vsnwprintf_s( &GetStringPtr()[currentLength], static_cast<unsigned>(additional) + 1, _TRUNCATE, format.data(), v );
	}

	// 可変長引数のポインタを解放
	va_end( v );

	// 文字列終端を再設定する
	_SetStringLength( currentLength + added );
}

//! バッファの最後にデータを追加する
void CNativeW::AppendNativeData( const CNativeW& cmemData )
{
	AppendRawData(cmemData.GetStringPtr(), cmemData.GetRawLength());
}

/*!
 * 指定した文字列を連結した文字列バッファを作成する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字列(std::wstring_view)
 * @return 新しい文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW operator + (const CNativeW& lhs, std::wstring_view rhs) noexcept(false)
{
	CNativeW tmp(lhs);
	return (tmp += rhs);
}

/*!
 * 指定した文字列を連結した文字列バッファを作成する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字列(std::wstring_view)
 * @return 新しい文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW operator + (std::wstring_view lhs, const CNativeW& rhs) noexcept(false)
{
	CNativeW tmp(lhs);
	return (tmp += rhs);
}

/*!
 * 指定した文字列を連結する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字列(std::wstring_view)
 * @return 文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW& operator += (CNativeW& lhs, std::wstring_view rhs) noexcept(false)
{
	lhs.AppendString(rhs);
	return lhs;
}

/*!
 * 指定した文字を連結する
 *
 * @param lhs 文字列バッファ(CNativeW)
 * @param rhs 文字(wchar_t)
 * @return 文字列バッファ
 * @throws std::bad_alloc メモリ確保に失敗した
 */
CNativeW& operator += (CNativeW& lhs, wchar_t rhs) noexcept(false)
{
	return (lhs += std::wstring_view(&rhs, 1));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()と同機能
[[nodiscard]] wchar_t CNativeW::operator[]( size_t nIndex ) const
{
	if( nIndex < static_cast<size_t>(GetStringLength()) ){
		return GetStringPtr()[nIndex];
	}else{
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
int CNativeW::Compare(const CNativeW& rhs) const noexcept
{
	if (this == &rhs) return 0;
	const auto lhsValid = IsValid();
	const auto rhsValid = rhs.IsValid();
	if (!lhsValid && !rhsValid) return 0;
	if (!lhsValid) return -1;
	if (!rhsValid) return 1;
	const auto lhs = static_cast<std::wstring_view>(*this);
	return lhs.compare(rhs);
}

/* 等しい内容か */
bool CNativeW::IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 )
{
	return cmem1.Equals(cmem2);
}

/*!
 * 同型との等価比較
 *
 * @param lhs 比較対象(CNativeW)
 * @param rhs 比較対象(CNativeW)
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator == (const CNativeW& lhs, const CNativeW& rhs) noexcept
{
	return lhs.Equals(rhs);
}

/*!
 * 文字列との等価比較
 *
 * @param lhs 比較対象(CNativeW)
 * @param rhs 比較対象(std::wstring_view)
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator == (const CNativeW& lhs, std::wstring_view rhs) noexcept(false)
{
	return 0 == lhs.Compare(rhs);
}

/*!
 * 文字列ポインタとの等価比較
 *
 * @param lhs 比較対象(CNativeW)
 * @param rhs 比較対象(C String)
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator == (const CNativeW& lhs, LPCWSTR rhs) noexcept(false)
{
	// rhsがNULLでない場合、文字列として比較する
	if (rhs) return (lhs == std::wstring_view(rhs));

	// rhsがNULLの場合、メモリ未確保を等しいとみなす
	return !lhs.IsValid();
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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字がwchar_t何個分かを返す
CLogicInt CNativeW::GetSizeOfChar( const wchar_t* pData, size_t cchData, size_t index )
{
	const auto nDataLen = int(cchData);
	const auto nIdx = int(index);

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
CKetaXInt CNativeW::GetKetaOfChar( const wchar_t* pData, size_t cchData, size_t index, CCharWidthCache& cache)
{
	const auto nDataLen = int(cchData);
	const auto nIdx = int(index);

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
CHabaXInt CNativeW::GetHabaOfChar( const wchar_t* pData, size_t cchData, size_t index,
	bool bEnableExtEol, CCharWidthCache& cache )
{
	const auto nDataLen = int(cchData);
	const auto nIdx = int(index);

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
const wchar_t* CNativeW::GetCharNext( const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent )
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
