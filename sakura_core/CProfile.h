//	$Id$
/*!	@file
	@brief INIファイル入出力

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CProfile;
#ifndef _CPROFILE_H_
#define _CPROFILE_H_


#include <windows.h>
//#include "CMemory.h"
class CMemory; // 2002/2/10 aroka

#define MAX_SECNUM	64
#define MAX_SECDATA	1024

#ifndef REGCNV_INT2SZ
	#define REGCNV_INT2SZ	1
#endif
#ifndef REGCNV_SZ2SZ
	#define REGCNV_SZ2SZ	2
#endif
#ifndef REGCNV_CHAR2SZ
	#define	REGCNV_CHAR2SZ	3
#endif
#ifndef REGCNV_WORD2SZ
	#define REGCNV_WORD2SZ	4
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief INIファイル入出力
*/
class CProfile
{
public:
	CProfile();
	~CProfile();
	void Init( void );
	BOOL ReadProfile( const char* );
	BOOL WriteProfile( const char*, const char* );
	int SearchSection( const char*, int );
	int SearchDataName( int, const char*, int );
	int AddSection( const char*, int );
	BOOL AddSectionData( int, const char*, int, const char*, int );
	BOOL IOProfileData( BOOL, const char*, const char*, int, char*, int );
	void DUMP( void );
protected:
	char m_szProfileName[_MAX_PATH];
	int m_nSecNum;
	CMemory* m_pSecNameArr[MAX_SECNUM];
	int m_nSecDataNumArr[MAX_SECNUM];
	CMemory* m_pDataNameArr[MAX_SECNUM][MAX_SECDATA];
	CMemory* m_pDataArr[MAX_SECNUM][MAX_SECDATA];
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */


/*[EOF]*/
