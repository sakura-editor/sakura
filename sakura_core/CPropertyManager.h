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
#ifndef SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_
#define SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_

#include "prop/CPropCommon.h"
#include "typeprop/CPropTypes.h"

class CImageListMgr;
class CMenuDrawer;

class CPropertyManager{
public:
	CPropertyManager( HWND, CImageListMgr*, CMenuDrawer* );

	/*
	|| ���̑�
	*/
	BOOL OpenPropertySheet( int nPageNum );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );	/* �^�C�v�ʐݒ� */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};

#endif /* SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_ */
/*[EOF]*/
