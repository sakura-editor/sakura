//	$Id$
/*!	@file
	GREP�_�C�A���O�{�b�N�X
	
	@author Norio Nakatani
    @date 1998.09/07  �V�K�쐬
    @date 1999.12/05 �č쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CDlgGrep;

#ifndef _CDLGGREP_H_
#define _CDLGGREP_H_

#include "CDialog.h"
//#include <windows.h>
//#include "CShareData.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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


	BOOL		m_bSubFolder;/* �T�u�t�H���_������������� */
	BOOL		m_bFromThisText;/* ���̕ҏW���̃e�L�X�g���猟������ */
	int			m_bLoHiCase;	/* �p�啶���Ə���������ʂ��� */
	int			m_bRegularExp;	/* ���K�\�� */
	BOOL		m_bKanjiCode_AutoDetect;	/* �����R�[�h�������� */
	BOOL		m_bGrepOutputLine;	/* �s���o�͂��邩�Y�����������o�͂��邩 */
	int			m_nGrepOutputStyle;				/* Grep: �o�͌`�� */
	char		m_szText[_MAX_PATH + 1];	/* ���������� */
	char		m_szFile[_MAX_PATH + 1];	/* �����t�@�C�� */
	char		m_szFolder[_MAX_PATH + 1];	/* �����t�H���_ */
	char		m_szCurrentFilePath[_MAX_PATH + 1];
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGGREP_H_ */

/*[EOF]*/
