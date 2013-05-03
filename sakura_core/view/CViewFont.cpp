#include "StdAfx.h"
#include "CViewFont.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"

CViewFont::CViewFont()
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	LOGFONT	lf = CEditWnd::getInstance()->GetLogfont();

	/* フォント作成 */
	m_hFont_HAN = CreateFontIndirect( &lf );

	/* 太字フォント作成 */
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD_UL = CreateFontIndirect( &lf );
}

CViewFont::~CViewFont()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_BOLD );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_BOLD_UL );
}

void CViewFont::UpdateFont()
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	LOGFONT	lf = CEditWnd::getInstance()->GetLogfont();

	/* フォント作成 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &lf );

	/* 太字フォント作成 */
	::DeleteObject( m_hFont_HAN_BOLD );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	::DeleteObject( m_hFont_HAN_BOLD_UL );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD_UL = CreateFontIndirect( &lf );
}

/*! フォントを選ぶ
	@param bBold TRUEで太字
	@param bUnderLine TRUEでアンダーライン
*/
HFONT CViewFont::ChooseFontHandle( BOOL bBold, BOOL bUnderLine ) const
{
	if( bBold ){	/* 太字か */
		if( bUnderLine ){	/* 下線か */
			return m_hFont_HAN_BOLD_UL;
		}else{
			return m_hFont_HAN_BOLD;
		}
	}else{
		if( bUnderLine ){	/* 下線か */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}

