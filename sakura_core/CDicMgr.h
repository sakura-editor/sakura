/*!	@file
	@brief CDicMgr�N���X��`

	@author Norio Nakatani
	@date	1998/11/05 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDicMgr;

#ifndef _CDICMGR_H_
#define _CDICMGR_H_

#include <windows.h>
#include "util/container.h"
#include "_main/global.h"

class CMemory;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDicMgr
{
public:
	/*
	||  Constructors
	*/
	CDicMgr();
	~CDicMgr();

	/*
	||  Attributes & Operations
	*/
//	BOOL Open( char* );
	static BOOL Search( const wchar_t*, const int, CNativeW**, CNativeW**, const TCHAR*, int * );	// 2006.04.10 fon (const int,CMemory**,int*)������ǉ�
	static int HokanSearch( const wchar_t* , BOOL, vector_ex<std::wstring>&, int, const TCHAR* );
//	BOOL Close( char* );


protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDICMGR_H_ */



