#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �����R�[�h�Z�b�g���
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT �ǉ�
SAKURA_CORE_API enum enumCodeType {
	CODE_SJIS,						//!< SJIS
	CODE_JIS,						//!< JIS
	CODE_EUC,						//!< EUC
	CODE_UNICODE,					//!< Unicode
	CODE_UTF8,						//!< UTF-8
	CODE_UTF7,						//!< UTF-7
	CODE_UNICODEBE,					//!< Unicode BigEndian
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< �����R�[�h��������
	CODE_ERROR      = -1,			//!< �G���[
	CODE_NONE       = -1,			//!< �����o
	CODE_DEFAULT    = CODE_SJIS,	//!< �f�t�H���g�̕����R�[�h
};
typedef enumCodeType ECodeType;


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
