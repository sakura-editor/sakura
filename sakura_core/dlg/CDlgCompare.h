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
	bool			m_bIsModified;
	TCHAR*			m_pszCompareLabel;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* ���E�ɕ��ׂĕ\�� */

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );	// �W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnSize( WPARAM wParam, LPARAM lParam );
	BOOL OnMinMaxInfo( LPARAM lParam );

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

private:
	POINT			m_ptDefaultSize;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCOMPARE_H_ */


