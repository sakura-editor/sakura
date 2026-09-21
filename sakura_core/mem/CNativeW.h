/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#define SAKURA_CNATIVEW_3B48F63E_5B62_4FAB_9718_0D80114E20C1_H_
#pragma once

#include "basis/SakuraBasis.h"
#include "basis/primitive.h"
#include "charset/charcode.h"
#include "debug/Debug2.h" //assert
#include "mem/CNative.h"

#include <string_view>

class CNativeW;

//! 文字列への参照を保持するクラス
// TODO: いつか廃止する
class CStringRef final{
public:
	CStringRef() noexcept = default;

	CStringRef( const wchar_t* pData, size_t nDataLen ) noexcept;

	/*!
	 * @brief 文字列を指定して構築する
	 *
	 * @tparam A [in] 文字列の型（文字列参照に変換できる型）
	 * @param source [in] 文字列
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
	constexpr explicit CStringRef(const A& source)
	{
		// 入力元をNUL終端文字列とみなす
		const auto szText = cxx::NullTerminatedString<WCHAR>{ source };

		// 入力元を文字列として扱う
		const auto text = static_cast<std::wstring_view>(szText);

		m_pData = text.data();
		m_nDataLen = static_cast<unsigned>(text.length());
	}

	/*!
	 * @brief バッファへのポインタを取得する
	 */
	constexpr LPCWSTR data() const noexcept
	{
		return m_pData;
	}

	/*!
	 * @brief 文字列が空かどうか調べる
	 */
	constexpr bool empty() const noexcept
	{
		return 0 == m_nDataLen || !data() || 0 == data()[0];
	}

	/*!
	 * @brief 文字列長を取得する
	 */
	constexpr size_t length() const noexcept
	{
		return m_nDataLen;
	}

	/*!
	 * @brief 文字列参照を取得する
	 *
	 * @return 文字列参照
	 */
	constexpr std::wstring_view str() const noexcept
	{
		return std::wstring_view{ data(), length() };
	}

	[[nodiscard]] const wchar_t* GetPtr() const noexcept { return m_pData; }
	[[nodiscard]] int GetLength() const noexcept { return static_cast<int>(m_nDataLen); }
	[[nodiscard]] bool IsValid() const noexcept { return m_pData != nullptr; }
	[[nodiscard]] wchar_t At( size_t nIndex ) const noexcept;
	[[nodiscard]] wchar_t operator []( size_t nIndex ) const noexcept { return m_pData[nIndex]; }

	/*!
	 * @brief 文字列に変換する演算子
	 *
	 * @return 文字列
	 */
	constexpr explicit operator std::wstring() const
	{
		return std::wstring{ str() };
	}

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * @return 文字列参照
	 */
	constexpr explicit operator std::wstring_view() const& noexcept
	{
		return str();
	}

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
private:
	using Base = CNative;
	using Me = CNativeW;

	friend bool operator == (const CNativeW& lhs, const wchar_t* rhs) noexcept;

public:
	//コンストラクタ・デストラクタ
	CNativeW() noexcept = default;

	/*!
	 * @brief 文字列をコピーして構築する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 *
	 * @param source [in, opt] 文字列
	 */
	/* implicit */ CNativeW(_In_opt_z_ LPCWSTR source)
	{
		SetString(source);
	}

	/*!
	 * @brief 文字列をコピーして構築する
	 *
	 * @tparam A [in] コピーする文字列の型（文字列参照に変換できる型）
	 * @param source [in] コピーする文字列
	 */
	template<basis::NullTerminatedStringConstructible<WCHAR> A>
	explicit CNativeW(const A& source)
	{
		// 入力元をNUL終端文字列とみなす
		const auto szText = cxx::NullTerminatedString<WCHAR>{ source };

		// 入力元を文字列として扱う
		const auto text = static_cast<std::wstring_view>(szText);
		SetString(text.data(), text.length());
	}

	CNativeW( const wchar_t* pData, size_t nDataLen ); //!< nDataLenは文字単位。

	/*!
	 * @brief バッファへのポインタを取得する
	 */
	LPCWSTR data() const noexcept
	{
		return GetStringPtr();
	}

	/*!
	 * @brief 文字列が空かどうか調べる
	 */
	bool empty() const noexcept
	{
		return !length() || !data() || !data()[0];
	}

	/*!
	 * @brief 文字列長を取得する
	 */
	size_t length() const noexcept
	{
		return static_cast<size_t>(GetStringLength());
	}

	/*!
	 * @brief 文字列参照を取得する
	 *
	 * @return 文字列参照
	 */
	std::wstring_view str() const noexcept
	{
		return std::wstring_view{ data(), length() };
	}

	/*! メモリ確保済みかどうか */
	[[nodiscard]] bool IsValid() const noexcept { return GetStringPtr() != nullptr; }

	//管理
	void AllocStringBuffer( size_t nDataLen );                    //!< (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	//WCHAR
	void SetString( const wchar_t* pData, size_t nDataLen );			//!< バッファの内容を置き換える。nDataLenは文字単位。

	/*!
	 * @brief バッファの内容を置き換える
	 *
	 * @param pszData [in, opt] コピーする文字列
	 */
	void SetString(_In_opt_z_ LPCWSTR pszData)
	{
		if (pszData) {
			std::wstring_view data(pszData);
			SetString(data.data(), data.length());
		}
		else {
			Reset();
		}
	}

	void SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen );

	void AppendString( const wchar_t* pszData, size_t nDataLen );		//!< バッファの最後にデータを追加する。nLengthは文字単位。成功すればtrue。メモリ確保に失敗したらfalseを返す。

	/*!
	 * @brief バッファの最後にデータを追加する
	 *
	 * @param pszData [in, opt] 追加する文字列
	 */
	void AppendString(_In_opt_z_ LPCWSTR pszData)
	{
		if (pszData) {
			std::wstring_view text(pszData);
			AppendString(text.data(), text.length());
		}
		else {
			// 何もしない
		}
	}

	/*!
	 * バッファの最後にデータを追加する (フォーマット機能付き)
	 *
	 * @param[in] format フォーマット書式文字列
	 * @param[in, opt] args 引数リスト
	 * @throws std::invalid_argument formatが無効値
	 * @throws std::bad_alloc メモリ確保に失敗
	 * @remark 不正なフォーマットを指定すると無効なパラメータ例外で即死します。
	 */
	template <typename... Args>
	void AppendStringF(
		std::wstring_view format,
		const Args&... args
	)
	{
		// 整形によって追加される文字数をカウント
		const int additional = cxx::_scprintf(format.data(), std::as_const(args)...);

		if (additional < 0) {
			const auto e = errno;
			throw std::domain_error(std::format("AppendStringF error. errno = {:d}", e));
		}

		if (additional <= 0) return;

		// 現在の文字列長を取得
		const auto currentLength = GetStringLength();

		// 現在の文字数 + 追加文字数が収まるようにバッファを拡張する
		const auto newCapacity = currentLength + additional;
		AllocStringBuffer(newCapacity);

		// 出力先を固定長バッファとして扱う
		auto buffer = std::span(&GetStringPtr()[currentLength], additional + 1);

		// 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
		const auto added = cxx::_sprintf_s(buffer, format.data(), std::as_const(args)...);

		if (added <= 0) return;

		// 文字列終端を再設定する
		_SetStringLength(currentLength + added);
	}

	//CNativeW
	void SetNativeData( const CNativeW& cNative );						//!< バッファの内容を置き換える
	void AppendNativeData( const CNativeW& cNative );					//!< バッファの最後にデータを追加する

	//演算子
	CNativeW  operator + (const CNativeW& rhs) const	{ return (CNativeW(*this) += rhs); }

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

	/*!
	 * @brief バッファの最後にデータを追加する
	 *
	 * @param rhs [in] 追加する文字列
	 * @return 自分自身への参照
	 */
	Me& operator += (const Me& rhs)
	{
		AppendNativeData(rhs);

		return *this;
	}

	/*!
	 * @brief バッファの最後にデータを追加する
	 *
	 * @tparam A [in] コピーする文字列の型（文字列参照に変換できる型）
	 * @param rhs [in] 追加する文字列
	 * @return 自分自身への参照
	 */
	template<basis::NullTerminatedStringConstructible<WCHAR> A>
	Me& operator += (const A & rhs)
	{
		// 入力元をNUL終端文字列とみなす
		const auto szText = cxx::NullTerminatedString{ rhs };

		// 入力元を文字列として扱う
		const auto text = static_cast<std::wstring_view>(szText);
		AppendString(text.data(), text.length());

		return *this;
	}

	/*!
	 * @brief バッファの最後に文字を追加する
	 *
	 * @param rhs [in] 追加する文字
	 * @return 自分自身への参照
	 */
	Me& operator += (const WCHAR& rhs)
	{
		AppendString(&rhs, 1);

		return *this;
	}

	/*!
	 * @brief 文字列に変換する演算子
	 *
	 * @return 文字列
	 */
	constexpr explicit operator std::wstring() const
	{
		return std::wstring{ str() };
	}

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * @return 文字列参照
	 */
	explicit operator std::wstring_view() const& noexcept
	{
		return str();
	}

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
