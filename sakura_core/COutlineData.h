//	$Id$
/************************************************************************

	COutlineData.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
class COutlineData;

#ifndef _COUTLINEDATA_H_
#define _COUTLINEDATA_H_

/*-----------------------------------------------------------------------
ÉNÉâÉXÇÃêÈåæ
-----------------------------------------------------------------------*/
class COutlineData
{
public:
	/*
	||  Constructors
	*/
	COutlineData();
	virtual ~COutlineData();

	/*
	||  Attributes & Operations
	*/
	COutlineData*	m_pPrev;
	COutlineData*	m_pNext;
	COutlineData*	m_pParent;
	COutlineData*	m_pChildFirst;
	COutlineData*	m_pChildLast;
protected:
	


};

///////////////////////////////////////////////////////////////////////
#endif /* _COUTLINEDATA_H_ */

/*[EOF]*/
