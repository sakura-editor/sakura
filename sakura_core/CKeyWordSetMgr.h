//	$Id$
/************************************************************************

	CKeyWordSetMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CKeyWordSetMgr;

#ifndef _CKEYWORDSETMGR_H_
#define _CKEYWORDSETMGR_H_

#include <windows.h>
#include "CMemory.h"

#define		MAX_SETNUM		20
#define		MAX_SETNAMELEN	32
#define		MAX_KEYWORDNUM	1000
#define		MAX_KEYWORDLEN	100

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CKeyWordSetMgr
{
public:
	/*
	||  Constructors
	*/
	CKeyWordSetMgr();
	~CKeyWordSetMgr();
	BOOL AddKeyWordSet( const char*, BOOL );	/* �Z�b�g�̒ǉ� */
	BOOL DelKeyWordSet( int  );	/* ���Ԗڂ̃Z�b�g���폜 */
	char* GetTypeName( int );	/* ���Ԗڂ̃Z�b�g����Ԃ� */
	int GetKeyWordNum( int );	/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
	char* GetKeyWord( int , int );	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
	char* UpdateKeyWord( int , int , const char* );	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW */
	BOOL AddKeyWord( int, const char* );	/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
	BOOL DelKeyWord( int , int );	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
	int SearchKeyWord( int , const char*, int );	/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
	BOOL IsModify( CKeyWordSetMgr&, BOOL* pnModifyFlagArr );	/* �ύX�󋵂𒲍� */
	/*
	|| ���Z�q
	*/
	const CKeyWordSetMgr& operator=( CKeyWordSetMgr& );
	/*
	||  Attributes & Operations
	*/
	int		m_nCurrentKeyWordSetIdx;
	int		m_nKeyWordSetNum;
	char	m_szSetNameArr[MAX_SETNUM][MAX_SETNAMELEN + 1];
    int		m_nKEYWORDCASEArr[MAX_SETNUM];	/* �L�[���[�h�̉p�啶����������� */
	int		m_nKeyWordNumArr[MAX_SETNUM];				
	char	m_szKeyWordArr[MAX_SETNUM][MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];				
protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYWORDSETMGR_H_ */

/*[EOF]*/
