//	$Id$
//
//	CAutoSave.h
//	Copyright (C) 2000, genta
//
//	�t�@�C���̎����ۑ�
//
//	�O���������I��Polling�����D
//
#ifndef __CAUTOSAVE_H_
#define __CAUTOSAVE_H_
#include <windows.h>
#include "global.h"

const int MSec2Min = 1000 * 60;

class SAKURA_CORE_API CPassiveTimer {
public:
	CPassiveTimer() : nInterval(1), bEnabled(false){ Reset(); }
	
	//	���ԊԊu
	void SetInterval(int m);

	int GetInterval(void) const {return nInterval / MSec2Min; }
	
	//	�^�C�}�[�̃��Z�b�g
	void Reset(void){ nLastTick = ::GetTickCount(); }
	
	//	�L���^�����̐ݒ�
	void Enable(bool flag);
	bool IsEnabled(void) const { return bEnabled; }
	
	//	����I�ɌĂяo�����
	bool CheckAction(void);

private:
	DWORD nLastTick;	//	�Ō�Ƀ`�F�b�N�����Ƃ��̎���
	int  nInterval;	//	Action�Ԋu (��)
	bool bEnabled;	//	�L�����ǂ���
};
#endif
