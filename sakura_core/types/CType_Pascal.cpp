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

#include "StdAfx.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

/* Pascal */
//Mar. 10, 2001 JEPRO	���p���l��F�����\��
void CType_Pascal::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Pascal") );
	_tcscpy( pType->m_szTypeExts, _T("dpr,pas") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* �s�R�����g�f���~�^ */		//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_cBlockComments[0].SetBlockCommentRule( L"{", L"}" );	/* �u���b�N�R�����g�f���~�^ */	//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_cBlockComments[1].SetBlockCommentRule( L"(*", L"*)" );	/* �u���b�N�R�����g�f���~�^2 */	//@@@ 2001.03.10 by MIK
	pType->m_nStringType = 1;										/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_nKeyWordSetIdx[0] = 8;									/* �L�[���[�h�Z�b�g */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//@@@ 2001.11.11 upd MIK
	pType->m_bStringLineOnly = true; // ������͍s���̂�
}



const wchar_t* g_ppszKeywordsPASCAL[] = {
	L"and",
	L"exports",
	L"mod",
	L"shr",
	L"array",
	L"file",
	L"nil",
	L"string",
	L"as",
	L"finalization",
	L"not",
	L"stringresource",
	L"asm",
	L"finally",
	L"object",
	L"then",
	L"begin",
	L"for",
	L"of",
	L"case",
	L"function",
	L"or",
	L"to",
	L"class",
	L"goto",
	L"out",
	L"try",
	L"const",
	L"if",
	L"packed",
	L"type",
	L"constructor",
	L"implementation",
	L"procedure",
	L"unit",
	L"destructor",
	L"in",
	L"program",
	L"until",
	L"dispinterface",
	L"inherited",
	L"property",
	L"uses",
	L"div",
	L"initialization",
	L"raise",
	L"var",
	L"do",
	L"inline",
	L"record",
	L"while",
	L"downto",
	L"interface",
	L"repeat",
	L"with",
	L"else",
	L"is",
	L"resourcestring",
	L"xor",
	L"end",
	L"label",
	L"set",
	L"except",
	L"library",
	L"shl",
	L"private",
	L"public",
	L"published",
	L"protected",
	L"override"
};
int g_nKeywordsPASCAL = _countof(g_ppszKeywordsPASCAL);
