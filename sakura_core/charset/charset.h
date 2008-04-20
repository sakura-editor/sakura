#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �����R�[�h�Z�b�g���
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT �ǉ�
SAKURA_CORE_API enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open))
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
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
inline bool IsValidCodeType(int code)
{
	return code>=0 && code<CODE_CODEMAX;
}

//2007.08.14 kobake �ǉ�
//!�L���ȕ����R�[�h�Z�b�g�Ȃ�true�B�������ASJIS�͏���(�Ӑ}�͕s��)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code!=CODE_SJIS;
}

//2007.08.14 kobake �ǉ�
//!ECodeType�^�ŕ\����l�Ȃ�true
inline bool IsInECodeType(int code)
{
	return (code>=0 && code<CODE_CODEMAX) || code==CODE_ERROR || code==CODE_AUTODETECT;
}

inline bool IsConcreteCodeType(ECodeType eCodeType)
{
	return IsValidCodeType(eCodeType) && eCodeType != CODE_AUTODETECT;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
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
//                      �R���{�{�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CCodeTypesForCombobox{
public:
	int			GetCount() const;
	ECodeType	GetCode(int nIndex) const;
	LPCTSTR		GetName(int nIndex) const;
};
