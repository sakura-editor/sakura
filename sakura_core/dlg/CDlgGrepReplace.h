/*!	@file
	@brief GREP�u���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 2011.12.15 CDlgFrep.h����쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgGrep;

#ifndef SAKURA_CDLGGREP_REPLACE_H_
#define SAKURA_CDLGGREP_REPLACE_H_

#include "dlg/CDialog.h"
#include "dlg/CDlgGrep.h"

//! GREP�u���_�C�A���O�{�b�N�X
class CDlgGrepReplace : public CDlgGrep
{
public:
	/*
	||  Constructors
	*/
	CDlgGrepReplace();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, const TCHAR*, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */

	bool		m_bPaste;
	bool		m_bBackup;

	std::wstring	m_strText2;				//!< �u����
	int				m_nReplaceKeySequence;	//!< �u����V�[�P���X

protected:
	CFontAutoDeleter		m_cFontText2;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGGREP_REPLACE_H_ */



