﻿/*! @file */
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
#ifndef SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#define SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#pragma once

#include "CNative.h"
#include "basis/SakuraBasis.h"
#include "charset/charcode.h"
#include "debug/Debug2.h" //assert

class CNativeW;

//! 文字列への参照を保持するクラス
class CStringRef final{
public:
	CStringRef() noexcept = default;
	CStringRef( const wchar_t* pData, size_t nDataLen ) noexcept;
	explicit CStringRef( const CNativeW& cmem ) noexcept;

	[[nodiscard]] const wchar_t* GetPtr() const noexcept { return m_pData; }
	[[nodiscard]] int GetLength() const noexcept { return static_cast<int>(m_nDataLen); }
	[[nodiscard]] bool IsValid() const noexcept { return m_pData != nullptr; }
	[[nodiscard]] wchar_t At( size_t nIndex ) const noexcept;

private:
	const wchar_t*	m_pData = nullptr;
	unsigned		m_nDataLen = 0;
};

// グローバル演算子の前方宣言
bool operator == (const CNativeW& lhs, const wchar_t* rhs) noexcept;
bool operator != (const CNativeW& lhs, const wchar_t* rhs) noexcept;
bool operator == (const wchar_t* lhs, const CNativeW& rhs) noexcept;
bool operator != (const wchar_t* lhs, const CNativeW& rhs) noexcept;
CNativeW operator + (const CNativeW& lhs, const wchar_t* rhs) noexcept(false);
CNativeW operator + (const wchar_t* lhs, const CNativeW& rhs) noexcept(false);

//! UNICODE文字列管理クラス
class CNativeW final : public CNative{
	friend bool operator == (const CNativeW& lhs, const wchar_t* rhs) noexcept;

public:
	//コンストラクタ・デストラクタ
	CNativeW() noexcept = default;
	CNativeW( const wchar_t* pData, size_t nDataLen ); //!< nDataLenは文字単位。
	CNativeW( const wchar_t* pData );

	/*! メモリ確保済みかどうか */
	[[nodiscard]] bool IsValid() const noexcept { return GetStringPtr() != nullptr; }

	//管理
	void AllocStringBuffer( size_t nDataLen );                    //!< (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	//WCHAR
	void SetString( const wchar_t* pData, size_t nDataLen );			//!< バッファの内容を置き換える。nDataLenは文字単位。
	void SetString( const wchar_t* pszData );							//!< バッファの内容を置き換える。
	void SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen );
	void AppendString( const wchar_t* pszData, size_t nDataLen );		//!< バッファの最後にデータを追加する。nLengthは文字単位。成功すればtrue。メモリ確保に失敗したらfalseを返す。
	void AppendString( std::wstring_view data );						//!< バッファの最後にデータを追加する
	void AppendStringF( std::wstring_view format, ... );				//!< バッファの最後にデータを追加する (フォーマット機能付き)

	//CNativeW
	void SetNativeData( const CNativeW& cNative );						//!< バッファの内容を置き換える
	void AppendNativeData( const CNativeW& cNative );					//!< バッファの最後にデータを追加する

	//演算子
	CNativeW  operator + (const CNativeW& rhs) const	{ return (CNativeW(*this) += rhs); }
	CNativeW& operator += (const CNativeW& rhs)			{ AppendNativeData(rhs); return *this; }
	CNativeW& operator += (wchar_t ch)					{ return (*this += CNativeW(&ch, 1)); }
	bool operator == (const CNativeW& rhs) const noexcept { return 0 == Compare(rhs); }
	bool operator != (const CNativeW& rhs) const noexcept { return !(*this == rhs); }

	//ネイティブ取得インターフェース
	[[nodiscard]] wchar_t operator[]( size_t nIndex ) const;                    //!< 任意位置の文字取得。nIndexは文字単位。
	CLogicInt GetStringLength() const                        //!< 文字列長を返す。文字単位。
	{
		return CLogicInt(CNative::GetRawLength() / sizeof(wchar_t));
	}
	const wchar_t* GetStringPtr() const
	{
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
	wchar_t* GetStringPtr()
	{
		return reinterpret_cast<wchar_t*>(GetRawPtr());
	}

	//特殊
	void _SetStringLength( size_t nLength )
	{
		_SetRawLength( nLength * sizeof(wchar_t) );
	}
	//末尾を1文字削る
	void Chop()
	{
		int n = GetStringLength();
		n-=1;
		if(n>=0){
			_SetStringLength(n);
		}
	}
	//! メモリバッファを入れ替える
	void swap( CNativeW& left ){
		CMemory::swap( left );
	}
	//! メモリ再確保を行わずに格納できる最大文字数を求める
	[[nodiscard]] int capacity() const noexcept {
		return CMemory::capacity() / sizeof(wchar_t);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           判定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	int Compare(const CNativeW& rhs) const noexcept;
	int Compare(const wchar_t* rhs) const noexcept;
	bool Equals(const CNativeW& rhs) const noexcept { return 0 == Compare(rhs); }
	bool Equals(const wchar_t* rhs) const noexcept { return 0 == Compare(rhs); }

	//! 同一の文字列ならtrue
	static bool IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           変換                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	void Replace( std::wstring_view strFrom, std::wstring_view strTo );   //!< 文字列置換
	void Replace( const wchar_t* pszFrom, size_t nFromLen, const wchar_t* pszTo, size_t nToLen );   //!< 文字列置換

public:
	// -- -- staticインターフェース -- -- //
	//! 指定した位置の文字がwchar_t何個分かを返す
	static CLogicInt GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx );
	static CLogicInt GetSizeOfChar( const CStringRef& cStr, int nIdx )
		{ return GetSizeOfChar( cStr.GetPtr(), cStr.GetLength(), nIdx ); }
	//! 指定した位置の文字が半角何個分かを返す
	static CKetaXInt GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx,
		CCharWidthCache& cache = GetCharWidthCache() );
	static CKetaXInt GetKetaOfChar(const CStringRef& cStr, int nIdx, CCharWidthCache& cache = GetCharWidthCache())
		{ return GetKetaOfChar(cStr.GetPtr(), cStr.GetLength(), nIdx, cache); }
	static const wchar_t* GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent ); //!< ポインタで示した文字の次にある文字の位置を返します
	static const wchar_t* GetCharPrev(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); //!< ポインタで示した文字の直前にある文字の位置を返します

	static CHabaXInt GetHabaOfChar( const wchar_t* pData, int nDataLen, int nIdx,
		bool bEnableExtEol, CCharWidthCache& cache = GetCharWidthCache() );
	static CLayoutXInt GetColmOfChar( const wchar_t* pData,
		int nDataLen, int nIdx, bool bEnableExtEol )
		{ return GetHabaOfChar(pData,nDataLen,nIdx, bEnableExtEol); }
	static CLayoutXInt GetColmOfChar( const CStringRef& cStr, int nIdx, bool bEnableExtEol )
		{ return GetHabaOfChar(cStr.GetPtr(), cStr.GetLength(), nIdx, bEnableExtEol); }
};

// 派生クラスでメンバー追加禁止
static_assert(sizeof(CNativeW) == sizeof(CNative), "size check");

#endif /* SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_ */
