/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����B�t�H���_�ŁB

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, YAZAKI
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2003, MIK

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

#ifndef _CMRUFOLDER_H_
#define _CMRUFOLDER_H_

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include "recent/CRecentFolder.h"

class CMenuDrawer;

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class CMRUFolder {
public:
	//	�R���X�g���N�^
	CMRUFolder();
	~CMRUFolder();

	//	���j���[���擾����
	HMENU CreateMenu( CMenuDrawer* pCMenuDrawer ) const;	//	���[��BpCMenuDrawer���K�v�Ȃ��Ȃ�Ƃ����Ȃ��B
	HMENU CreateMenu( HMENU hMenu, CMenuDrawer* pCMenuDrawer ) const;	//	2010/5/21 Uchi
	BOOL DestroyMenu( HMENU hMenu ) const;
	
	//	�t�H���_���̈ꗗ��������
	std::vector<LPCTSTR> GetPathList() const;

	//	�A�N�Z�X�֐�
	int Length() const;	//	�A�C�e���̐��B
	int MenuLength(void) const{ return t_min(Length(), m_cRecentFolder.GetViewCount()); }	//	���j���[�ɕ\�������A�C�e���̐�
	void ClearAll();					//	�A�C�e�����폜�`�B
	void Add( const TCHAR* pszFolder );	//	pszFolder��ǉ�����B
	const TCHAR* GetPath(int num) const;

protected:
	//	���L�������A�N�Z�X�p�B
	struct DLLSHAREDATA*	m_pShareData;			//	���L���������Q�Ƃ����B

private:
	CRecentFolder	m_cRecentFolder;	//����	//@@@ 2003.04.08 MIK
};

#endif
