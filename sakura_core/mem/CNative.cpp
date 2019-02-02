/*! @file */
#include "StdAfx.h"
#include "CNative.h"

//! 基底の CMemory を明示的に初期化して最低限のメモリ確保させる
CNative::CNative( void )
	: CMemory( NULL, 0 )
{

}

//! 空っぽにする
void CNative::Clear()
{
	this->_GetMemory()->_SetRawLength(0);
}
