//	$Id$
/*!	@file
	@brief DIFF�����\���_�C�A���O�{�b�N�X

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2002, MIK

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

class CDlgDiff;

#ifndef _CDLGDIFF_H_
#define _CDLGDIFF_H_

#include "CDialog.h"
/*!
	@brief DIFF�����\���_�C�A���O�{�b�N�X
*/
class CDlgDiff : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgDiff();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const char*, BOOL );	/* ���[�_���_�C�A���O�̕\�� */

	char	m_szFile1[_MAX_PATH];	/* ���t�@�C�� */
	char	m_szFile2[_MAX_PATH];	/* ����t�@�C�� */
	BOOL	m_bIsModified;			/* ���t�@�C���X�V�� */
	BOOL	m_bIsModifiedDst;		/* ����t�@�C���X�V�� */
	int		m_nDiffFlgOpt;			/* DIFF�I�v�V���� */
	//int		m_nDiffFlgFile12;		/* �V���t�@�C�� */
	HWND	m_hWnd_Dst;				/* ����E�C���h�E�n���h�� */

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnBnClicked( int );
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnEditChange( HWND hwndCtl, int wID );
	LPVOID	GetHelpIdTable(void);

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

private:
	//	Feb. 28, 2004 genta �Ō�ɑI������Ă����ԍ���ۑ�����
	int m_nIndexSave;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGDIFF_H_ */

/*[EOF]*/
