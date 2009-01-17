#include "stdafx.h"
#include "CConvert_ToZenkana.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!できる限り全角カタカナにする
bool CConvert_ToZenkana::DoConvert(CNativeW* pcData)
{
	//半角カナ→全角カナ
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //文字数が減ることはあっても増えることは無いので、これでＯＫ
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf);

	//全角ひら→全角カナ
	Convert_ZenhiraToZenkana(pBuf, wcslen(pBuf));

	//半角英数→全角英数
	Convert_HanEisuu_To_ZenEisuu(pBuf, wcslen(pBuf));

	//設定
	pcData->SetString(pBuf);

	//バッファ解放
	delete[] pBuf;

	return true;
}
