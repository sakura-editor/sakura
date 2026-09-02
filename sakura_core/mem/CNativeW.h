/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

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

//! 文字列への参照を保持するクラス
// TODO: いつか廃止する
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

	/*!
	 * @brief 文字列が空かどうか調べる
	 */
	constexpr bool empty() const noexcept { return 0 == length(); }

	/*!
	 * @brief 文字列長を取得する
	 */
	constexpr size_t length() const noexcept
	{
		return static_cast<size_t>(m_nDataLen);
	}

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 * C++への移行を加速させるために仮置き。
	 *
	 * @return 文字列参照
	 */
	constexpr /* implicit */ operator std::wstring_view()   const & noexcept { return std::wstring_view{ m_pData, m_nDataLen }; }

private:
	const wchar_t*	m_pData = nullptr;
	unsigned		m_nDataLen = 0;
};

// グローバル演算子の前方宣言
bool operator == (const CNativeW& lhs, const CNativeW& rhs) noexcept;
bool operator == (const CNativeW& lhs, std::wstring_view rhs);
bool operator == (const CNativeW& lhs, LPCWSTR rhs);
CNativeW operator + (const CNativeW& lhs, std::wstring_view rhs) noexcept(false);
CNativeW operator + (std::wstring_view lhs, const CNativeW& rhs) noexcept(false);
CNativeW& operator += (CNativeW& lhs, std::wstring_view rhs) noexcept(false);
CNativeW& operator += (CNativeW& lhs, wchar_t rhs) noexcept(false);

//! UNICODE文字列管理クラス
class CNativeW final : public CNative{
private:
	friend bool operator == (const CNativeW& lhs, const CNativeW& rhs) noexcept;
	friend bool operator == (const CNativeW& lhs, std::wstring_view rhs) noexcept(false);
	friend bool operator == (const CNativeW& lhs, LPCWSTR rhs) noexcept(false);
	friend CNativeW operator + (const CNativeW& lhs, std::wstring_view rhs) noexcept(false);
	friend CNativeW operator + (std::wstring_view lhs, const CNativeW& rhs) noexcept(false);
	friend CNativeW& operator += (CNativeW& lhs, std::wstring_view rhs) noexcept(false);
	friend CNativeW& operator += (CNativeW& lhs, wchar_t rhs) noexcept(false);

	using Me = CNativeW;

public:
	//コンストラクタ・デストラクタ
	CNativeW() noexcept = default;
	CNativeW( const wchar_t* pData, size_t nDataLen ); //!< nDataLenは文字単位。

	explicit CNativeW(std::wstring_view text);

	/*! メモリ確保済みかどうか */
	[[nodiscard]] bool IsValid() const noexcept { return GetStringPtr() != nullptr; }

	//管理
	void AllocStringBuffer( size_t nDataLen );                    //!< (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	//WCHAR
	void SetString( const wchar_t* pData, size_t nDataLen );			//!< バッファの内容を置き換える。nDataLenは文字単位。
	void SetString( std::wstring_view data );							//!< バッファの内容を置き換える。
	void SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen );
	void AppendString( const wchar_t* pszData, size_t nDataLen );		//!< バッファの最後にデータを追加する。nLengthは文字単位。成功すればtrue。メモリ確保に失敗したらfalseを返す。
	void AppendString( std::wstring_view data );						//!< バッファの最後にデータを追加する
	void AppendStringF( std::wstring_view format, ... );				//!< バッファの最後にデータを追加する (フォーマット機能付き)

	//CNativeW
	void SetNativeData( const CNativeW& cNative );						//!< バッファの内容を置き換える
	void AppendNativeData( const CNativeW& cNative );					//!< バッファの最後にデータを追加する

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

	/*!
	 * @brief 文字列が空かどうか調べる
	 */
	bool empty() const noexcept { return 0 == length(); }

	/*!
	 * @brief 文字列長を取得する
	 */
	size_t length() const noexcept
	{
		return static_cast<size_t>(GetStringLength());
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
	
	int		Compare(const CNativeW& rhs) const noexcept;
	int		Compare(std::wstring_view rhs) const { return Compare(CNativeW(rhs)); }
	bool	Equals(const CNativeW& rhs) const noexcept { return 0 == Compare(rhs); }

	//! 同一の文字列ならtrue
	static bool IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           変換                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	void Replace( std::wstring_view strFrom, std::wstring_view strTo );   //!< 文字列置換

	/*!
	 * @brief 文字列を代入する演算子
	 *
	 * @param rhs [in] 代入元文字列
	 * @return 自分自身への参照
	 */
	Me& operator = (std::wstring_view rhs) noexcept { SetString(rhs.data(), rhs.length()); return *this; }

	Me& operator = (const std::wstring& rhs) noexcept { return operator = (static_cast<std::wstring_view>(rhs)); }
	Me& operator = (const std::filesystem::path& path) noexcept { return operator = (path.native()); }

	/*!
	 * @brief 文字列を代入する演算子
	 *
	 * @param rhs [in] 代入元文字列
	 * @return 自分自身への参照
	 */
	template<size_t N>
	Me& operator = (const WCHAR (&rhs)[N]) noexcept
	{
		return operator = (std::wstring_view(rhs, N - 1));
	}

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 * C++への移行を加速させるために仮置き。
	 *
	 * @return 文字列参照
	 */
	/* implicit */ operator std::wstring_view() const & noexcept { return std::wstring_view(GetStringPtr(), static_cast<size_t>(GetStringLength())); }

public:
	// -- -- staticインターフェース -- -- //
	//! 指定した位置の文字がwchar_t何個分かを返す
	static CLogicInt GetSizeOfChar( const wchar_t* pData, size_t cchData, size_t index );
	static CLogicInt GetSizeOfChar( const CStringRef& cStr, size_t index )
		{ return GetSizeOfChar( cStr.GetPtr(), cStr.GetLength(), index ); }
	//! 指定した位置の文字が半角何個分かを返す
	static CKetaXInt GetKetaOfChar(const wchar_t* pData, size_t cchData, size_t index, CCharWidthCache& cache = GetCharWidthCache());
	static CKetaXInt GetKetaOfChar(const CStringRef& cStr, size_t index, CCharWidthCache& cache = GetCharWidthCache())
		{ return GetKetaOfChar(cStr.GetPtr(), cStr.GetLength(), index, cache); }
	static const wchar_t* GetCharNext(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); //!< ポインタで示した文字の次にある文字の位置を返します
	static const wchar_t* GetCharPrev(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); //!< ポインタで示した文字の直前にある文字の位置を返します

	static CHabaXInt GetHabaOfChar( const wchar_t* pData, size_t cchData, size_t index, bool bEnableExtEol, CCharWidthCache& cache = GetCharWidthCache() );
	static CLayoutXInt GetColmOfChar( const wchar_t* pData, size_t cchData, size_t index, bool bEnableExtEol )
		{ return GetHabaOfChar(pData, cchData, index, bEnableExtEol); }
	static CLayoutXInt GetColmOfChar( const CStringRef& cStr, size_t index, bool bEnableExtEol )
		{ return GetHabaOfChar(cStr.GetPtr(), cStr.GetLength(), index, bEnableExtEol); }
};

// 派生クラスでメンバー追加禁止
static_assert(sizeof(CNativeW) == sizeof(CNative), "size check");

#endif /* SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_ */
