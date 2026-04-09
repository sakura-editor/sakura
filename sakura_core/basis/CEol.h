/*! @file
	@brief End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

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

/*!
 * 行終端子情報定義用構造体
 */
struct SEolDefinition
{
	EEolType			m_eEolType;
	WORD				m_wName;
	std::wstring		m_Data;

	constexpr SEolDefinition(
		EEolType eEolType,
		int wName,
		std::wstring_view data
	);

	const std::wstring&	GetVal() const noexcept;
	std::string			GetVal(ECodeType eCodeType) const;

	bool StartsWith(LPCWSTR pData, size_t nLen) const noexcept;
	bool StartsWith(LPCSTR pData, size_t nLen, ECodeType eCodeType) const;
};

constexpr auto EOL_TYPE_NUM = static_cast<size_t>(EEolType::code_max); // 8

/*!
	@brief 行末の改行コードを管理

	管理とは言ってもオブジェクト化することで安全に設定を行えたり関連情報の取得を
	オブジェクトに対するメソッドで行えるだけだが、グローバル変数への参照を
	クラス内部に閉じこめることができるのでそれなりに意味はあると思う。
*/
class CEol {
public:
	static constexpr bool IsNone(EEolType t) noexcept
	{
		return t == EEolType::none;
	}
	static constexpr bool IsValid(EEolType t) noexcept
	{
		return EEolType::none < t && t < EEolType::code_max;
	}
	static constexpr bool IsNoneOrValid(EEolType t) noexcept
	{
		return IsNone(t) || IsValid(t);
	}

	CEol() noexcept = default;

	constexpr explicit CEol(EEolType t) noexcept
	{
		SetType(t);
	}

	//取得
	const SEolDefinition& GetData() const noexcept;

	constexpr bool     IsNone() const noexcept	{ return IsNone(m_eEolType); }	//!< 行終端子がないかどうか
	constexpr bool     IsValid() const noexcept { return !IsNone(); }			//!< 行終端子があるかどうか
	constexpr EEolType GetType() const noexcept { return m_eEolType; }			//!< 現在のTypeを取得

	LPCWSTR			GetValue2() const noexcept;	//!< 現在のEOL文字列先頭へのポインタを取得
	CLogicInt		GetLen() const noexcept;	//!< 現在のEOL長を取得。文字単位。
	LPCWSTR			GetName() const;

	//比較
	constexpr bool operator == (EEolType rhs) const noexcept { return GetType() == rhs; }
	constexpr bool operator == (const CEol& rhs) const noexcept { return *this == static_cast<EEolType>(rhs); }

	//型変換
	constexpr explicit operator bool() const noexcept { return IsValid(); }
	constexpr explicit operator EEolType() const noexcept { return GetType(); }

	/*!
	 * 行末種別の設定。
	 * @param t 行終端子の種別
	 * @retval true 正常終了。設定が反映された。
	 * @retval false 異常終了。強制的にCRLFに設定。
	 */
	constexpr bool SetType( EEolType t ) noexcept
	{
		const auto ret = IsNoneOrValid(t);
		if (ret) {
			// 正しい値
			m_eEolType = t;
		} else {
			// 異常値
			m_eEolType = EEolType::cr_and_lf;
		}
		return ret;
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

private:
	EEolType m_eEolType = EEolType::none;	//!< 改行コードの種類
};

#endif /* SAKURA_CEOL_036E1E16_7462_46A4_8F59_51D8E171E657_H_ */
