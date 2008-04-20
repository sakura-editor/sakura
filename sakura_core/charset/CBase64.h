#pragma once

class CBase64{
public:
	static uchar_t Base64_CharToVal( const uchar_t c )
	{
		return BASE64VAL[c];
	}

	// BASE64�f�R�[�h����ۂɎg���o�C�i���l
	static const uchar_t BASE64VAL[];

public:
	//�e�픻��֐�
	static bool IsBase64Char( const uchar_t );  // UTF-7 �Ŏg���� Modified BASE64 �𔻕�
};


inline bool CBase64::IsBase64Char( const uchar_t c )
{
	return ( !(c & 0x80) && BASE64VAL[c] != 0xff );
}
