//	$Id$
/************************************************************************

	CWaitCursor.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CWaitCursor;

#ifndef _CWAITCURSOR_H_
#define _CWAITCURSOR_H_


#include <windows.h>


/*-----------------------------------------------------------------------
ÉNÉâÉXÇÃêÈåæ
-----------------------------------------------------------------------*/
class CWaitCursor
{
public:
	/*
	||  Constructors
	*/
	CWaitCursor( HWND );
	~CWaitCursor();
public:
	HCURSOR	m_hCursor;
	HCURSOR	m_hCursorOld;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CWAITCURSOR_H_ */

/*[EOF]*/
