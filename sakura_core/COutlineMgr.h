//	$Id$
/************************************************************************

	COutlineMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
class COutlineMgr;

#ifndef _COUTLINEMGR_H_
#define _COUTLINEMGR_H_

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class COutlineMgr
{
public:
	/*
	||  Constructors
	*/
	COutlineMgr();
	virtual ~COutlineMgr();

	/*
	||  Attributes & Operations
	*/
	CMemory	cmemOutlineName;
	
	
protected:
	virtual Analyzer( void ) = 0;
	


};

///////////////////////////////////////////////////////////////////////
#endif /* _COUTLINEMGR_H_ */

/*[EOF]*/
