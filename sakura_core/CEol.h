//	$Id$
/************************************************************************

	CEOL.h
    End of Line��ʂ̊Ǘ�
	Copyright (C) 2000, genta

    UPDATE:
    CREATE: 2000/05/15  �V�K�쐬	genta

************************************************************************/
#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"
#include "debug.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CEOL
{
	static const char* gm_pszEolDataArr[EOL_TYPE_NUM];
	static const int gm_pnEolLenArr[EOL_TYPE_NUM];
	static const char* gm_pszEolNameArr[EOL_TYPE_NUM];
public:
	
	//	�ݒ�֐�
	void Init(void){
		m_enumEOLType = EOL_NONE;
	//	m_nEOLLen = 2; /* = CR+LF */
	}
	
	static enumEOLType GetEOLType( const char* pszData, int nDataLen );
	bool SetType( enumEOLType t);	//	Type�̐ݒ�
	void GetTypeFromString( const char* pszData, int nDataLen )
		{	SetType( GetEOLType( pszData, nDataLen )); }

	//	�ǂݏo���֐�
	enumEOLType GetType(void) const { return m_enumEOLType; }	//	���݂�Type���擾
	int GetLen(void) const
		{ return gm_pnEolLenArr[ m_enumEOLType ]; }	//	���݂�EOL�����擾
	const char* GetName(void) const
		{ return gm_pszEolNameArr[ m_enumEOLType ]; }	//	���݂�EOL�̖��̎擾
	const char* GetValue(void) const
		{ return gm_pszEolDataArr[ m_enumEOLType ]; }	//	���݂�EOL������擪�ւ̃|�C���^���擾

	//	���֐�����̂��߂�Overload
	bool operator==( enumEOLType t ) const { return GetType() == t; }
	bool operator!=( enumEOLType t ) const { return GetType() != t; }
	const CEOL& operator=( const CEOL& t )
		{ m_enumEOLType = t.m_enumEOLType; return *this; }
	operator enumEOLType(void) const { return GetType(); }
	
	//	constructor
	CEOL(){ Init(); }
	CEOL( enumEOLType t ){ SetType(t); }
	
private:
	enumEOLType		m_enumEOLType;	/* ���s�R�[�h�̎�� */
	//int			m_nEOLLen;		/* ���s�R�[�h�̒��� */
};

//	���֐�����̂��߂�Overload
// inline bool operator==(m_enumEOLType t, const CEOL& c ){ return c == t; }
// inline bool operator!=(m_enumEOLType t, const CEOL& c ){ return c != t; }

#endif
