//	$Id$
/************************************************************************

	CDlgCompare.h
	Copyright (C) 1998-2000, Norio Nakatani

663552
************************************************************************/

class CDlgCompare;

#ifndef _CDLGCOMPARE_H_
#define _CDLGCOMPARE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgCompare : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCompare();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const char*, BOOL, char*, HWND* );	/* ���[�_���_�C�A���O�̕\�� */

	const char*		m_pszPath;
	BOOL			m_bIsModified;
	char*			m_pszComparePath;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* ���E�ɕ��ׂĕ\�� */
//	BOOL			m_bCompareAndTileHorz;/* ���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnBnClicked( int );

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCOMPARE_H_ */

/*[EOF]*/
