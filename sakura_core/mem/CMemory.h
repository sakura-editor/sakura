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

#pragma once

#include "_main/global.h"

/*! ファイル文字コードセット判別時の先読み最大サイズ */
#define CheckKanjiCode_MAXREADLENGTH 16384

//! メモリバッファクラス
class CMemory
{
	//コンストラクタ・デストラクタ
public:
	CMemory() noexcept;
	CMemory(const void* pData, int nDataLenBytes);
	CMemory(const CMemory& rhs);
	CMemory(CMemory&& other) noexcept;
	// デストラクタを仮想にすると仮想関数テーブルへのポインタを持つ為にインスタンスの容量が増えてしまうので仮想にしない
	// 仮想デストラクタでは無いので派生クラスでメンバー変数を追加しない事
	~CMemory();

	//インターフェース
public:
	void AllocBuffer(int nNewDataLen);                               //!< バッファサイズの調整。必要に応じて拡大する。
	void SetRawData( const void* pData, int nDataLen );    //!< バッファの内容を置き換える
	void SetRawData(const CMemory& pcmemData);                     //!< バッファの内容を置き換える
	void SetRawDataHoldBuffer( const void* pData, int nDataLen );    //!< バッファの内容を置き換える(バッファを保持)
	void SetRawDataHoldBuffer(const CMemory& pcmemData);                     //!< バッファの内容を置き換える(バッファを保持)
	void AppendRawData( const void* pData, int nDataLen ); //!< バッファの最後にデータを追加する
	void AppendRawData(const CMemory* pcmemData);                  //!< バッファの最後にデータを追加する
	void Clean(){ _Empty(); }
	void Clear(){ _Empty(); }

	inline const void* GetRawPtr() const{ return m_pRawData; } //!< データへのポインタを返す
	inline void* GetRawPtr(){ return m_pRawData; }             //!< データへのポインタを返す
	int GetRawLength() const { return m_nRawLen; }                //!<データ長を返す。バイト単位。

	// 演算子
	//! コピー代入演算子
	CMemory& operator = (const CMemory& rhs) {
		if (this != &rhs) {
			SetRawData(rhs);
		}
		return *this;
	}
	//! ムーブ代入演算子
	CMemory& operator = (CMemory&& rhs) noexcept {
		if (this != &rhs) {
			_Empty();
			swap(rhs);
		}
		return *this;
	}

	// 比較
	static int IsEqual(const CMemory& cmem1, const CMemory& cmem2);	/* 等しい内容か */

	// 変換関数
	static void SwapHLByte(char* pData, const int nDataLen); // 下記関数のstatic関数版
	void SwapHLByte();			// Byteを交換する
	bool SwabHLByte(const CMemory& mem); // Byteを交換する(コピー版)

protected:
	/*
	||  実装ヘルパ関数
	*/
	void _Empty( void ); //!< 解放する。m_pRawDataはNULLになる。
	void _AddData(const void* pData, int nDataLen);
public:
	void _AppendSz(const char* str);
	void _SetRawLength(int nLength);
	void swap( CMemory& left ) noexcept {
		std::swap( m_nDataBufSize, left.m_nDataBufSize );
		std::swap( m_pRawData, left.m_pRawData );
		std::swap( m_nRawLen, left.m_nRawLen );
	}
	int capacity() const { return m_nDataBufSize ? m_nDataBufSize - 2: 0; }

private: // 2002/2/10 aroka アクセス権変更
	/*
	|| メンバ変数
	*/
	char*	m_pRawData;		//バッファ
	int		m_nRawLen;		//データサイズ(m_nDataBufSize以内)。バイト単位。
	int		m_nDataBufSize;	//バッファサイズ。バイト単位。
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     inline関数の実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
