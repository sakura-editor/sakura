// ���̍s�͕��������΍��p�ł��B
#include "StdAfx.h"
#include "CCesu8.h"



//! BOM�f�[�^�擾
void CCesu8::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	pcmemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}
