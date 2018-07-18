#include "StdAfx.h"
#include "CConvert_ToHankaku.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角にできるものは全部半角に変換
bool CConvert_ToHankaku::DoConvert(CNativeW* pcData)
{
	//全角→半角
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()*2+1]; //濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	int nDstLen = 0;
	Convert_ToHankaku(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);
	delete[] pBuf;

	return true;
}
