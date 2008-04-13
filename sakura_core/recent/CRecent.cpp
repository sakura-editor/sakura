/*!	@file
	@brief �ŋߎg�������X�g

	���C�ɓ�����܂ލŋߎg�������X�g���Ǘ�����B

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005

*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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


#include "stdafx.h"
#include "recent/CRecent.h"
#include "CShareData.h"

DLLSHAREDATA* CRecent::GetShareData()
{
	return CShareData::getInstance()->GetShareData();
}
