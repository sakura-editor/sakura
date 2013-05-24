/*!	@file
	@brief ����ݒ�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

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
#ifndef SAKURA_CDLGPRINTSETTING_9DF803C0_8BBB_41EC_B6A7_AFEBBDBC517D_H_
#define SAKURA_CDLGPRINTSETTING_9DF803C0_8BBB_41EC_B6A7_AFEBBDBC517D_H_

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
	int DoModal( HINSTANCE, HWND, int*, PRINTSETTING*, int );	/* ���[�_���_�C�A���O�̕\�� */

private:
	int				m_nCurrentPrintSetting;
	PRINTSETTING	m_PrintSettingArr[MAX_PRINTSETTINGARR];
	int				m_nLineNumberColmns;					// �s�ԍ��\������ꍇ�̌���
	bool			m_bPrintableLinesAndColumnInvalid;
	HFONT			m_hFontDlg;								// �_�C�A���O�̃t�H���g�n���h��
	int				m_nFontHeight;							// �_�C�A���O�̃t�H���g�̃T�C�Y

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
	BOOL OnStnClicked( int );
	BOOL OnEnChange( HWND hwndCtl, int wID );
	BOOL OnEnKillFocus( HWND hwndCtl, int wID );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType( BOOL );	/* �ݒ�̃^�C�v���ς���� */
	void OnSpin( int , BOOL );	/* �X�s���R���g���[���̏��� */
	int DataCheckAndCorrect( int , int );	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	BOOL CalcPrintableLineAndColumn();	/* �s���ƌ������v�Z */
	void UpdatePrintableLineAndColumn();	/* �s���ƌ����̌v�Z�v�� */
	void SetFontName( int idTxt, int idUse, LOGFONT& lf, int nPointSize );	// �t�H���g��/�g�p�{�^���̐ݒ�
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGPRINTSETTING_9DF803C0_8BBB_41EC_B6A7_AFEBBDBC517D_H_ */
