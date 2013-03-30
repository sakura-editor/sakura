/*!	@file
	@brief BASE64 Decode

	@author 
*/

/*
	Copyright (C)

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


#include "StdAfx.h"
#include "convert/CDecode_Base64Decode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"

/* Base64�f�R�[�h */
bool CDecode_Base64Decode::DoDecode( const CNativeW& pcSrc, CMemory* pcDst )
{
	using namespace WCODE;

	const int BUFFER_SIZE = 1024;  // �o�b�t�@�T�C�Y�B�P�ȏ�̐������S�̔{���ŁB
	const int _BUFSIZE = ((BUFFER_SIZE+3)/4)*4;

	const wchar_t *pSrc;
	int nSrcLen;
	char *pw, *pw_base;
	wchar_t buffer[_BUFSIZE];
	int i, j;
	wchar_t c = 0;

	pSrc = pcSrc.GetStringPtr();
	nSrcLen = pcSrc.GetStringLength();
	pcDst->AllocBuffer( nSrcLen );  // �������݃o�b�t�@���m��
	pw_base = pw = reinterpret_cast<char *>(pcDst->GetRawPtr());

	i = 0;  // pcSrc �̓Y����
	do{
		j = 0;
		for( ; i < nSrcLen; ++i ){
		// �o�b�t�@�ɕ��������߂郋�[�v
			c = pSrc[i];
			if( c == CR || c == LF || c == TAB || c == SPACE ){
				continue;
			}
			if( j == _BUFSIZE || c == LTEXT('=') ){
				break;
			}
			if( !IsBase64(c) ){
				return false;
			}
			buffer[j] = static_cast<char>(c & 0xff);
			++j;
		}
		pw += _DecodeBase64( &buffer[0], j, pw );
	}while( i < nSrcLen && c != LTEXT('=') );

	//if( CheckBase64Padbit(&buffer[0], j) == false ){
	//	return false;
	//}

	pcDst->_SetRawLength( pw - pw_base );
	return true;
}
