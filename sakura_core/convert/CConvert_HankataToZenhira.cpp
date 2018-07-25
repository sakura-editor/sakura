#include "StdAfx.h"
#include "CConvert_HankataToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角カナ→全角ひらがな
bool CConvert_HankataToZenhira::DoConvert(CNativeW* pcData)
{
	//半角カナ→全角ひらがな
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	int nDstLen = 0;
	Convert_HankataToZenhira(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);
	delete[] pBuf;

	return true;
}
