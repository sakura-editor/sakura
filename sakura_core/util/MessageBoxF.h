/*!	@file
	@brief MessageBox�p�֐�

	@author Norio Nakatani

	@date 2013/03/03 Uchi Debug1.h���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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

#ifndef SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_
#define SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake ���b�Z�[�W�{�b�N�X�̎g�p�̓f�o�b�O���Ɍ���Ȃ��̂ŁA�uDebug�`�v�Ƃ������O��p�~

//�e�L�X�g���`�@�\�t��MessageBox
int VMessageBoxF( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, va_list& v );
int MessageBoxF ( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ... );


//                ���[�U�p���b�Z�[�W�{�b�N�X                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�f�o�b�O�p���b�Z�[�W�{�b�N�X
#define MYMESSAGEBOX MessageBoxF

//��ʂ̌x����
#define DefaultBeep()   ::MessageBeep(MB_OK)

//�G���[�F�ԊۂɁu�~�v[OK]
int ErrorMessage   (HWND hwnd, LPCTSTR format, ...);
int TopErrorMessage(HWND hwnd, LPCTSTR format, ...);	//(TOPMOST)
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

//�x���F�O�p�Ɂu�I�v[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

//���F�ۂɁui�v[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

//�m�F�F�����o���́u�H�v [�͂�][������] �߂�l:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

//�O���F�����o���́u�H�v [�͂�][������][�L�����Z��]  �߂�l:ID_YES,ID_NO,ID_CANCEL
int Select3Message   (HWND hwnd, LPCTSTR format, ...);
int TopSelect3Message(HWND hwnd, LPCTSTR format, ...);

//���̑����b�Z�[�W�\���p�{�b�N�X[OK]
int OkMessage   (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage(HWND hwnd, LPCTSTR format, ...);

//�^�C�v�w�胁�b�Z�[�W�\���p�{�b�N�X
int CustomMessage   (HWND hwnd, UINT uType, LPCTSTR format, ...);
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);	//(TOPMOST)

//��҂ɋ����ė~�����G���[
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_ */



