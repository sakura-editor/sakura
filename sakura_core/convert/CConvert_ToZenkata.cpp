#include "StdAfx.h"
#include "CConvert_ToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!できる限り全角カタカナにする
bool CConvert_ToZenkata::DoConvert(CNativeW* pcData)
{
	//半角カナ→全角カナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	int nBufLen = 0;
	Convert_HankataToZenkata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);

	//全角ひら→全角カナ
	Convert_ZenhiraToZenkata(pBuf, nBufLen);

	//半角英数→全角英数
	Convert_HaneisuToZeneisu(pBuf, nBufLen);

	//設定
	pcData->SetString(pBuf, nBufLen);

	//バッファ解放
	delete[] pBuf;

	return true;
}
