// $Id$
/************************************************************************

	CDocLineMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/5  �V�K�쐬


************************************************************************/

class CDocLineMgr;

#ifndef _CDOCLINEMGR_H_
#define _CDOCLINEMGR_H_


#include <windows.h>
#include "CDocLine.h"
#include "CMemory.h"
#include "cRunningTimer.h"
#include "CJre.h"

struct DocLineReplaceArg {
	int			nDelLineFrom;			/* �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n) */
	int			nDelPosFrom;			/* �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
	int			nDelLineTo;			/* �폜�͈͍s�@To   ���s�P�ʂ̍s�ԍ� 0�J�n) */
	int			nDelPosTo;				/* �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
	CMemory*	pcmemDeleted;		/* �폜���ꂽ�f�[�^��ۑ� */
	int			nDeletedLineNum;	/* �폜�����s�̑��� */
	const char*	pInsData;			/* �}������f�[�^ */      
	int			nInsDataLen;		/* �}������f�[�^�̒��� */
	int			nInsLineNum;		/* �}���ɂ���đ������s�̐� */
	int			nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDocLineMgr
{
public:
	/*
	||  Constructors
	*/
	CDocLineMgr();
	~CDocLineMgr();

	/*
	||  �Q�ƌn
	*/
	char* GetAllData( int* );	/* �S�s�f�[�^��Ԃ� */
	int GetLineCount( void ) { return m_nLines; }	/* �S�s����Ԃ� */
	const char* GetLineStr( int , int* );
	const char* GetFirstLinrStr( int* );	/* ���A�N�Z�X���[�h�F�擪�s�𓾂� */
	const char* GetNextLinrStr( int* );	/* ���A�N�Z�X���[�h�F���̍s�𓾂� */
	int	WhereCurrentWord( int , int , int* , int*, CMemory*, CMemory* );	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	static int	WhatKindOfChar( char*, int, int );	/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
	int PrevOrNextWord( int , int , int* , int );	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	int SearchWord( int , int , const char* , int , int , int , int , int* , int* , int*, CJre* ); /* �P�ꌟ�� */
//	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int*, int ); /* �����񌟍� */
	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int ); /* �����񌟍� */
	static void CreateCharCharsArr( const unsigned char*, int, int** );	/* ���������̏�� */
//	static void CreateCharUsedArr( const unsigned char*, int, const int*, int** ); /* ���������̏��(�L�[������̎g�p�����\)�쐬 */


	
	void DUMP( void );
//	void ResetAllModifyFlag( BOOL );	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	void ResetAllModifyFlag( void );	/* �s�ύX��Ԃ����ׂă��Z�b�g */


	/*
	|| �X�V�n
	*/
	void Init();
	void Empty();
//	void InsertLineStr( int );	/* �w��s�̑O�ɒǉ����� */

#if 0
	void AddLineStrSz( const char* );	/* �����ɍs��ǉ� Ver0 */
	void AddLineStr( const char*, int );	/* �����ɍs��ǉ� Ver1 */
	void AddLineStr( CMemory& );	/* �����ɍs��ǉ� Ver2 */
#endif	
	//	May 15, 2000 genta
	void AddLineStrX( const char*, int, CEOL );	/* �����ɍs��ǉ� Ver1.5 */

	void DeleteData( int , int , int , int* , int* , int* , int*, CMemory&, int );	/* �f�[�^�̍폜 */

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	void CDocLineMgr::ReplaceData(
		DocLineReplaceArg*
#if 0
		int			nDelLineFrom,		/* �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n) */
		int			nDelPosFrom,		/* �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
		int			nDelLineTo,			/* �폜�͈͍s�@To   ���s�P�ʂ̍s�ԍ� 0�J�n) */
		int			nDelPosTo,			/* �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
		CMemory*	pcmemDeleted,		/* �폜���ꂽ�f�[�^��ۑ� */
		int*		pnDeletedLineNum,	/* �폜�����s�̑��� */
		const char*	pInsData,			/* �}������f�[�^ */      
		int			nInsDataLen,		/* �}������f�[�^�̒��� */
		int*		pnInsLineNum,		/* �}���ɂ���đ������s�̐� */
		int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		int*		pnNewPos			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
#endif
	);
	void DeleteNode( CDocLine* );/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
	void InsertNode( CDocLine*, CDocLine* );	/* �s�I�u�W�F�N�g�̑}���A���X�g�ύX�A�s��++ */

	
	void InsertData_CDocLineMgr( int , int , const char* , int , int* , int* , int*, int );	/* �f�[�^�̑}�� */
	//	Nov. 12, 2000 genta �����ǉ�
	int ReadFile( const char*, HWND, HWND, int, FILETIME*, int extraflag );
	//	Feb. 6, 2001 genta �����ǉ�(���s�R�[�h�ݒ�)
	int WriteFile( const char*, HWND, HWND, int, FILETIME*, CEOL );
	CDocLine* GetLineInfo( int );

	/*
	|| �����o�ϐ�
	*/
	CDocLine*	m_pDocLineTop;
	CDocLine*	m_pDocLineBot;
	CDocLine*	m_pDocLineCurrent;	/* ���A�N�Z�X���̌��݈ʒu */
	int			m_nLines;		/* �S�s�� */
	int			m_nPrevReferLine;
	CDocLine*	m_pCodePrevRefer;
protected:

	/*
	|| �����w���p�n
	*/
protected:



};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */

/*[EOF]*/
