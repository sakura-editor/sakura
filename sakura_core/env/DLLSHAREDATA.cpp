#include "StdAfx.h"
#include "DLLSHAREDATA.h"

//GetDllShareData�p�O���[�o���ϐ�
DLLSHAREDATA* g_theDLLSHAREDATA = NULL;

//DLLSHAREDATA�ւ̊ȈՃA�N�Z�T
DLLSHAREDATA& GetDllShareData()
{
	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

//DLLSHAREDATA���m�ۂ�����A�܂�������ĂԁB
void DLLSHAREDATA::OnInit()
{
	g_theDLLSHAREDATA = this;
}
