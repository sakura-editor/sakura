/*!	@file
	@brief �����R�[�h�������ێ��N���X

	@author Sakura-Editor collaborators
	@date 2006/12/10 �V�K�쐬
*/
/*
	Copyright (C) 2006, rastiv

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

#ifndef _CESI_H_
#define _CESI_H_

#include "global.h"
#include "charcode.h"

/*
	�g�p���@
	
	CESI( const char*, const int ) �ŃI�u�W�F�N�g���쐬���邩�C
	ScanEncoding() ���g���ăf�[�^���擾���Ă���C
	Detect �n�֐����g��.
*/


class CESI
{
public:
	
	// CODE_CODEMAX -2 := �}���`�o�C�g�n�����R�[�h�̐�
	enum enum_CESI_public_constants {
		NUM_OF_MBCODE = CODE_CODEMAX - 2,
	};
	
	CESI(){ }
	CESI( const char* pS, const int nLen ){ ScanEncoding( pS, nLen ); }
	bool ScanEncoding( const char*, const int );
	int DetectUnicode( WCCODE_INFO* );
	int DetectMultibyte( MBCODE_INFO* );
	
private:
	
	MBCODE_INFO m_pEI[NUM_OF_MBCODE];	//!< �}���`�o�C�g�n�R�[�h���
	UNICODE_INFO m_WEI;		//!< ���j�R�[�h�n�R�[�h���
};


#endif /*_CESI_H_*/

