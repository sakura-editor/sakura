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
#include "CViewFont.h"

/*! �t�H���g�쐬
*/
void CViewFont::CreateFont(const LOGFONT *plf)
{
	LOGFONT	lf;

	/* �t�H���g�쐬 */
	lf = *plf;
	m_hFont_HAN = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = *plf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = *plf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	lf = *plf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD_UL = CreateFontIndirect( &lf );
}

/*! �t�H���g�폜
*/
void CViewFont::DeleteFont()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_BOLD );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_BOLD_UL );
}

/*! �t�H���g��I��
	@param bBold true�ő���
	@param bUnderLine true�ŉ���
*/
HFONT CViewFont::ChooseFontHandle( bool bBold, bool bUnderLine ) const
{
	if( bBold ){	/* ������ */
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_BOLD_UL;
		}else{
			return m_hFont_HAN_BOLD;
		}
	}else{
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}

