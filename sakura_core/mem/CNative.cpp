/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CNative.h"

//! 空っぽにする
void CNative::Clear()
{
	this->_GetMemory()->_SetRawLength(0);
}
