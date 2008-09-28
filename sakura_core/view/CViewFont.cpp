#include "stdafx.h"
#include "CViewFont.h"
#include "env/CShareData.h"

CViewFont::CViewFont()
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	LOGFONT	lf;

	/* �t�H���g�쐬 */
	m_hFont_HAN = CreateFontIndirect( &(pShareData->m_Common.m_sView.m_lf) );

	/* �����t�H���g�쐬 */
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );
}

CViewFont::~CViewFont()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_FAT );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_FAT_UL );
}

void CViewFont::UpdateFont()
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	LOGFONT	lf;

	/* �t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &pShareData->m_Common.m_sView.m_lf );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_FAT );
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_FAT_UL );
	lf = pShareData->m_Common.m_sView.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );
}

/*! �t�H���g��I��
	@param bFat TRUE�ő���
	@param bUnderLine TRUE�ŃA���_�[���C��
*/
HFONT CViewFont::ChooseFontHandle( BOOL bFat, BOOL bUnderLine ) const
{
	if( bFat ){	/* ������ */
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_FAT_UL;
		}else{
			return m_hFont_HAN_FAT;
		}
	}else{
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}

