/*
	2008.05.18 kobake CShareData ���番��
*/

#include "StdAfx.h"
#include "DLLSHAREDATA.h"

#include "CFormatManager.h"

/*! ���t���t�H�[�}�b�g
	systime�F�����f�[�^
	
	pszDest�F�t�H�[�}�b�g�ς݃e�L�X�g�i�[�p�o�b�t�@
	nDestLen�FpszDest�̒���
	
	pszDateFormat�F
		�J�X�^���̂Ƃ��̃t�H�[�}�b�g
*/
const TCHAR* CFormatManager::MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nDateFormatType,
		m_pShareData->m_Common.m_sFormat.m_szDateFormat
	);
}

const TCHAR* CFormatManager::MyGetDateFormat(
	const SYSTEMTIME&		systime,
	TCHAR*		pszDest,
	int				nDestLen,
	int				nDateFormatType,
	const TCHAR*	szDateFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/* �������t�H�[�}�b�g */
const TCHAR* CFormatManager::MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType,
		m_pShareData->m_Common.m_sFormat.m_szTimeFormat
	);
}

/* �������t�H�[�}�b�g */
const TCHAR* CFormatManager::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	TCHAR*			pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const TCHAR*		szTimeFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}

