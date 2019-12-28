/*! @file */
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
#pragma once

#include "util/StaticType.h"
#include "CProfile.h"

/*!
 * 独自バッファ参照型
 *
 * 固定長の文字配列を標準stringのように扱うためのクラス
 */
class StringBufferW {
	wchar_t*	m_pData;			//!< 文字列バッファを指すポインタ
	uint32_t	m_cchDataLength;	//!< 有効文字列長
	uint32_t	m_cchDataCount;		//!< 文字列バッファのサイズ

public:
	/*!
	 * コンストラクタ
	 *
	 * StringBufferWのインスタンスを構築する
	 *
	 * @param [in] pszData 文字列バッファを指すポインタ
	 * @param [in] cchDataCount 文字列バッファの確保サイズ
	 */
	StringBufferW( wchar_t* pszData, size_t cchDataCount ) noexcept
		: m_pData( pszData )
		, m_cchDataLength( 0 )
		, m_cchDataCount( cchDataCount )
	{
		// 文字列ポインタとサイズが有効な場合、有効文字列長を求める
		if( m_pData != NULL && m_cchDataCount > 0 ) {
			m_cchDataLength = ::wcsnlen( m_pData, m_cchDataCount );
			// NUL終端がなかったら、強制的にNUL終端する
			if( m_cchDataLength == m_cchDataCount ){
				m_pData[--m_cchDataLength] = L'\0';
			}
		}
	}

	/*!
	 * テンプレートコンストラクタ
	 *
	 * StringBufferWのインスタンスを構築する
	 *
	 * @param [in] szData 固定長文字配列への参照
	 */
	template <uint32_t cchDataCount>
	StringBufferW( wchar_t( &szData )[cchDataCount] ) noexcept
		: StringBufferW( szData, cchDataCount )
	{
	}

	// このクラスはコピー禁止
	StringBufferW( const StringBufferW& ) = delete;
	StringBufferW& operator = ( const StringBufferW& ) = delete;

	/*!
	 * ムーブコンストラクタ
	 *
	 * 空のStringBufferWインスタンスを構築し、
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	StringBufferW( StringBufferW&& other ) noexcept
		: StringBufferW( NULL, 0 )
	{
		*this = std::forward<StringBufferW>( other );
	}

	/*!
	 * ムーブ代入演算子
	 *
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	StringBufferW& operator = ( StringBufferW&& rhs ) noexcept
	{
		std::swap( m_pData, rhs.m_pData );
		std::swap( m_cchDataLength, rhs.m_cchDataLength );
		std::swap( m_cchDataCount, rhs.m_cchDataCount );
		return *this;
	}

	const wchar_t* c_str() const noexcept { return m_pData; }		//!< 文字列ポインタ(C-Style)
	size_t length() const noexcept { return m_cchDataLength; }		//!< 有効文字列長
	size_t capacity() const noexcept { return m_cchDataCount; }		//!< 文字列バッファのサイズ

	/*!
	 * バッファの内容を指定した文字列で置き替える
	 *
	 * @param [in] pSrc 文字列(NULL指定不可)
	 */
	void assign( const wchar_t* pSrc )
	{
		::wcsncpy_s( m_pData, capacity(), pSrc, _TRUNCATE );
		m_cchDataLength = ::wcsnlen( c_str(), capacity() );
	}

	StringBufferW& operator = ( const wchar_t* rhs ) { assign( rhs ); return *this; }
};

//2007.09.24 kobake データ変換部を子クラスに分離
//!各種データ変換付きCProfile
class CDataProfile : public CProfile{
private:
	//専用型
	typedef std::wstring wstring;

protected:
	static const wchar_t* _work_itow(int n)
	{
		static wchar_t buf[32];
		_itow(n,buf,10);
		return buf;
	}
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       データ変換部                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	//bool
	void profile_to_value(const wstring& profile, bool* value)
	{
		if(profile != L"0")*value=true;
		else *value=false;
	}
	void value_to_profile(const bool& value, wstring* profile)
	{
		*profile = _work_itow(value?1:0);
	}
	//int
	void profile_to_value(const wstring& profile, int* value)
	{
		*value = _wtoi(profile.c_str());
	}
	void value_to_profile(const int& value, wstring* profile)
	{
		*profile = _work_itow(value);
	}

	//int式入出力実装マクロ
	#define AS_INT(TYPE) \
		void profile_to_value(const wstring& profile, TYPE* value){ *value = (TYPE)_wtoi(profile.c_str()); } \
		void value_to_profile(const TYPE& value, wstring* profile){ *profile = _work_itow(value);    }

	//int式
// CType.hをincludeしないといけないから廃止
//	AS_INT(EOutlineType) 
	AS_INT(WORD)

#ifdef USE_STRICT_INT
	//CLayoutInt
	void profile_to_value(const wstring& profile, CLayoutInt* value){ *value = (CLayoutInt)_wtoi(profile.c_str()); }
	void value_to_profile(const CLayoutInt& value, wstring* profile){ *profile = _work_itow((Int)value);    }
	//CLogicInt
	void profile_to_value(const wstring& profile, CLogicInt* value){ *value = (CLogicInt)_wtoi(profile.c_str()); }
	void value_to_profile(const CLogicInt& value, wstring* profile){ *profile = _work_itow((Int)value);    }
#endif
	//ACHAR
	void profile_to_value(const wstring& profile, ACHAR* value)
	{
		if(profile.length()>0){
			ACHAR buf[2]={0};
			int ret=wctomb(buf,profile[0]);
			assert_warning(ret==1);
			(void)ret;
			*value = buf[0];
		}
		else{
			*value = '\0';
		}
	}
	void value_to_profile(const ACHAR& value, wstring* profile)
	{
		WCHAR buf[2]={0};
		mbtowc(buf,&value,1);
		profile->assign(1,buf[0]);
	}
	//WCHAR
	void profile_to_value(const wstring& profile, WCHAR* value)
	{
		*value = profile[0];
	}
	void value_to_profile(const WCHAR& value, wstring* profile)
	{
		profile->assign(1,value);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         入出力部                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 標準stringを介して設定値の入出力を行う。
	 */
	template <class T> //T=={bool, int, WORD, wchar_t, char}
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		T&						tEntryValue		//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			//文字列読み込み
			if( GetProfileDataImp( pszSectionName, pszEntryKey, buf ) ){
				//Tに変換
				profile_to_value(buf, &tEntryValue);
				ret = true;
			}
		}else{
			//文字列に変換
			value_to_profile(tEntryValue, &buf);
			//文字列書き込み
			ret = SetProfileDataImp( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	/*!
	 * 独自バッファ参照型の入出力(標準stringを介して入出力)
	 *
	 * 引数型を右辺値参照(StringBufferW&&)としたいために通常入出力と分離。
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めたが長すぎて切り捨てられた
	 * @retval false 設定値が存在しなかったため読込できなかった
	 */
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		StringBufferW&&			refEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			// 文字列読み込み
			if( GetProfileDataImp( pszSectionName, pszEntryKey, buf ) ){
				//StringBufferWに変換
				refEntryValue = buf.c_str();
				ret = buf.length() < refEntryValue.capacity();
			}
		}else{
			// 文字列に変換
			buf = refEntryValue.c_str();
			// 文字列書き込み
			ret = SetProfileDataImp( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力(標準stringを介して入出力)
	 *
	 * 型引数が合わないために通常入出力と分離。
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めたが長すぎて切り捨てられた
	 * @retval false 設定値が存在しなかったため読込できなかった
	 */
	template <int N>
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		StaticString<WCHAR, N>&	szEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			//文字列読み込み
			if( IOProfileData( pszSectionName, pszEntryKey, buf ) ){
				//StaticString<WCHAR, N>に変換
				szEntryValue = buf.c_str();
				ret = buf.length() < _countof2(szEntryValue);
			}
		}else{
			//文字列に変換
			buf = szEntryValue.GetBufferPointer();
			//文字列書き込み
			ret = IOProfileData( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	//2007.08.14 kobake 追加
	//! intを介して任意型の入出力を行う
	template <class T>
	bool IOProfileData_WrapInt( const WCHAR* pszSectionName, const WCHAR* pszEntryKey, T& nEntryValue)
	{
		int n=nEntryValue;
		bool ret=this->IOProfileData( pszSectionName, pszEntryKey, n );
		nEntryValue=(T)n;
		return ret;
	}
};

/*!
 * 文字列型(標準string)の入出力(無変換)
 */
template <> inline
bool CDataProfile::IOProfileData<std::wstring>(
	const WCHAR*			pszSectionName,	//!< [in] セクション名
	const WCHAR*			pszEntryKey,	//!< [in] エントリ名
	std::wstring&			strEntryValue	//!< [in,out] エントリ値
) noexcept
{
	bool ret = false;
	if( IsReadingMode() ){
		//文字列読み込み
		ret = GetProfileDataImp( pszSectionName, pszEntryKey, strEntryValue );
	}else{
		//文字列書き込み
		ret = SetProfileDataImp( pszSectionName, pszEntryKey, strEntryValue );
	}
	return ret;
}

/*[EOF]*/
