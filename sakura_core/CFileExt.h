/*!	@file
	@brief �I�[�v���_�C�A���O�p�t�@�C���g���q�Ǘ�

	@author MIK
	@date 2003.5.12
*/
/*
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

#ifndef SAKURA_CFILEEXT_H_
#define SAKURA_CFILEEXT_H_

#include "_main/global.h"
#include "config/maxdata.h"

class CFileExt
{
public:
	CFileExt();
	~CFileExt();

	bool AppendExt( const TCHAR *pszName, const TCHAR *pszExt );
	bool AppendExtRaw( const TCHAR *pszName, const TCHAR *pszExt );
	const TCHAR *GetName( int nIndex );
	const TCHAR *GetExt( int nIndex );

	//�_�C�A���O�ɓn���g���q�t�B���^���擾����B(lpstrFilter�ɒ��ڎw��\)
	//2��Ăяo���ƌÂ��o�b�t�@�������ɂȂ邱�Ƃ�����̂ɒ���
	const TCHAR *GetExtFilter( void );

	int GetCount( void ) { return m_nCount; }

protected:
	bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, TCHAR *pszDstExt );

private:

	typedef struct {
		TCHAR	m_szName[64];		//���O(64�����ȉ��̂͂���m_szTypeName)
		TCHAR	m_szExt[MAX_TYPES_EXTS*3+1];	//�g���q(64�����ȉ��̂͂���m_szTypeExts) �Ȃ� "*." ��ǉ�����̂ł���Ȃ�ɕK�v
	} FileExtInfoTag;

	int				m_nCount;
	FileExtInfoTag	*m_puFileExtInfo;
	std::vector<TCHAR>	m_vstrFilter;
};

#endif	//SAKURA_CFILEEXT_H_

