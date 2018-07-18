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

/*! フォント作成
*/
void CViewFont::CreateFont(const LOGFONT *plf)
{
	LOGFONT	lf;
	int miniSize = GetDllShareData().m_Common.m_sWindow.m_nMiniMapFontSize;
	int quality = GetDllShareData().m_Common.m_sWindow.m_nMiniMapQuality;
	int outPrec = OUT_TT_ONLY_PRECIS;	// FixedSys等でMiniMapのフォントが小さくならない修正

	/* フォント作成 */
	lf = *plf;
	if( m_bMiniMap ){
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	m_hFont_HAN = CreateFontIndirect( &lf );
	m_LogFont = lf;

	/* 太字フォント作成 */
	lf = *plf;
	if( m_bMiniMap ){
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	lf = *plf;
	if( m_bMiniMap ){
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	lf = *plf;
	if( m_bMiniMap ){
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD_UL = CreateFontIndirect( &lf );
}

/*! フォント削除
*/
void CViewFont::DeleteFont()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_BOLD );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_BOLD_UL );
}

/*! フォントを選ぶ
	@param m_bBoldFont trueで太字
	@param m_bUnderLine trueで下線
*/
HFONT CViewFont::ChooseFontHandle( int fontNo, SFontAttr sFontAttr ) const
{
	assert( fontNo == 0 );
	if( sFontAttr.m_bBoldFont ){	/* 太字か */
		if( sFontAttr.m_bUnderLine ){	/* 下線か */
			return m_hFont_HAN_BOLD_UL;
		}else{
			return m_hFont_HAN_BOLD;
		}
	}else{
		if( sFontAttr.m_bUnderLine ){	/* 下線か */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}

