#include "stdafx.h"
#include "CConvert.h"
#include "Funccode.h"
#include "CEOL.h"
#include "charcode.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkanaToHankana.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankanaToZenkana.h"
#include "CConvert_HankanaToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkana.h"
#include "CConvert_Trim.h"

/* 機能種別によるバッファの変換 */
void CConvertMediator::ConvMemory( CNativeW* pCMemory, int nFuncCode, int nTabWidth )
{
	switch( nFuncCode ){
	//文字種変換、整形
	case F_TOLOWER:					CConvert_ToLower().CallConvert(pCMemory);			break;	// 小文字
	case F_TOUPPER:					CConvert_ToUpper().CallConvert(pCMemory);			break;	// 大文字
	case F_TOHANKAKU:				CConvert_ToHankaku().CallConvert(pCMemory);			break;	// 全角→半角
	case F_TOHANKATA:				CConvert_ZenkanaToHankana().CallConvert(pCMemory);	break;	// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					CConvert_HaneisuToZeneisu().CallConvert(pCMemory);	break;	// 半角英数→全角英数
	case F_TOHANEI:					CConvert_ZeneisuToHaneisu().CallConvert(pCMemory);	break;	// 全角英数→半角英数
	case F_TOZENKAKUKATA:			CConvert_ToZenkana().CallConvert(pCMemory);			break;	// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			CConvert_ToZenhira().CallConvert(pCMemory);			break;	// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKAKUKATA:	CConvert_HankanaToZenkana().CallConvert(pCMemory);	break;	// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENKAKUHIRA:	CConvert_HankanaToZenhira().CallConvert(pCMemory);	break;	// 半角カタカナ→全角ひらがな
	//文字種変換、整形
	case F_TABTOSPACE:				CConvert_TabToSpace(nTabWidth).CallConvert(pCMemory);break;	// TAB→空白
	case F_SPACETOTAB:				CConvert_SpaceToTab(nTabWidth).CallConvert(pCMemory);break;	// 空白→TAB
	case F_LTRIM:					CConvert_Trim(true).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	case F_RTRIM:					CConvert_Trim(false).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	}
	return;
}

