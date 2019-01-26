/*! @file */
#include "StdAfx.h"
#include "CNative.h"

/*
	バッファサイズを縮小して空にする。
*/
void CNative::shrink_to_empty()
{
	this->SetRawData("",0);
}
