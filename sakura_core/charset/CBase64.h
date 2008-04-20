#pragma once

class CBase64{
public:
	static uchar_t Base64_CharToVal( const uchar_t c )
	{
		return BASE64VAL[c];
	}

	// BASE64デコードする際に使うバイナリ値
	static const uchar_t BASE64VAL[];

public:
	//各種判定関数
	static bool IsBase64Char( const uchar_t );  // UTF-7 で使われる Modified BASE64 を判別
};


inline bool CBase64::IsBase64Char( const uchar_t c )
{
	return ( !(c & 0x80) && BASE64VAL[c] != 0xff );
}
