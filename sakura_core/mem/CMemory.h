/*!	@file
	@brief メモリバッファクラス

	@author Norio Nakatani
	@date 1998/03/06 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, Moca, genta, aroka
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMEMORY_EE37AF3F_6B73_412E_8F0C_8A64F4250AE3_H_
#define SAKURA_CMEMORY_EE37AF3F_6B73_412E_8F0C_8A64F4250AE3_H_
#pragma once

#include <cstddef>
#include <string_view>

/*! ファイル文字コードセット判別時の先読み最大サイズ */
#define CheckKanjiCode_MAXREADLENGTH 16384

/*!
 * メモリバッファクラス
 *
 * ヒープメモリにバッファ領域を確保する
 * 内部バッファのサイズは8の倍数に切り上げて確保される。
 * NUL文字(\0)を含むバイナリシーケンスを格納することができる。
 *
 * * 元々は文字列バッファの変換機能を多く含むゴッドオブジェクトだった。
 * * UNICODE対応で文字列変換機能の一部が分離された。
 * * 残タスク _AppendSzメソッドの分離。
 * * 残タスク SwapHLByteメソッドの分離。
 *
 * @author Norio Nakatani
 * @date 1998/03/06 新規作成
 * @date 2007/11/06 kobake Unicode対応
 */
class CMemory
{
	//コンストラクタ・デストラクタ
public:
	CMemory() noexcept = default;
	CMemory( const void* pData, size_t nDataLen );
	CMemory( const CMemory& rhs );
	CMemory( CMemory&& other ) noexcept;

	// デストラクタを仮想にすると仮想関数テーブルへのポインタを持つ為にインスタンスの容量が増えてしまうので仮想にしない
	// 仮想デストラクタでは無いので派生クラスでメンバー変数を追加しない事
	~CMemory() noexcept;

	//インターフェース
public:
	void AllocBuffer( size_t nNewDataLen );								//!< バッファサイズの調整。必要に応じて拡大する。
	void SetRawData( const void* pData, size_t nDataLen );				//!< バッファの内容を置き換える
	void SetRawData( const CMemory& cmemData );							//!< バッファの内容を置き換える
	void SetRawDataHoldBuffer( const void* pData, size_t nDataLen );	//!< バッファの内容を置き換える(バッファを保持)
	void SetRawDataHoldBuffer( const CMemory& cmemData );				//!< バッファの内容を置き換える(バッファを保持)
	void AppendRawData( const void* pData, size_t nDataLen );			//!< バッファの最後にデータを追加する
	void Reset( void ) noexcept;										//!< バッファをリセットする

	template<typename T> auto get()       noexcept { return (T*)m_pRawData; } //!< データへのポインタを返す
	template<typename T> auto get() const noexcept { return (const T*)m_pRawData; } //!< データへのポインタを返す

	auto GetRawPtr()       noexcept { return m_pRawData; } //!< データへのポインタを返す
	auto GetRawPtr() const noexcept { return m_pRawData; } //!< データへのポインタを返す

	int GetRawLength() const noexcept { return Length<std::byte>(); }                //!<データ長を返す。バイト単位。

	template<typename T> auto length() const noexcept { return m_nRawLen / sizeof(T); }
	template<typename T> int  Length() const noexcept { return int(length<T>()); }

	// 演算子
	CMemory& operator = ( const CMemory& rhs );
	CMemory& operator = ( CMemory&& rhs ) noexcept;

	// 比較
	static bool IsEqual( const CMemory& cmem1, const CMemory& cmem2 );	/* 等しい内容か */

	// 変換関数
	static void SwapHLByte( char* pData, const size_t nDataLen ) noexcept; // 下記関数のstatic関数版
	void SwapHLByte() noexcept;		//!< データをWORD値の配列とみなして上下BYTEを交換する

	void _AppendSz( std::string_view str );
	void _SetRawLength( size_t nLength );
	void swap( CMemory& left ) noexcept;
	//! メモリ再確保を行わずに格納できる最大バイト数を求める
	int capacity() const noexcept { return 8 <= m_nDataBufSize ? m_nDataBufSize - 2: 0; }

private: // 2002/2/10 aroka アクセス権変更
	/*
	|| メンバ変数
	*/
	std::byte*	m_pRawData = nullptr;	//!< バッファ
	unsigned	m_nRawLen = 0;			//!< データサイズ(m_nDataBufSize未満)。バイト単位。
	unsigned	m_nDataBufSize = 0;		//!< バッファサイズ。バイト単位。
};

#endif /* SAKURA_CMEMORY_EE37AF3F_6B73_412E_8F0C_8A64F4250AE3_H_ */
