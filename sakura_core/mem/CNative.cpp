/*! @file */
#include "StdAfx.h"
#include "CNative.h"

//! 空っぽにする
void CNative::Clear()
{
	this->_GetMemory()->_SetRawLength(0);
}
