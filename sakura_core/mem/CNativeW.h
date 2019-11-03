﻿/*! @file */
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CNATIVEW_59D44E96_F966_471D_A399_73D86F939DDA9_H_
#define SAKURA_CNATIVEW_59D44E96_F966_471D_A399_73D86F939DDA9_H_

#include "CNative.h"
#include "basis/SakuraBasis.h"
#include "debug/Debug2.h" //assert

//! 文字列への参照を取得するインターフェース
class IStringRef{
public:
	virtual const wchar_t*	GetPtr()	const = 0;
	virtual int				GetLength()	const = 0;
};

//! 文字列への参照を保持するクラス
class CStringRef : public IStringRef{
public:
	CStringRef() : m_pData(NULL), m_nDataLen(0) { }
	CStringRef(const wchar_t* pData, int nDataLen) : m_pData(pData), m_nDataLen(nDataLen) { }
	const wchar_t*	GetPtr()		const{ return m_pData;    }
	int				GetLength()		const{ return m_nDataLen; }

	//########補助
	bool			IsValid()		const{ return m_pData!=NULL; }
	wchar_t			At(int nIndex)	const{ assert(nIndex>=0 && nIndex<m_nDataLen); return m_pData[nIndex]; }
private:
	const wchar_t*	m_pData;
	int				m_nDataLen;
};

//! UNICODE文字列管理クラス
class CNativeW : public CNative{
public:
	//コンストラクタ・デストラクタ
	CNativeW() noexcept;
	CNativeW( const CNativeW& rhs );
	CNativeW( CNativeW&& other ) noexcept;
	CNativeW( const wchar_t* pData, int nDataLen ); //!< nDataLenは文字単位。
	CNativeW( const wchar_t* pData);

	//管理
	void AllocStringBuffer( int nDataLen );                    //!< (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	//WCHAR
	void SetString( const wchar_t* pData, int nDataLen );      //!< バッファの内容を置き換える。nDataLenは文字単位。
	void SetString( const wchar_t* pszData );                  //!< バッファの内容を置き換える
	void SetStringHoldBuffer( const wchar_t* pData, int nDataLen );
	void AppendString( const wchar_t* pszData );               //!< バッファの最後にデータを追加する
	void AppendString( const wchar_t* pszData, int nLength );  //!< バッファの最後にデータを追加する。nLengthは文字単位。成功すればtrue。メモリ確保に失敗したらfalseを返す。
	void AppendStringF(const wchar_t* pszData, ...);           //!< バッファの最後にデータを追加する (フォーマット機能付き)

	//CNativeW
	void SetNativeData( const CNativeW& pcNative );            //!< バッファの内容を置き換える
	void AppendNativeData( const CNativeW& );                  //!< バッファの最後にデータを追加する

	//演算子
	CNativeW& operator = (const CNativeW& rhs)			{ CNative::operator=(rhs); return *this; }
	CNativeW& operator = (CNativeW&& rhs) noexcept		{ CNative::operator=(std::forward<CNativeW>(rhs)); return *this; }
	CNativeW& operator = (std::nullptr_t) noexcept		{ CNative::operator=(CNativeW()); return *this; }
	CNativeW& operator = (const wchar_t* rhs) {
		if (rhs == nullptr) return (*this = nullptr);
		SetString(rhs);
		return *this;
	}
	CNativeW& operator = (const wchar_t ch) {
		if (ch == L'\0') return (*this = nullptr);
		const wchar_t sz[]{ ch, 0 };
		return (*this = sz);
	}
	CNativeW& operator += (const wchar_t ch) {
		const wchar_t sz[]{ ch, 0 };
		return (*this += sz);
	}
	CNativeW& operator += (const CNativeW& rhs)			{ AppendNativeData(rhs); return *this; }
	CNativeW operator + (const CNativeW& rhs) const		{ CNativeW tmp(*this); return (tmp += rhs); }

	//ネイティブ取得インターフェース
	wchar_t operator[](int nIndex) const;                    //!< 任意位置の文字取得。nIndexは文字単位。
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
	void _SetStringLength(int nLength)
	{
		_GetMemory()->_SetRawLength(nLength*sizeof(wchar_t));
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
	void swap( CNativeW& left ){
		_GetMemory()->swap( *left._GetMemory() );
	}
	int capacity(){
		return _GetMemory()->capacity() / sizeof(wchar_t);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           判定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	//! 同一の文字列ならtrue
	static bool IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           変換                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	void Replace( const wchar_t* pszFrom, const wchar_t* pszTo );   //!< 文字列置換
	void Replace( const wchar_t* pszFrom, int nFromLen, const wchar_t* pszTo, int nToLen );   //!< 文字列置換

public:
	// -- -- staticインターフェース -- -- //
	static CLogicInt GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字がwchar_t何個分かを返す
	static CHabaXInt GetHabaOfChar( const wchar_t* pData, int nDataLen, int nIdx );
	static CKetaXInt GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字が半角何個分かを返す
	static const wchar_t* GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent ); //!< ポインタで示した文字の次にある文字の位置を返します
	static const wchar_t* GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent ); //!< ポインタで示した文字の直前にある文字の位置を返します

	static CKetaXInt GetKetaOfChar( const CStringRef& cStr, int nIdx ) //!< 指定した位置の文字が半角何個分かを返す
	{
		return GetKetaOfChar(cStr.GetPtr(), cStr.GetLength(), nIdx);
	}
	static CLayoutXInt GetColmOfChar( const wchar_t* pData, int nDataLen, int nIdx )
		{ return GetHabaOfChar(pData,nDataLen,nIdx);}
	static CLayoutXInt GetColmOfChar( const CStringRef& cStr, int nIdx )
		{ return GetHabaOfChar(cStr.GetPtr(), cStr.GetLength(), nIdx);}
};

// 派生クラスでメンバー追加禁止
static_assert(sizeof(CNativeW) == sizeof(CNative), "size check");

namespace std {
template <>
	inline void swap(CNativeW& n1, CNativeW& n2)
	{
		n1.swap(n2);
	}
}

#endif /* SAKURA_CNATIVEW_59D44E96_F966_471D_A399_73D86F939DDA9_H_ */
/*[EOF]*/
