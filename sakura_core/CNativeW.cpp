#include "stdafx.h"
#include "CNativeW.h"
#include "charcode.h"
#include "CEOL.h"
#include <mbstring.h>
#include "charset/CShiftJis.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CNativeW2::CNativeW2()
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
}

//! nDataLenは文字単位。
CNativeW2::CNativeW2( const wchar_t* pData, int nDataLen )
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,nDataLen);
}

CNativeW2::CNativeW2( const wchar_t* pData)
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,wcslen(pData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// バッファの内容を置き換える
void CNativeW2::SetString( const wchar_t* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void CNativeW2::SetString( const wchar_t* pszData )
{
	SetString(pszData,wcslen(pszData));
}

// バッファの内容を置き換える
void CNativeW2::SetNativeData( const CNativeW2& pcNative )
{
	CNative::SetRawData(pcNative);
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeW2::AllocStringBuffer( int nDataLen )
{
	CNative::AllocBuffer(nDataLen * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する
void CNativeW2::AppendString( const wchar_t* pszData )
{
	AppendString(pszData,wcslen(pszData));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeW2::AppendString( const wchar_t* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

//! バッファの最後にデータを追加する
void CNativeW2::AppendNativeData( const CNativeW2& cmemData )
{
	AppendString(cmemData.GetStringPtr(),cmemData.GetStringLength());
}

// -- -- charからの移行用 -- -- //

//! バッファの内容を置き換える。nDataLenは文字単位。
void CNativeW2::SetStringOld( const char* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(char));
	CShiftJis::SJISToUnicode(this->_GetMemory());
}

//! バッファの内容を置き換える
void CNativeW2::SetStringOld( const char* pszData )
{
	SetStringOld(pszData,strlen(pszData));
}

void CNativeW2::AppendStringOld( const char* pData, int nDataLen )
{
	wchar_t* szTmp=mbstowcs_new(pData,nDataLen);
	AppendString(szTmp);
	delete[] szTmp;
}

//! バッファの最後にデータを追加する。pszDataはSJIS。
void CNativeW2::AppendStringOld( const char* pszData )
{
	AppendStringOld(pszData,strlen(pszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()と同機能
wchar_t CNativeW2::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}

//! 文字列長を返す。文字単位。
CLogicInt CNativeW2::GetStringLength() const
{
	return CLogicInt(CNative::GetRawLength() / sizeof(wchar_t));
}


/* 等しい内容か */
bool CNativeW2::IsEqual( const CNativeW2& cmem1, const CNativeW2& cmem2 )
{
	if(&cmem1==&cmem2)return true;

	const wchar_t* psz1;
	const wchar_t* psz2;
	int nLen1;
	int nLen2;

	psz1 = cmem1.GetStringPtr( &nLen1 );
	psz2 = cmem2.GetStringPtr( &nLen2 );
	if( nLen1 == nLen2 ){
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
void CNativeW2::Replace( const wchar_t* pszFrom, const wchar_t* pszTo )
{
	CNativeW2	cmemWork;
	int			nFromLen = wcslen( pszFrom );
	int			nToLen = wcslen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= GetStringLength() - nFromLen ){
		if( 0 == wmemcmp( &GetStringPtr()[nBgn], pszFrom, nFromLen ) ){
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
	if( 0  < GetStringLength() - nBgnOld ){
		cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
	}
	SetNativeData( cmemWork );
}








// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字がwchar_t何個分かを返す
CLogicInt CNativeW2::GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen )
		return CLogicInt(0);

	return CLogicInt(1);
}

//! 指定した位置の文字が半角何個分かを返す
CLayoutInt CNativeW2::GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	//文字列範囲外なら 0
	if( nIdx >= nDataLen )
		return CLayoutInt(0);

	//半角文字なら 1
	if(WCODE::isHankaku(pData[nIdx]) )
		return CLayoutInt(1);

	//全角文字なら 2
	else
		return CLayoutInt(2);
}


/* ポインタで示した文字の次にある文字の位置を返します */
/* 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します */
const wchar_t* CNativeW2::GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pNext = pDataCurrent + 1;

	if( pNext >= &pData[nDataLen] ){
		pNext = &pData[nDataLen];
	}
	return pNext;
}

/* ポインタで示した文字の直前にある文字の位置を返します */
/* 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します */
const wchar_t* CNativeW2::GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pPrev = pDataCurrent - 1;
	if( pPrev < pData ){
		pPrev = pData;
	}
	return pPrev;
//	return ::CharPrevW2( pData, pDataCurrent );
}


//ShiftJISに変換して返す
const char* CNativeW2::GetStringPtrOld() const
{
	return to_achar(GetStringPtr(),GetStringLength());
}

