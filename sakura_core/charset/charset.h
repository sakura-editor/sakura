#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字コードセット種別
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT 追加
SAKURA_CORE_API enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open))
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< 文字コード自動判別
	CODE_ERROR      = -1,			//!< エラー
	CODE_NONE       = -1,			//!< 未検出
	CODE_DEFAULT    = CODE_SJIS,	//!< デフォルトの文字コード
	/*
		- MS-CP50220 
			Unicode から cp50220 への変換時に、
			JIS X 0201 片仮名は JIS X 0208 の片仮名に置換される
		- MS-CP50221
			Unicode から cp50221 への変換時に、
			JIS X 0201 片仮名は、G0 集合への指示のエスケープシーケンス ESC ( I を用いてエンコードされる
		- MS-CP50222
			Unicode から cp50222 への変換時に、
			JIS X 0201 片仮名は、SO/SI を用いてエンコードされる
		
		参考
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           判定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.08.14 kobake 追加
//!有効な文字コードセットならtrue
inline bool IsValidCodeType(int code)
{
	return code>=0 && code<CODE_CODEMAX;
}

//2007.08.14 kobake 追加
//!有効な文字コードセットならtrue。ただし、SJISは除く(意図は不明)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code!=CODE_SJIS;
}

//2007.08.14 kobake 追加
//!ECodeType型で表せる値ならtrue
inline bool IsInECodeType(int code)
{
	return (code>=0 && code<CODE_CODEMAX) || code==CODE_ERROR || code==CODE_AUTODETECT;
}

inline bool IsConcreteCodeType(ECodeType eCodeType)
{
	return IsValidCodeType(eCodeType) && eCodeType != CODE_AUTODETECT;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypeName{
public:
	CCodeTypeName(ECodeType eCodeType) : m_eCodeType(eCodeType) { }
	LPCTSTR Normal() const;
	LPCTSTR Short() const;
	LPCTSTR Bracket() const;
private:
	ECodeType m_eCodeType;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      コンボボックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypesForCombobox{
public:
	int			GetCount() const;
	ECodeType	GetCode(int nIndex) const;
	LPCTSTR		GetName(int nIndex) const;
};
