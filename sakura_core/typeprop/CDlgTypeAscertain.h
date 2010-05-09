/*!	@file
	@brief �^�C�v�ʐݒ�C���|�[�g�m�F�_�C�A���O

	@author Uchi
	@date 2010/4/17 �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

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

class CDlgTypeAscertain;

#ifndef _CDLGTYPEASCERTAIN_H_
#define _CDLGTYPEASCERTAIN_H_
using std::wstring;
using std::tstring;

#include "dlg/CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O
*/
class CDlgTypeAscertain : public CDialog
{
public:
	// �^
	struct SAscertainInfo {
		tstring	sImportFile;	//!< in �C���|�[�g�t�@�C����
		wstring	sTypeNameTo;	//!< in �^�C�v���i�C���|�[�g��j
		wstring	sTypeNameFile;	//!< in �^�C�v���i�t�@�C������j
		int 	nColorType;		//!< out �������(�J���[�R�s�[�p)
		wstring	sColorFile;		//!< out �F�ݒ�t�@�C����
	};

public:
	//  Constructors
	CDlgTypeAscertain();
	// ���[�_���_�C�A���O�̕\��
	int DoModal( HINSTANCE, HWND, SAscertainInfo* );	/* ���[�_���_�C�A���O�̕\�� */

protected:
	// �����w���p�֐�
	BOOL OnBnClicked( int );
	void SetData();	/* �_�C�A���O�f�[�^�̐ݒ� */
	LPVOID GetHelpIdTable(void);

private:
	SAscertainInfo* m_psi;			// �C���^�[�t�F�C�X
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPEASCERTAIN_H_ */



