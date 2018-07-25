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
#ifndef SAKURA_CHARSET_51A8CEE7_80CB_463F_975E_B30715B1C385_H_
#define SAKURA_CHARSET_51A8CEE7_80CB_463F_975E_B30715B1C385_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字コードセット種別
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT 追加
enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS)ではない)
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open)ではない)
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CESU8,						//!< CESU-8
	CODE_LATIN1,					//!< Latin1				(Latin1, 欧文, Windows-1252, Windows Codepage 1252 West European)
	CODE_CODEMAX,
	CODE_CPACP      = 90,
	CODE_CPOEM      = 91,
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
//	2010/6/21	inlineをはずす
bool IsValidCodeType(int code);

//2007.08.14 kobake 追加
//!有効な文字コードセットならtrue。ただし、SJISは除く(ファイル一覧に文字コードを[]付きで表示のため)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code!=CODE_SJIS;
}

// 2010/6/21 Uchi 削除
//2007.08.14 kobake 追加
//!ECodeType型で表せる値ならtrue
//inline bool IsInECodeType(int code)
//{
//	return (code>=0 && code<CODE_CODEMAX) || code==CODE_ERROR || code==CODE_AUTODETECT;
//}

// 2010/6/21 Uchi 削除
//inline bool IsConcreteCodeType(ECodeType eCodeType)
//{
//	return IsValidCodeType(eCodeType) && eCodeType != CODE_AUTODETECT;
//}
inline bool IsValidCodePageEx(int code)
{
	return 12000 == code
		|| 12001 == code
		|| ::IsValidCodePage(code);
}

void InitCodeSet();
inline bool IsValidCodeOrCPType(int code)
{
	return IsValidCodeType(code) || CODE_CPACP == code || CODE_CPOEM == code || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}
inline bool IsValidCodeOrCPTypeExceptSJIS(int code)
{
	return IsValidCodeTypeExceptSJIS(code) || CODE_CPACP == code || CODE_CPOEM == code || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypeName{
public:
	CCodeTypeName(ECodeType eCodeType) : m_eCodeType(eCodeType) { InitCodeSet(); }
	CCodeTypeName(int eCodeType) : m_eCodeType((ECodeType)eCodeType) { InitCodeSet(); }
	ECodeType GetCode() const { return m_eCodeType; }
	LPCTSTR	Normal() const;
	LPCTSTR	Short() const;
	LPCTSTR	Bracket() const;
	bool	UseBom();
	bool	CanDefault();
	bool	IsBomDefOn();
private:
	ECodeType m_eCodeType;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      コンボボックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypesForCombobox{
public:
	CCodeTypesForCombobox() { InitCodeSet(); }
	int			GetCount() const;
	ECodeType	GetCode(int nIndex) const;
	LPCTSTR		GetName(int nIndex) const;
};

#endif /* SAKURA_CHARSET_51A8CEE7_80CB_463F_975E_B30715B1C385_H_ */
/*[EOF]*/
