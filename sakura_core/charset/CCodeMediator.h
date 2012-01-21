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
#ifndef SAKURA_CCODEMEDIATOR_653AC1CB_8935_4F2A_AF03_8FE669F6FA9F_H_
#define SAKURA_CCODEMEDIATOR_653AC1CB_8935_4F2A_AF03_8FE669F6FA9F_H_

#include "charset/CESI.h"
class CEditDoc;

class CCodeMediator{
protected:
	// CESI.cpp �̔���֐��������Ɉڂ�
	static ECodeType DetectMBCode( CESI* );
	static ECodeType DetectUnicode( CESI* );

public:

	explicit CCodeMediator( CEditDoc &ref ) : m_pcEditDoc(&ref) { }

	static ECodeType DetectUnicodeBom( const char* pS, const int nLen );

	/* ���{��R�[�h�Z�b�g���� */
	ECodeType CheckKanjiCode( const char*, int );
	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
	ECodeType CheckKanjiCodeOfFile( const TCHAR* );

	static ECodeType CheckKanjiCode( CESI* );  // CESI �\���́i�H�j���O���ō\�z�����ꍇ�Ɏg�p

private:
	CEditDoc* m_pcEditDoc;
};

#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
#include "CCesu8.h"

#endif /* SAKURA_CCODEMEDIATOR_653AC1CB_8935_4F2A_AF03_8FE669F6FA9F_H_ */
/*[EOF]*/
