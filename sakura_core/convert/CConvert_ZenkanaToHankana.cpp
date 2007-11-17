#include "stdafx.h"
#include "CConvert_ZenkanaToHankana.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!全角カナ→半角カナ
bool CConvert_ZenkanaToHankana::DoConvert(CNativeW* pcData)
{
	//全角カタカナ→半角カタカナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()*2+1]; //濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	Convert_ZenKana_To_HanKana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf);
	pcData->SetString(pBuf);
	delete[] pBuf;

	return true;
}
