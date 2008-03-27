#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字コードセット種別
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT 追加
SAKURA_CORE_API enum enumCodeType {
	CODE_SJIS,						//!< SJIS
	CODE_JIS,						//!< JIS
	CODE_EUC,						//!< EUC
	CODE_UNICODE,					//!< Unicode
	CODE_UTF8,						//!< UTF-8
	CODE_UTF7,						//!< UTF-7
	CODE_UNICODEBE,					//!< Unicode BigEndian
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< 文字コード自動判別
	CODE_ERROR      = -1,			//!< エラー
	CODE_NONE       = -1,			//!< 未検出
	CODE_DEFAULT    = CODE_SJIS,	//!< デフォルトの文字コード
};
typedef enumCodeType ECodeType;


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
