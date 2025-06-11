/*! @file
	@brief End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CEOL_036E1E16_7462_46A4_8F59_51D8E171E657_H_
#define SAKURA_CEOL_036E1E16_7462_46A4_8F59_51D8E171E657_H_
#pragma once

#include "_main/global.h"
#include "basis/primitive.h"
#include "basis/SakuraBasis.h"

/*!
	行終端子の種類

	行末記号の種類を定義する。
	0より大きい値は、終端の種類に対応する。
	ファイル末尾の行では「終端がない状態」があり得る。
	ドキュメントの行末スタイルに合わせて自動付与を行うための値も定義しておく。

	@date 2002/09/22 Moca EOL_CRLF_UNICODEを廃止
	@date 2021/03/27 berryzplus 定数に意味のある名前を付ける
 */
enum class EEolType : char {
	none,					//!< 行終端子なし（改行コード変換では「変換しない」）
	cr_and_lf,				//!< \x0d\x0a 復帰改行
	line_feed,				//!< \x0a 改行
	carriage_return,		//!< \x0d 復帰
	next_line,				//!< \u0085 NEL
	line_separator,			//!< \u2028 LS
	paragraph_separator,	//!< \u2029 PS
	code_max,				//!< 範囲外検出用のマーカー(行終端子として使用しないこと)
};

struct SEolDefinition{
	const WCHAR*	m_szName;
	const WCHAR*	m_szDataW;
	const ACHAR*	m_szDataA;
	size_t			m_nLen;

	bool StartsWith(const WCHAR* pData, size_t nLen) const{ return m_nLen<=nLen && 0==wmemcmp(pData,m_szDataW,m_nLen); }
	bool StartsWith(const ACHAR* pData, size_t nLen) const{ return m_nLen<=nLen && m_szDataA[0] != '\0' && 0==memcmp(pData,m_szDataA,m_nLen); }
};

constexpr auto EOL_TYPE_NUM = static_cast<size_t>(EEolType::code_max); // 8

/*!
	@brief 行末の改行コードを管理

	管理とは言ってもオブジェクト化することで安全に設定を行えたり関連情報の取得を
	オブジェクトに対するメソッドで行えるだけだが、グローバル変数への参照を
	クラス内部に閉じこめることができるのでそれなりに意味はあると思う。
*/
class CEol {
	EEolType m_eEolType = EEolType::none;	//!< 改行コードの種類

public:
	static constexpr bool IsNone( EEolType t ) noexcept
	{
		return t == EEolType::none;
	}
	static constexpr bool IsValid( EEolType t ) noexcept
	{
		return EEolType::none < t && t < EEolType::code_max;
	}
	static constexpr bool IsNoneOrValid( EEolType t ) noexcept
	{
		return IsNone( t ) || IsValid( t );
	}

	constexpr explicit CEol( EEolType t ) noexcept
	{
		SetType( t );
	}
	CEol() noexcept = default;

	//取得
	[[nodiscard]] bool IsNone() const noexcept { return IsNone( m_eEolType ); }			//!< 行終端子がないかどうか
	[[nodiscard]] bool IsValid() const noexcept { return !IsNone(); }					//!< 行終端子があるかどうか
	[[nodiscard]] constexpr EEolType GetType() const noexcept { return m_eEolType; }	//!< 現在のTypeを取得
	[[nodiscard]] LPCWSTR	GetName() const noexcept;	//!< 現在のEOLの名称取得
	[[nodiscard]] LPCWSTR	GetValue2() const noexcept;	//!< 現在のEOL文字列先頭へのポインタを取得
	[[nodiscard]] CLogicInt	GetLen() const noexcept;	//!< 現在のEOL長を取得。文字単位。

	//比較
	[[nodiscard]] constexpr bool operator == ( EEolType t ) const noexcept { return GetType() == t; }
	[[nodiscard]] constexpr bool operator != ( EEolType t ) const noexcept { return !operator == ( t ); }

	//型変換
	[[nodiscard]] constexpr explicit operator EEolType() const { return GetType(); }

	/*!
		行末種別の設定。
		@param t 行終端子の種別
		@retval true 正常終了。設定が反映された。
		@retval false 異常終了。強制的にCRLFに設定。
	 */
	constexpr bool SetType( EEolType t ) noexcept
	{
		if( IsNoneOrValid( t ) ){
			// 正しい値
			m_eEolType = t;
			return true;
		}else{
			// 異常値
			m_eEolType = EEolType::cr_and_lf;
			return false;
		}
	}

	//代入演算子
	CEol& operator = ( EEolType t ) noexcept { SetType( t ); return *this; }

	//文字列内の行終端子を解析
	void SetTypeByString( const wchar_t* pszData, size_t nDataLen );
	void SetTypeByString( const char* pszData, size_t nDataLen );

	//設定（ファイル読み込み時に使用）
	void SetTypeByStringForFile( const char* pszData, size_t nDataLen ){ SetTypeByString( pszData, nDataLen ); }
	void SetTypeByStringForFile_uni( const char* pszData, size_t nDataLen );
	void SetTypeByStringForFile_unibe( const char* pszData, size_t nDataLen );
};

// グローバル演算子
bool operator == ( const CEol& lhs, const CEol& rhs ) noexcept;
bool operator != ( const CEol& lhs, const CEol& rhs ) noexcept;
bool operator == ( EEolType lhs, const CEol& rhs ) noexcept;
bool operator != ( EEolType lhs, const CEol& rhs ) noexcept;

#endif /* SAKURA_CEOL_036E1E16_7462_46A4_8F59_51D8E171E657_H_ */
