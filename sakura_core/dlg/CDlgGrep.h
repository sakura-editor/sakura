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

#include "dlg/CDialog.h"
#include "recent/CRecent.h"
#include "util/window.h"

//! GREP�_�C�A���O�{�b�N�X
class CDlgGrep : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgGrep();
	/*
	||  Attributes & Operations
	*/
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	int DoModal( HINSTANCE, HWND, const TCHAR* );	/* ���[�_���_�C�A���O�̕\�� */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* ���[�h���X�_�C�A���O�̕\�� */


	BOOL		m_bSubFolder;/*!< �T�u�t�H���_������������� */
	BOOL		m_bFromThisText;/*!< ���̕ҏW���̃e�L�X�g���猟������ */

	SSearchOption	m_sSearchOption;	//!< �����I�v�V����

	ECodeType	m_nGrepCharSet;			/*!< �����R�[�h�Z�b�g */
	int			m_nGrepOutputStyle;		/*!< Grep: �o�͌`�� */
	BOOL		m_bGrepOutputLine;		/*!< �s���o�͂��邩�Y�����������o�͂��邩 */
	bool		m_bGrepOutputFileOnly;		/*!< �t�@�C�����ŏ��̂݌��� */
	bool		m_bGrepOutputBaseFolder;	/*!< �x�[�X�t�H���_�\�� */
	bool		m_bGrepSeparateFolder;		/*!< �t�H���_���ɕ\�� */


	std::wstring	m_strText;				/*!< ���������� */
	bool			m_bSetText;				//!< �����������ݒ肵����
	SFilePath	m_szFile;					/*!< �����t�@�C�� */
	SFilePath	m_szFolder;					/*!< �����t�H���_ */
	SFilePath	m_szCurrentFilePath;
protected:
	SComboBoxItemDeleter	m_comboDelText;
	CRecentSearch			m_cRecentSearch;
	SComboBoxItemDeleter	m_comboDelFile;
	CRecentGrepFile			m_cRecentGrepFile;
	SComboBoxItemDeleter	m_comboDelFolder;
	CRecentGrepFolder		m_cRecentGrepFolder;
	CFontAutoDeleter		m_cFontText;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetDataFromThisText( bool );	/* ���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGGREP_H_ */



