#include "stdafx.h"
#include "input.h"

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	Shift,Ctrl,Alt�L�[��Ԃ̎擾

	@retval nIdx Shift,Ctrl,Alt�L�[���
	@date 2004.10.10 �֐���
*/
int getCtrlKeyState(void)
{
	int nIdx = 0;

	/* Shift�L�[��������Ă���Ȃ� */
	if(GetKeyState_Shift()){
		nIdx |= _SHIFT;
	}
	/* Ctrl�L�[��������Ă���Ȃ� */
	if( GetKeyState_Control() ){
		nIdx |= _CTRL;
	}
	/* Alt�L�[��������Ă���Ȃ� */
	if( GetKeyState_Alt() ){
		nIdx |= _ALT;
	}

	return nIdx;
}
