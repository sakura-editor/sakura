#include "stdafx.h"
#include "CConvert_HankanaToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角カナ→全角ひらがな
bool CConvert_HankanaToZenhira::DoConvert(CNativeW* pcData)
{
	//半角カナ→全角カナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	int nDstLen = 0;
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);

	//全角カナ→全角ひらがな
	Convert_ZenkanaToZenhira(pBuf, nDstLen);

	//設定
	pcData->SetString(pBuf, nDstLen);

	//バッファ解放
	delete[] pBuf;

	return true;
}
