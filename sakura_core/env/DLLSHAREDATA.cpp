#include "StdAfx.h"
#include "DLLSHAREDATA.h"

//GetDllShareData用グローバル変数
DLLSHAREDATA* g_theDLLSHAREDATA = NULL;

//DLLSHAREDATAへの簡易アクセサ
DLLSHAREDATA& GetDllShareData()
{
	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

//DLLSHAREDATAを確保したら、まずこれを呼ぶ。
void DLLSHAREDATA::OnInit()
{
	g_theDLLSHAREDATA = this;
}
