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
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �����R�[�h�Z�b�g���
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT �ǉ�
SAKURA_CORE_API enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS)�ł͂Ȃ�)
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open)�ł͂Ȃ�)
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CESU8,						//!< CESU-8
	CODE_LATIN1,					//!< Latin1				(Latin1, ����, Windows-1252, Windows Codepage 1252 West European)
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< �����R�[�h��������
	CODE_ERROR      = -1,			//!< �G���[
	CODE_NONE       = -1,			//!< �����o
	CODE_DEFAULT    = CODE_SJIS,	//!< �f�t�H���g�̕����R�[�h
	/*
		- MS-CP50220 
			Unicode ���� cp50220 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����� JIS X 0208 �̕Љ����ɒu�������
		- MS-CP50221
			Unicode ���� cp50221 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́AG0 �W���ւ̎w���̃G�X�P�[�v�V�[�P���X ESC ( I ��p���ăG���R�[�h�����
		- MS-CP50222
			Unicode ���� cp50222 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́ASO/SI ��p���ăG���R�[�h�����
		
		�Q�l
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.08.14 kobake �ǉ�
//!�L���ȕ����R�[�h�Z�b�g�Ȃ�true
//	2010/6/21	inline���͂���
bool IsValidCodeType(int code);

//2007.08.14 kobake �ǉ�
//!�L���ȕ����R�[�h�Z�b�g�Ȃ�true�B�������ASJIS�͏���(�t�@�C���ꗗ�ɕ����R�[�h��[]�t���ŕ\���̂���)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code!=CODE_SJIS;
}

// 2010/6/21 Uchi �폜
//2007.08.14 kobake �ǉ�
//!ECodeType�^�ŕ\����l�Ȃ�true
//inline bool IsInECodeType(int code)
//{
//	return (code>=0 && code<CODE_CODEMAX) || code==CODE_ERROR || code==CODE_AUTODETECT;
//}

// 2010/6/21 Uchi �폜
//inline bool IsConcreteCodeType(ECodeType eCodeType)
//{
//	return IsValidCodeType(eCodeType) && eCodeType != CODE_AUTODETECT;
//}

void InitCodeSet();


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypeName{
public:
	CCodeTypeName(ECodeType eCodeType) : m_eCodeType(eCodeType) { InitCodeSet(); }
	CCodeTypeName(int eCodeType) : m_eCodeType((ECodeType)eCodeType) { InitCodeSet(); }
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
//                      �R���{�{�b�N�X                         //
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
