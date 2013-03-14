/*!	@file
	@brief ����ݒ�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgPrintSetting;

#ifndef _CDLGPRINTSETTING_H_
#define _CDLGPRINTSETTING_H_

#include "dlg/CDialog.h"
#include "config/maxdata.h" // MAX_PRINTSETTINGARR
#include "print/CPrint.h" //PRINTSETTING

/*!	����ݒ�_�C�A���O

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class CDlgPrintSetting : public CDialog
{
public:
	/*
	||  Constructors
	*/

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, int*, PRINTSETTING* );	/* ���[�_���_�C�A���O�̕\�� */

	int				m_nCurrentPrintSetting;
	PRINTSETTING	m_PrintSettingArr[MAX_PRINTSETTINGARR];


protected:
	/*
	||  �����w���p�֐�
	*/
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy( void );
	BOOL OnNotify( WPARAM,  LPARAM );
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType( BOOL );	/* �ݒ�̃^�C�v���ς���� */
	void OnSpin( int , BOOL );	/* �X�s���R���g���[���̏��� */
	int DataCheckAndCrrect( int , int );	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	BOOL OnTimer( WPARAM );	/* �^�C�}�[���� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTSETTING_H_ */



