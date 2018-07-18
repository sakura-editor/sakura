#include "StdAfx.h"
#include "CConvert_HankataToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角カナ→全角カナ
bool CConvert_HankataToZenkata::DoConvert(CNativeW* pcData)
{
	//半角カナ→全角カナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	int nDstLen = 0;
	Convert_HankataToZenkata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);
	delete[] pBuf;

	return true;
}
