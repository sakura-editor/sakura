#include "stdafx.h"
#include "debug/Debug2.h"

//2007.08.30 kobake �ǉ�

//!�f�o�b�O���b�Z�[�W�o��
void debug_output(const char* str, ...)
{
	static char buf[256];
	va_list mark;
	va_start(mark,str);
	tchar_vsprintf(buf,str,mark);
	va_end(mark);

	//�f�o�b�K�ɏo��
	OutputDebugStringA(buf);
}

//!�����I��
void debug_exit()
{
	MessageBox(NULL,_T("assert�Ƃ��Ɉ����|�������ۂ��ł�"),GSTR_APPNAME,MB_OK);
	exit(1);
}

void warning_point()
{
	int n;
	n=0; //���������Ƀu���[�N�|�C���g��݂��Ă����ƁA�C�Ӄ��[�j���O�Ńu���[�N�ł���
}
