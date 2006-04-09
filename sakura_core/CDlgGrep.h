/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998.09/07  �V�K�쐬
	@date 1999.12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgGrep;

#ifndef _CDLGGREP_H_
#define _CDLGGREP_H_

#include "CDialog.h"

//! GREP�_�C�A���O�{�b�N�X
class SAKURA_CORE_API CDlgGrep : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgGrep();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, const char* );	/* ���[�_���_�C�A���O�̕\�� */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* ���[�h���X�_�C�A���O�̕\�� */


	BOOL		m_bSubFolder;/*!< �T�u�t�H���_������������� */
	BOOL		m_bFromThisText;/*!< ���̕ҏW���̃e�L�X�g���猟������ */
	int			m_bLoHiCase;	/*!< �p�啶���Ə���������ʂ��� */
	int			m_bRegularExp;	/*!< ���K�\�� */
	BOOL		m_bGrepOutputLine;	/*!< �s���o�͂��邩�Y�����������o�͂��邩 */

	//2001/06/23 N.Nakatani add
	BOOL		m_bWordOnly;	/*!< �P��P�ʂŌ��� */
	// 2002/09/22 Moca Add
	int			m_nGrepCharSet;		/*!< �����R�[�h�Z�b�g */

	int			m_nGrepOutputStyle;				/*!< Grep: �o�͌`�� */
	char		m_szText[_MAX_PATH + 1];	/*!< ���������� */
	char		m_szFile[_MAX_PATH + 1];	/*!< �����t�@�C�� */
	char		m_szFolder[_MAX_PATH + 1];	/*!< �����t�H���_ */
	char		m_szCurrentFilePath[_MAX_PATH + 1];
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGGREP_H_ */


/*[EOF]*/
