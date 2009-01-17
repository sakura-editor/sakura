#include "stdafx.h"
#include "CConvert_ToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!できる限り全角ひらがなにする
bool CConvert_ToZenhira::DoConvert(CNativeW* pcData)
{
	//半カナ→全角カナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf);

	//全カナ→全角ひらがな
	Convert_ZenkanaToZenhira(pBuf, wcslen(pBuf));

	//半角英数→全角英数
	Convert_HanEisuu_To_ZenEisuu(pBuf, wcslen(pBuf));

	//設定
	pcData->SetString(pBuf);

	//バッファ解放
	delete[] pBuf;

	return true;
}
