//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
	@date 1998/04/14 �f�[�^�̍폜������
	@date 1999/12/20 �f�[�^�̒u��������
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayoutMgr;

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

//#include <windows.h>
#include "CLayout.h"
#include "CDocLineMgr.h"
#include "CMemory.h"
 //	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif


struct LayoutReplaceArg {
	int			nDelLineFrom;			/*!< �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
	int			nDelColmFrom;			/*!< �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
	int			nDelLineTo;				/*!< �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
	int			nDelColmTo;				/*!< �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
	CMemory*	pcmemDeleted;			/*!< �폜���ꂽ�f�[�^ */
	const char*	pInsData;				/*!< �}������f�[�^ */
	int			nInsDataLen;			/*!< �}������f�[�^�̒��� */

	int			nAddLineNum;			/*!< �ĕ`��q���g ���C�A�E�g�s�̑��� */
	int			nModLineFrom;			/*!< �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
	int			nModLineTo;			/*!< �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */

	int			nNewLine;				/*!< �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
	int			nNewPos;				/*!< �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */

	BOOL		bDispSSTRING;			/*!< �V���O���N�H�[�e�[�V�����������\������ */
	BOOL		bDispWSTRING;			/*!< �_�u���N�H�[�e�[�V�����������\������ */
	BOOL		bUndo;					/*!< Undo���삩�ǂ��� */
};


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//! �e�L�X�g�̃��C�A�E�g���Ǘ�
class SAKURA_CORE_API CLayoutMgr
{
public:
	/*
	||  Constructors
	*/
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CDocLineMgr* );
	/*
	||  �Q�ƌn
	*/
	int GetLineCount( void ) { return m_nLines; }	/* �S�����s����Ԃ� */
	const char* GetLineStr( int , int* );	/* �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� */
	const char* GetLineStr2( int , int*, const CLayout** );	/* �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� */
//	const CLayout* GetLineData( int );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	CLayout* Search( int );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	int WhereCurrentWord( int , int , int* , int* , int* , int*, CMemory*, CMemory* );	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	int PrevOrNextWord( int, int, int*, int*, int );	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	int SearchWord( int	, int , const char* , int , int , int , int , int* , int* , int* , int*, CBregexp* );	/* �P�ꌟ�� */
	void CaretPos_Phys2Log( int, int, int*, int* );
	void CaretPos_Log2Phys( int, int, int*, int* );
	void DUMP( void );	/* �e�X�g�p�Ƀ��C�A�E�g�����_���v */

	/*
	|| �X�V�n
	*/
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	void SetLayoutInfo( int , BOOL, int , char*, char*, char*, char*, char*, char*, char*, int, int, HWND, BOOL, BOOL ); /* ���C�A�E�g���̕ύX */	//Jun. 01, 2001 JEPRO char* (�s�R�����g�f���~�^3�p)��1�ǉ�
//#else
//	void SetLayoutInfo( int , BOOL, int , char*, char*, char*, char*, int, int, HWND, BOOL, BOOL ); /* ���C�A�E�g���̕ύX */
//#endif
	void DeleteData_CLayoutMgr( int , int , int, int *, int *, int *, CMemory&, BOOL, BOOL, BOOL );	/* �s�������폜 */
	void InsertData_CLayoutMgr( int, int, const char*, int, int*, int*, int*, int*, BOOL, BOOL, BOOL );	/* ������}�� */

/* ������u�� */
void CLayoutMgr::ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
#if 0
		int			nDelLineFrom,			/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
		int			nDelColmFrom,			/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
		int			nDelLineTo,				/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
		int			nDelColmTo,				/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
		CMemory*	pcmemDeleted,			/* �폜���ꂽ�f�[�^ */
		const char*	pInsData,				/* �}������f�[�^ */
		int			nInsDataLen,			/* �}������f�[�^�̒��� */

		int*		pnAddLineNum,			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
		int*		pnModLineFrom,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
		int*		pnModLineTo,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */

		int*		pnNewLine,				/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
		int*		pnNewPos,				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */

		BOOL		bDispSSTRING,			/* �V���O���N�H�[�e�[�V�����������\������ */
		BOOL		bDispWSTRING,			/* �_�u���N�H�[�e�[�V�����������\������ */
		BOOL		bUndo					/* Undo���삩�ǂ��� */
#endif
);



protected:
	/*
	||  �Q�ƌn
	*/
	const char* GetFirstLinrStr( int* );	/* ���A�N�Z�X���[�h�F�擪�s�𓾂� */
	const char* GetNextLinrStr( int* );	/* ���A�N�Z�X���[�h�F���̍s�𓾂� */
	void XYLogicalToLayout( CLayout*, int, int, int, int*, int* );	/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */


	/*
	|| �X�V�n
	*/
//	void SetMaxLineSize( int, int, int );	/* �܂�Ԃ��������̕ύX�^���C�A�E�g���č\�z */
	void DoLayout( HWND, BOOL,BOOL );	/* ���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂� */
//	void DoLayout( int, BOOL, HWND, BOOL, BOOL );	/* �V�����܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂� */
//	int DoLayout3( CLayout* , int, int, int );	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int DoLayout3_New( CLayout* , /*CLayout* ,*/ int, int, int, int, int*, BOOL, BOOL );	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	CLayout* DeleteLayoutAsLogical( CLayout*, int, int , int, int, int, int* );	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜 */
	void ShiftLogicalLineNum( CLayout* , int );	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */


	/*
	|| �����o�ϐ�
	*/
public:
	CDocLineMgr*	m_pcDocLineMgr;	/* �s�o�b�t�@�Ǘ��}�l�[�W�� */
protected:
	CLayout*		m_pLayoutTop;
	CLayout*		m_pLayoutBot;
//	CLayout*		m_pLayoutCurrent;			/* ���A�N�Z�X���̌��݈ʒu */
	int				m_nLines;					/* �S�����s�� */
	int				m_nMaxLineSize;				/* �܂�Ԃ������� */
	BOOL			m_bWordWrap;				/* �p�����[�h���b�v������ */
	int				m_nTabSpace;				/* TAB�����X�y�[�X */
	char*			m_pszLineComment;			/* �s�R�����g�f���~�^ */
	char*			m_pszLineComment2;			/* �s�R�����g�f���~�^2 */
	char*			m_pszLineComment3;			/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
	char*			m_pszBlockCommentFrom;		/* �u���b�N�R�����g�f���~�^(From) */
	char*			m_pszBlockCommentTo;		/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	char*			m_pszBlockCommentFrom2;		/* �u���b�N�R�����g�f���~�^2(From) */
	char*			m_pszBlockCommentTo2;		/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
	int				m_nStringType;				/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */

	int				m_nPrevReferLine;
	CLayout*		m_pLayoutPrevRefer;
	/*
	|| �����w���p�n
	*/
	CLayout* InsertLineNext( CLayout*, CDocLine*, /*const char*,*/ int, int, int, int, int );
	void AddLineBottom( CDocLine*, /*const char*,*/ int, int, int, int, int );
public:
	void Init();
	void Empty();

};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
