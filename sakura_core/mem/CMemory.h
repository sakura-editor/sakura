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

#ifndef _CMEMORY_H_
#define _CMEMORY_H_

/*! ファイル文字コードセット判別時の先読み最大サイズ */
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "_main/global.h"

//! メモリバッファクラス
class CMemory
{
	//コンストラクタ・デストラクタ
public:
	CMemory();
	CMemory(const CMemory& rhs);
	CMemory(const void* pData, int nDataLenBytes);
	virtual ~CMemory();
protected:
	void _init_members();

	//インターフェース
public:
	void AllocBuffer( int );                               //!< バッファサイズの調整。必要に応じて拡大する。
	void SetRawData( const void* pData, int nDataLen );    //!< バッファの内容を置き換える
	void SetRawData( const CMemory& );                     //!< バッファの内容を置き換える
	void SetRawDataHoldBuffer( const void* pData, int nDataLen );    //!< バッファの内容を置き換える(バッファを保持)
	void SetRawDataHoldBuffer( const CMemory& );                     //!< バッファの内容を置き換える(バッファを保持)
	void AppendRawData( const void* pData, int nDataLen ); //!< バッファの最後にデータを追加する
	void AppendRawData( const CMemory* );                  //!< バッファの最後にデータを追加する
	void Clean(){ _Empty(); }
	void Clear(){ _Empty(); }

	inline const void* GetRawPtr(int* pnLength) const;      //!< データへのポインタと長さ返す
	inline void* GetRawPtr(int* pnLength);                  //!< データへのポインタと長さ返す
	inline const void* GetRawPtr() const{ return m_pRawData; } //!< データへのポインタを返す
	inline void* GetRawPtr(){ return m_pRawData; }             //!< データへのポインタを返す
	int GetRawLength() const { return m_nRawLen; }                //!<データ長を返す。バイト単位。

	// 演算子
	const CMemory& operator=( const CMemory& );

	// 比較
	static int IsEqual( CMemory&, CMemory& );	/* 等しい内容か */

	// 変換関数
	static void SwapHLByte( char*, const int ); // 下記関数のstatic関数版
	void SwapHLByte();			// Byteを交換する
	bool SwabHLByte( const CMemory& ); // Byteを交換する(コピー版)


protected:
	/*
	||  実装ヘルパ関数
	*/
	void _Empty( void ); //!< 解放する。m_pRawDataはNULLになる。
	void _AddData( const void*, int );
public:
	void _AppendSz(const char* str);
	void _SetRawLength(int nLength);
	void swap( CMemory& left ){
		std::swap( m_nDataBufSize, left.m_nDataBufSize );
		std::swap( m_pRawData, left.m_pRawData );
		std::swap( m_nRawLen, left.m_nRawLen );
	}
	int capacity() const { return m_nDataBufSize ? m_nDataBufSize - 2: 0; }

#ifdef _DEBUG
protected:
	typedef char* PCHAR;
	PCHAR& _DebugGetPointerRef(){ return m_pRawData; } //デバッグ用。バッファポインタの参照を返す。
#endif

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
inline const void* CMemory::GetRawPtr(int* pnLength) const //!< データへのポインタと長さ返す
{
	if(pnLength) *pnLength = GetRawLength();
	return m_pRawData;
}
inline void* CMemory::GetRawPtr(int* pnLength) //!< データへのポインタと長さ返す
{
	if(pnLength) *pnLength = GetRawLength();
	return m_pRawData;
}

///////////////////////////////////////////////////////////////////////
#endif /* _CMEMORY_H_ */



