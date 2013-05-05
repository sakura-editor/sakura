#include "StdAfx.h"
#include "CViewFont.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"

CViewFont::CViewFont()
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	LOGFONT	lf = CEditWnd::getInstance()->GetLogfont();

	/* �t�H���g�쐬 */
	m_hFont_HAN = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
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

	/* �t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_BOLD );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_BOLD_UL );
	lf = CEditWnd::getInstance()->GetLogfont();
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_BOLD_UL = CreateFontIndirect( &lf );
}

/*! �t�H���g��I��
	@param bBold true�ő���
	@param bUnderLine true�ŃA���_�[���C��
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

