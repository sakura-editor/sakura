//	$Id$
/*!	@file
	�A�E�g���C����̓f�[�^
	
	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
class COutlineData;

#ifndef _COUTLINEDATA_H_
#define _COUTLINEDATA_H_

/*-----------------------------------------------------------------------
�N���X�̐錾
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
