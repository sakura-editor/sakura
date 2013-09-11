/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef _CPROPERTYMANAGER_H_
#define _CPROPERTYMANAGER_H_

#include "CPropCommon.h"
#include "CPropTypes.h"

class CImageListMgr;
class CMenuDrawer;

class CPropertyManager{
public:
	void Create( HINSTANCE, HWND, CImageListMgr*, CMenuDrawer* );

	/*
	|| ÇªÇÃëº
	*/
	bool OpenPropertySheet( HWND hWnd, int nPageNum );	/* ã§í ê›íË */
	bool OpenPropertySheetTypes( HWND hWnd, int nPageNum, int nSettingType );	/* É^ÉCÉvï ê›íË */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};

#endif /* _CPROPERTYMANAGER_H_ */
/*[EOF]*/
