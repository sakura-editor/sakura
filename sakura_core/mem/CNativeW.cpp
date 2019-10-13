﻿/*! @file */
#include "StdAfx.h"
#include "mem/CNativeW.h"
#include "CEol.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CNativeW::CNativeW() noexcept
	: CNative()
{
}

CNativeW::CNativeW(const CNativeW& rhs)
	: CNative(rhs)
{
}

CNativeW::CNativeW(CNativeW&& other) noexcept
	: CNative(std::forward<CNativeW>(other))
{
}

//! nDataLenは文字単位。
CNativeW::CNativeW( const wchar_t* pData, int nDataLen )
	: CNative()
{
	SetString(pData, nDataLen);
}

CNativeW::CNativeW( const wchar_t* pData )
	: CNative()
{
	SetString(pData);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeW::SetString( const wchar_t* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void CNativeW::SetString( const wchar_t* pszData )
{
	CNative::SetRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

void CNativeW::SetStringHoldBuffer( const wchar_t* pData, int nDataLen )
{
	CNative::SetRawDataHoldBuffer(pData, nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void CNativeW::SetNativeData( const CNativeW& pcNative )
{
	CNative::SetRawData(pcNative);
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeW::AllocStringBuffer( int nDataLen )
{
	CNative::AllocBuffer(nDataLen * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する
void CNativeW::AppendString( const wchar_t* pszData )
{
	CNative::AppendRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeW::AppendString( const wchar_t* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する (フォーマット機能付き)
void CNativeW::AppendStringF(const wchar_t* pszData, ...)
{
	wchar_t buf[2048];

	// 整形
	va_list v;
	va_start(v, pszData);
	int len = _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, pszData, v);
	int e = errno;
	va_end(v);

	if (len == -1) {
		DEBUG_TRACE(L"AppendStringF error. errno = %d", e);
		throw std::exception();
	}

	// 追加
	this->AppendString(buf, len);
}

//! バッファの最後にデータを追加する
void CNativeW::AppendNativeData( const CNativeW& cmemData )
{
	CNative::AppendRawData(cmemData.GetStringPtr(), cmemData.GetRawLength());
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()と同機能
wchar_t CNativeW::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 文字列置換
void CNativeW::Replace( const wchar_t* pszFrom, const wchar_t* pszTo )
{
	int			nFromLen = wcslen( pszFrom );
	int			nToLen = wcslen( pszTo );
	Replace( pszFrom, nFromLen, pszTo, nToLen );
}

void CNativeW::Replace( const wchar_t* pszFrom, int nFromLen, const wchar_t* pszTo, int nToLen )
{
	CNativeW	cmemWork;
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= GetStringLength() - nFromLen ){
		if( 0 == wmemcmp( &GetStringPtr()[nBgn], pszFrom, nFromLen ) ){
			if( nBgnOld == 0 && nFromLen <= nToLen ){
				cmemWork.AllocStringBuffer( GetStringLength() );
			}
			if( 0  < nBgn - nBgnOld ){
				cmemWork.AppendString( &GetStringPtr()[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( nBgnOld != 0 ){
		if( 0  < GetStringLength() - nBgnOld ){
			cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
		}
		SetNativeData( cmemWork );
	}else{
		if( this->GetStringPtr() == NULL ){
			this->SetString(L"");
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字が半角何個分かを返す
CKetaXInt CNativeW::GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx )
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
	if(WCODE::IsHankaku(pData[nIdx]) )
		return CKetaXInt(1);

	//全角文字なら 2
	else
		return CKetaXInt(2);
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

/* ポインタで示した文字の直前にある文字の位置を返します */
/* 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します */
const wchar_t* CNativeW::GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pPrev = pDataCurrent - 1;
	if( pPrev <= pData ){
		return pData;
	}

	// サロゲートペア対応	2008/7/6 Uchi
	if (IsUTF16Low(*pPrev)) {
		if (IsUTF16High(*(pPrev-1))) {
			pPrev -= 1;
		}
	}

	return pPrev;
//	return ::CharPrev( pData, pDataCurrent );
}
