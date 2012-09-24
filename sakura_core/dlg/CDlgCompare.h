/*!	@file
	@brief �t�@�C����r�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgCompare;

#ifndef _CDLGCOMPARE_H_
#define _CDLGCOMPARE_H_

#include "dlg/CDialog.h"
/*!
	@brief �t�@�C����r�_�C�A���O�{�b�N�X
*/
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
	int DoModal( HINSTANCE, HWND, LPARAM, const TCHAR*, bool, TCHAR*, HWND* );	/* ���[�_���_�C�A���O�̕\�� */

	const TCHAR*	m_pszPath;
	BOOL			m_bIsModified;
	TCHAR*			m_pszComparePath;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* ���E�ɕ��ׂĕ\�� */

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCOMPARE_H_ */


