/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#define SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#pragma once

#include "CNative.h"
#include "basis/SakuraBasis.h"
#include "charset/charcode.h"
#include "debug/Debug2.h" //assert

class CNativeW;

/*!
 * 文字列への参照を保持するクラス
 *
 * @author kobake
 * @date 2007/11/06 kobake 新規作成
 * @deprecated use std::wstring_view instead
 */
class CStringRef final{
public:
	CStringRef() noexcept = default;
	CStringRef( const wchar_t* pData, size_t nDataLen ) noexcept;
	explicit CStringRef( const CNativeW& cmem ) noexcept;

	[[nodiscard]] const wchar_t* GetPtr() const noexcept { return m_pData; }
	[[nodiscard]] int GetLength() const noexcept { return static_cast<int>(m_nDataLen); }
	[[nodiscard]] bool IsValid() const noexcept { return m_pData != nullptr; }
	[[nodiscard]] wchar_t At( size_t nIndex ) const noexcept;
	[[nodiscard]] wchar_t operator []( size_t nIndex ) const noexcept { return m_pData[nIndex]; }

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

/*!
 * ワイド文字列管理クラス
 *
 * @author kobake
 * @date 2007/11/06 kobake 新規作成
 */
class CNativeW : public CNative<WCHAR> {
private:
	using Me = CNativeW;
	using Base = CNative<WCHAR>;

public:
	//コンストラクタ・デストラクタ
	using Base::Base;

	using Base::operator=;
	using Base::operator+=;

	void AppendNativeData(const Me& cNative) { AppendRawData(cNative.GetRawPtr(), cNative.GetRawLength()); }      //!< バッファの最後にデータを追加する

	/*! メモリ確保済みかどうか */
	[[nodiscard]] bool IsValid() const noexcept { return GetStringPtr() != nullptr; }

	void SetStringHoldBuffer(_In_reads_(nDataLen) const char_type* pData, size_t nDataLen) { Base::SetRawDataHoldBuffer(pData, nDataLen * sizeof(char_type)); }
	void SetNativeData(const Me& cNative) { SetString(cNative.data(), cNative.length()); }	//!< バッファの内容を置き換える

	//演算子
	Me& operator += (const Me& rhs)			{ AppendNativeData(rhs); return *this; }

	friend bool operator == (const Me& lhs, const Me& rhs) noexcept { return 0 == lhs.Compare(rhs); }
	friend bool operator != (const Me& lhs, const Me& rhs) noexcept { return !(lhs == rhs); }

	//! 任意位置の文字取得。nIndexは文字単位。
	char_type operator[]( size_t nIndex ) const	//!< 任意位置の文字取得。nIndexは文字単位。
	{
		if (nIndex < length()) {
			return data()[nIndex];
		} else {
			return 0;
		}
	}

	CLogicInt GetStringLength() const { return CLogicInt(Base::GetStringLength()); }

	//末尾を1文字削る
	void Chop() {
		if (auto n = Base::GetStringLength(); 0 < n){
			_SetStringLength(n - 1);
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

// CMemory派生クラスにはメンバー追加禁止
static_assert(sizeof(CNativeW) == sizeof(CMemory), "size check");

#endif /* SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_ */
