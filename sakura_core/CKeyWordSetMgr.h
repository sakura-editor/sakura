//	$Id$
/*!	@file
	@brief �����L�[���[�h�Ǘ�

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CKeyWordSetMgr;

#ifndef _CKEYWORDSETMGR_H_
#define _CKEYWORDSETMGR_H_

#include <windows.h>
#include "global.h"// 2002/2/10 aroka

#define		MAX_SETNUM		20	//Jul. 12, 2001 jepro notes: �����L�[���[�h�̃Z�b�g���̍ő�l
#define		MAX_SETNAMELEN	32
#define		MAX_KEYWORDNUM	1000
#define		MAX_KEYWORDLEN	100

//! �����L�[���[�h�Ǘ�
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
	BOOL DelKeyWord( int , int );			/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
	int SearchKeyWord( int , const char*, int );	/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
	BOOL IsModify( CKeyWordSetMgr&, BOOL* pnModifyFlagArr );	/* �ύX�󋵂𒲍� */
	void SortKeyWord( int );						/* ���Ԗڂ̃Z�b�g�̃L�[���[�h���\�[�g���� */  //MIK
	int SearchKeyWord2( int , const char*, int );	/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���o�C�i���T�[�` �����Ƃ���-1��Ԃ� */	//MIK
	void SetKeyWordCase( int, int );				/* ���Ԗڂ̃Z�b�g�̑啶�����������f���Z�b�g���� */	//MIK
	int GetKeyWordCase( int );						/* ���Ԗڂ̃Z�b�g�̑啶�����������f���擾���� */			//MIK
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
	int		m_nKEYWORDCASEArr[MAX_SETNUM];	/*!< �L�[���[�h�̉p�啶����������� */
	int		m_nKeyWordNumArr[MAX_SETNUM];
	char	m_szKeyWordArr[MAX_SETNUM][MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];
	char	m_IsSorted[MAX_SETNUM];	/*!< �\�[�g�������ǂ����̃t���O */  //MIK
protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYWORDSETMGR_H_ */


/*[EOF]*/
