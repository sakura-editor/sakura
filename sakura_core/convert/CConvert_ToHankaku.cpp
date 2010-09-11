#include "stdafx.h"
#include "CConvert_ToHankaku.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角にできるものは全部半角に変換
bool CConvert_ToHankaku::DoConvert(CNativeW* pcData)
{
	//全角ひらがな→全角カタカナ
	Convert_ZenhiraToZenkana(pcData->GetStringPtr(), pcData->GetStringLength());

	//全角英数→半角英数
	Convert_ZenEisuu_To_HanEisuu(pcData->GetStringPtr(), pcData->GetStringLength());

	//全角カタカナ→半角カタカナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()*2+1]; //濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	int nDstLen = 0;
	Convert_ZenKana_To_HanKana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);
	delete[] pBuf;

	return true;
}
