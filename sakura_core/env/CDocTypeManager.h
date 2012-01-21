/*
	2008.05.18 kobake CShareData ���番��
*/
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
#ifndef SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_
#define SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_

#include "DLLSHAREDATA.h"

//! �h�L�������g�^�C�v�Ǘ�
class CDocTypeManager{
public:
	CDocTypeManager()
	{
		m_pShareData = &GetDllShareData();
	}
	CTypeConfig GetDocumentTypeOfPath( const TCHAR* pszFilePath );	/* �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	CTypeConfig GetDocumentTypeOfExt( const TCHAR* pszExt );		/* �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */

	STypeConfig& GetTypeSetting(CTypeConfig cDocumentType)
	{
		int n = cDocumentType.GetIndex();
		assert(n>=0 && n<_countof(m_pShareData->m_Types));
		return m_pShareData->m_Types[n];
	}

private:
	DLLSHAREDATA* m_pShareData;
};

#endif /* SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_ */
/*[EOF]*/
