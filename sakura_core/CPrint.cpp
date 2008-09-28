/*!	@file
	@brief ���

	@author Norio Nakatani
	
	@date 2006.08.14 Moca �p�����ꗗ�̏d���폜�E���̓���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK
	Copyright (C) 2003, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include "CPrint.h"
#include "global.h"
#include "debug/Debug.h" // 2002/2/10 aroka
#include "env/CShareData.h"
#include <winspool.h>


// 2006.08.14 Moca �p�����ꗗ�̏d���폜�E���̓���
const PAPER_INFO CPrint::m_paperInfoArr[] = {
	// 	�p��ID, ��
	{DMPAPER_A4,                  2100,  2970, _T("A4 (210 x 297 mm)")},
	{DMPAPER_A3,                  2970,  4200, _T("A3 (297 x 420 mm)")},
	{DMPAPER_A4SMALL,             2100,  2970, _T("A4 small(210 x 297 mm)")},
	{DMPAPER_A5,                  1480,  2100, _T("A5 (148 x 210 mm)")},
	{DMPAPER_B4,                  2500,  3540, _T("B4 (250 x 354 mm)")},
	{DMPAPER_B5,                  1820,  2570, _T("B5 (182 x 257 mm)")},
	{DMPAPER_QUARTO,              2150,  2750, _T("Quarto(215 x 275 mm)")},
	{DMPAPER_ENV_DL,              1100,  2200, _T("DL Envelope(110 x 220 mm)")},
	{DMPAPER_ENV_C5,              1620,  2290, _T("C5 Envelope(162 x 229 mm)")},
	{DMPAPER_ENV_C3,              3240,  4580, _T("C3 Envelope(324 x 458 mm)")},
	{DMPAPER_ENV_C4,              2290,  3240, _T("C4 Envelope(229 x 324 mm)")},
	{DMPAPER_ENV_C6,              1140,  1620, _T("C6 Envelope(114 x 162 mm)")},
	{DMPAPER_ENV_C65,             1140,  2290, _T("C65 Envelope(114 x 229 mm)")},
	{DMPAPER_ENV_B4,              2500,  3530, _T("B4 Envelope(250 x 353 mm)")},
	{DMPAPER_ENV_B5,              1760,  2500, _T("B5 Envelope(176 x 250 mm)")},
	{DMPAPER_ENV_B6,              1760,  1250, _T("B6 Envelope(176 x 125 mm)")},
	{DMPAPER_ENV_ITALY,           1100,  2300, _T("Italy Envelope(110 x 230 mm)")},
	{DMPAPER_LETTER,              2159,  2794, _T("Letter (8 1/2 x 11 inch)")},
	{DMPAPER_LEGAL,               2159,  3556, _T("Legal  (8 1/2 x 14 inch)")},
	{DMPAPER_CSHEET,              4318,  5588, _T("C sheet (17 x 22 inch)")},
	{DMPAPER_DSHEET,              5588,  8634, _T("D sheet (22 x 34 inch)")},
	{DMPAPER_ESHEET,              8634, 11176, _T("E sheet (34 x 44 inch)")},
	{DMPAPER_LETTERSMALL,         2159,  2794, _T("Letter Small (8 1/2 x 11 inch)")},
	{DMPAPER_TABLOID,             2794,  4318, _T("Tabloid (11 x 17 inch)")},
	{DMPAPER_LEDGER,              4318,  2794, _T("Ledger  (17 x 11 inch)")},
	{DMPAPER_STATEMENT,           1397,  2159, _T("Statement (5 1/2 x 8 1/2 inch)")},
	{DMPAPER_EXECUTIVE,           1841,  2667, _T("Executive (7 1/4 x 10 1/2 inch)")},
	{DMPAPER_FOLIO,               2159,  3302, _T("Folio (8 1/2 x 13 inch)")},
	{DMPAPER_10X14,               2540,  3556, _T("10x14 inch sheet")},
	{DMPAPER_11X17,               2794,  4318, _T("11x17 inch sheet")},
	{DMPAPER_NOTE,                2159,  2794, _T("Note (8 1/2 x 11 inch)")},
	{DMPAPER_ENV_9,                984,  2254, _T("#9 Envelope  (3 7/8 x 8 7/8 inch)")},
	{DMPAPER_ENV_10,              1047,  2413, _T("#10 Envelope (4 1/8 x 9 1/2 inch)")},
	{DMPAPER_ENV_11,              1143,  2635, _T("#11 Envelope (4 1/2 x 10 3/8 inch)")},
	{DMPAPER_ENV_12,              1206,  2794, _T("#12 Envelope (4 3/4 x 11 inch)")},
	{DMPAPER_ENV_14,              1270,  2921, _T("#14 Envelope (5 x 11 1/2 inch)")},
	{DMPAPER_ENV_MONARCH,          984,  1905, _T("Monarch Envelope (3 7/8 x 7 1/2 inch)")},
	{DMPAPER_ENV_PERSONAL,         920,  1651, _T("6 3/4 Envelope (3 5/8 x 6 1/2 inch)")},
	{DMPAPER_FANFOLD_US,          3778,  2794, _T("US Std Fanfold (14 7/8 x 11 inch)")},
	{DMPAPER_FANFOLD_STD_GERMAN,  2159,  3048, _T("German Std Fanfold   (8 1/2 x 12 inch)")},
	{DMPAPER_FANFOLD_LGL_GERMAN,  2159,  3302, _T("German Legal Fanfold (8 1/2 x 13 inch)")},
};

const int CPrint::m_nPaperInfoArrNum = _countof( m_paperInfoArr );



CPrint::CPrint( void )
{
	m_hDevMode	= NULL;
	m_hDevNames	= NULL;
	return;
}

CPrint::~CPrint( void )
{
	// ���������蓖�čς݂Ȃ�΁A�������
	// 2003.05.18 �����
	if ( m_hDevMode != NULL ) {
		::GlobalFree( m_hDevMode );
	}
	if ( m_hDevNames != NULL ) {
		::GlobalFree( m_hDevNames );
	}
	m_hDevMode	= NULL;
	m_hDevNames	= NULL;
	return;
}



/*! @brief �v�����^�_�C�A���O��\�����āA�v�����^��I������
** 
** @param pPD			[i/o]	�v�����^�_�C�A���O�\����
** @param pMYDEVMODE 	[i/o] 	����ݒ�

	@author �����
	@date 2003.
*/
BOOL CPrint::PrintDlg( PRINTDLG *pPD, MYDEVMODE *pMYDEVMODE )
{
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* �v�����^�ݒ� DEVNAMES�p*/

	// �f�t�H���g�v�����^���I������Ă��Ȃ���΁A�I������
	if ( m_hDevMode == NULL ) {
		if ( !GetDefaultPrinter( pMYDEVMODE ) ) {
			return FALSE;
		}
	}

	//
	//  ���݂̃v�����^�ݒ�̕K�v������ύX
	//
	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;
	// PrintDlg()��ReAlloc����鎖���l���āA�Ăяo���O��Unlock
	::GlobalUnlock( m_hDevMode );

	/* �v�����^�_�C�A���O��\�����āA�v�����^��I�� */
	pPD->lStructSize = sizeof(*pPD);
	pPD->hDevMode = m_hDevMode;
	pPD->hDevNames = m_hDevNames;
	if( !::PrintDlg( pPD ) ){
		// �v�����^��ύX���Ȃ�����
		return FALSE;
	}

	m_hDevMode = pPD->hDevMode;
	m_hDevNames = pPD->hDevNames;

	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( m_hDevNames );

	// �v�����^�h���C�o��
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDriverName,
		_countof(pMYDEVMODE->m_szPrinterDriverName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// �v�����^�f�o�C�X��
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDeviceName,
		_countof(pMYDEVMODE->m_szPrinterDeviceName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// �v�����^�|�[�g��
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterOutputName,
		_countof(pMYDEVMODE->m_szPrinterOutputName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// �v�����^���瓾��ꂽ�AdmFields�͕ύX���Ȃ�
	// �v�����^���T�|�[�g���Ȃ�bit���Z�b�g����ƁA�v�����^�h���C�o�ɂ���ẮA�s����ȓ���������ꍇ������
	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����Z�b�g����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;

#ifdef _DEBUG
	MYTRACE_A( " (����/�o��) �f�o�C�X �h���C�o=[%ls]\n", (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE_A( " (����/�o��) �f�o�C�X��=[%ls]\n",        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE_A( "�����o�̓��f�B�A (�o�̓|�[�g) =[%ls]\n", (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE_A( "�f�t�H���g�̃v�����^��=[%d]\n", pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( m_hDevMode );
	::GlobalUnlock( m_hDevNames );
	return TRUE;
}


/*! @brief �f�t�H���g�̃v�����^���擾���AMYDEVMODE �ɐݒ� 
** 
** @param pMYDEVMODE 	[out] 	����ݒ�
*/
BOOL CPrint::GetDefaultPrinter( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* �v�����^�ݒ� DEVNAMES�p*/

	// ���ł� DEVMODE���擾�ς݂Ȃ�A�������Ȃ�
	if (m_hDevMode != NULL) {
		return TRUE;
	}

	//
	// PRINTDLG�\���̂�����������i�_�C�A���O�͕\�����Ȃ��悤�Ɂj
	// PrintDlg()�Ńf�t�H���g�v�����^�̃f�o�C�X���Ȃǂ��擾����
	//
	memset_raw ( &pd, 0, sizeof(pd) );
	pd.lStructSize	= sizeof(pd);
	pd.Flags		= PD_RETURNDEFAULT;
	if( !::PrintDlg( &pd ) ){
		pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* �v�����^���Ȃ������t���O */
		return FALSE;
	}
	pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* �v�����^���Ȃ������t���O */

	/* ������ */
	memset_raw( pMYDEVMODE, 0, sizeof(*pMYDEVMODE) );
	m_hDevMode = pd.hDevMode;
	m_hDevNames = pd.hDevNames;

	// MYDEVMODE�ւ̃R�s�[
	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( m_hDevNames );

	// �v�����^�h���C�o��
	_tcscpy_s( pMYDEVMODE->m_szPrinterDriverName,
		_countof(pMYDEVMODE->m_szPrinterDriverName),
		(TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// �v�����^�f�o�C�X��
	_tcscpy_s( pMYDEVMODE->m_szPrinterDeviceName,
		_countof(pMYDEVMODE->m_szPrinterDeviceName),
		(TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// �v�����^�|�[�g��
	_tcscpy_s( pMYDEVMODE->m_szPrinterOutputName,
		_countof(pMYDEVMODE->m_szPrinterOutputName),
		(TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// �v�����^���瓾��ꂽ�AdmFields�͕ύX���Ȃ�
	// �v�����^���T�|�[�g���Ȃ�bit���Z�b�g����ƁA�v�����^�h���C�o�ɂ���ẮA�s����ȓ���������ꍇ������
	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����R�s�[����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;

#ifdef _DEBUG
	MYTRACE_A( " (����/�o��) �f�o�C�X �h���C�o=[%ts]\n", (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE_A( " (����/�o��) �f�o�C�X��=[%ts]\n",        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE_A( "�����o�̓��f�B�A (�o�̓|�[�g) =[%ts]\n", (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE_A( "�f�t�H���g�̃v�����^��=[%d]\n",          pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( m_hDevMode );
	::GlobalUnlock( m_hDevNames );
	return TRUE;
}

/*! 
** @brief �v�����^���I�[�v�����AhDC���쐬����
*/
HDC CPrint::CreateDC(
	MYDEVMODE*	pMYDEVMODE,
	TCHAR*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
)
{
	HDC			hdc = NULL;
	HANDLE		hPrinter = NULL;
	DEVMODE*	pDEVMODE;

	// �v�����^���I������Ă��Ȃ���΁ANULL��Ԃ�
	if ( m_hDevMode == NULL ) {
		return NULL;
	}

	//
	// OpenPrinter()�ŁA�f�o�C�X���Ńv�����^�n���h�����擾
	//
	if( !::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,		/* �v�����^�f�o�C�X�� */
		&hPrinter,					/* �v�����^�n���h���̃|�C���^ */
		NULL
	) ){
		auto_sprintf(
			pszErrMsg,
			_T("OpenPrinter()�Ɏ��s�B\n")
			_T("�v�����^�f�o�C�X��=[%ts]"),
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		goto end_of_func;
	}

	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;

	//
	//DocumentProperties()�ŃA�v���P�[�V�����Ǝ��̃v�����^�ݒ�ɕύX����
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */,
		pDEVMODE,
		pDEVMODE,
		DM_OUT_BUFFER | DM_IN_BUFFER
	);
	/* �w��f�o�C�X�ɑ΂���f�o�C�X �R���e�L�X�g���쐬���܂��B */
	hdc = ::CreateDC(
		pMYDEVMODE->m_szPrinterDriverName,	/* �v�����^�h���C�o�� */
		pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		pMYDEVMODE->m_szPrinterOutputName,	/* �v�����^�|�[�g�� */
		pDEVMODE
	);

	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����R�s�[����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;

	::GlobalUnlock( m_hDevMode );

end_of_func:;
	if (hPrinter != NULL) {
		::ClosePrinter( hPrinter );
	}

	return hdc;
}


/* ���/�v���r���[�ɕK�v�ȏ����擾 */
BOOL CPrint::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
	int*		pnPaperAllWidth,	/* �p���� */
	int*		pnPaperAllHeight,	/* �p������ */
	int*		pnPaperWidth,		/* �p������\�� */
	int*		pnPaperHeight,		/* �p������\���� */
	int*		pnPaperOffsetLeft,	/* �p���]�����[ */
	int*		pnPaperOffsetTop,	/* �p���]����[ */
	TCHAR*		pszErrMsg			/* �G���[���b�Z�[�W�i�[�ꏊ */
)
{
	BOOL		bRet;
	HDC			hdc;
	bRet = TRUE;

	/* ���݂̐ݒ�ŁA�p���̕��A�������m�肵�ACreateDC�ɓn�� */
	if( !GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);

		bRet = TRUE;
	}

	// pMYDEVMODE���g���āAhdc���擾
	if ( NULL == (hdc = CreateDC( pMYDEVMODE, pszErrMsg )) ){
		bRet = FALSE;
		goto end_of_func;
	}

	/* CreateDC���s�ɂ���ē���ꂽ���ۂ̃v�����^�̗p���̕��A�������擾 */
	if( !GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);

		bRet = TRUE;
	}

	/* �}�b�s���O ���[�h�̐ݒ� */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC	���ꂼ��̘_���P�ʂ� 0.1 mm �Ƀ}�b�v����܂��B

	/* �ŏ����}�[�W���ƍŏ���}�[�W�����擾(1mm�P��) */
	POINT	po;
	if( 0 < ::Escape( hdc, GETPRINTINGOFFSET, (int)NULL, NULL, (LPPOINT)&po ) ){
		::DPtoLP( hdc, &po, 1 );
		*pnPaperOffsetLeft = abs( po.x );	/* �p���]�����[ */
		*pnPaperOffsetTop = abs( po.y );	/* �p���]����[ */
	}else{
		*pnPaperOffsetLeft = 0;	/* �p���]�����[ */
		*pnPaperOffsetTop = 0;	/* �p���]����[ */
	}

	/* �p���̈���\�ȕ��A���� */
	po.x = ::GetDeviceCaps( hdc, HORZRES );	/* �p������\���������f�B�X�v���C�̕� (mm �P��) */
	po.y = ::GetDeviceCaps( hdc, VERTRES );	/* �p������\�����������f�B�X�v���C�̍��� (mm �P��)  */
	::DPtoLP( hdc, &po, 1 );
	*pnPaperWidth = abs( po.x );
	*pnPaperHeight = abs( po.y );

	::DeleteDC( hdc );

end_of_func:;

	return bRet;
}



/* �p���̕��A���� */
BOOL CPrint::GetPaperSize(
	int*		pnPaperAllWidth,
	int*		pnPaperAllHeight,
	MYDEVMODE*	pDEVMODE
)
{
	int	nWork;


	if( pDEVMODE->dmFields &  DM_PAPERSIZE ){
		// 2006.08.14 Moca swich/case�e�[�u����p�~���� �p�����𓝍�
		const PAPER_INFO* pi = FindPaperInfo( pDEVMODE->dmPaperSize );
		if( NULL != pi ){
			*pnPaperAllWidth = pi->m_nAllWidth;
			*pnPaperAllHeight = pi->m_nAllHeight;
		}else{
			// 2001.12.21 hor �}�E�X�ŃN���b�N�����܂܃��X�g�O�ɏo��Ƃ����ɂ��邯�ǁA
			//	�ُ�ł͂Ȃ��̂� FALSE ��Ԃ����Ƃɂ���
			return FALSE;
		}
	}
	if(pDEVMODE->dmFields & DM_PAPERLENGTH && 0 != pDEVMODE->dmPaperLength ){
		/* pDEVMODE->dmPaperLength��1/10mm�P�ʂł��� */
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	} else {
		pDEVMODE->dmPaperLength = *pnPaperAllHeight;
		pDEVMODE->dmFields |= DM_PAPERLENGTH;
	}
	if(pDEVMODE->dmFields & DM_PAPERWIDTH && 0 != pDEVMODE->dmPaperWidth ){
		/* pDEVMODE->dmPaperWidth��1/10mm�P�ʂł��� */
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
	} else {
		pDEVMODE->dmPaperWidth = *pnPaperAllWidth;
		pDEVMODE->dmFields |= DM_PAPERWIDTH;
	}
	/* �p���̕��� */
	if( DMORIENT_LANDSCAPE == pDEVMODE->dmOrientation ){
		nWork = *pnPaperAllWidth;
		*pnPaperAllWidth = *pnPaperAllHeight;
		*pnPaperAllHeight = nWork;
	}
	return TRUE;
}







/* ��� �W���u�J�n */
BOOL CPrint::PrintOpen(
	TCHAR*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HDC*		phdc,
	TCHAR*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
)
{
	BOOL		bRet;
	HDC			hdc;
	DOCINFO		di;
	bRet = TRUE;
	// 
	// hdc���擾
	//
	if ( NULL == (hdc = CreateDC( pMYDEVMODE, pszErrMsg )) ){
		bRet = FALSE;
		goto end_of_func;
	}

	/* �}�b�s���O ���[�h�̐ݒ� */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC		���ꂼ��̘_���P�ʂ́A0.1 mm �Ƀ}�b�v����܂��B

	//
	//  ����W���u�J�n
	//
	memset_raw( &di, 0, sizeof( di ) );
	di.cbSize = sizeof(di);
	di.lpszDocName = pszJobName;
	di.lpszOutput  = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if( 0 >= ::StartDoc( hdc, &di ) ){
		auto_sprintf(
			pszErrMsg,
			_T("StartDoc()�Ɏ��s�B\n")
			_T("�v�����^�f�o�C�X��=[%ts]"),
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}

	*phdc = hdc;

end_of_func:;

	return bRet;
}



/* ��� �y�[�W�J�n */
void CPrint::PrintStartPage( HDC hdc )
{
	::StartPage( hdc );

}



/* ��� �y�[�W�I�� */
void CPrint::PrintEndPage( HDC hdc )
{
	::EndPage( hdc );

}


/* ��� �W���u�I�� */
void CPrint::PrintClose( HDC hdc )
{
	::EndDoc( hdc );
	::DeleteDC( hdc );
}





/* �p���̖��O���擾 */
TCHAR* CPrint::GetPaperName( int nPaperSize, TCHAR* pszPaperName )
{
	// 2006.08.14 Moca �p�����̓���
	const PAPER_INFO* paperInfo = FindPaperInfo( nPaperSize );
	if( NULL != paperInfo ){
		_tcscpy( pszPaperName, paperInfo->m_pszName );
	}else{
		_tcscpy( pszPaperName, _T("�s��") );
	}
	return pszPaperName;
}

/*!
	�p�����̎擾
	@date 2006.08.14 Moca �V�K�쐬 �p�����̓���
*/
const PAPER_INFO* CPrint::FindPaperInfo( int id )
{
	for( int i = 0; i < m_nPaperInfoArrNum; ++i ){
		if( m_paperInfoArr[i].m_nId == id ){
			return &(m_paperInfoArr[i]);
		}
	}
	return NULL;
}


/*!	@brief PRINTSETTING�̏�����

	�����ł�m_mdmDevMode�� �v�����^�ݒ�͎擾�E���������Ȃ�

	@date 2006.08.14 Moca  Initialize���疼�̕ύX�B�������P�ʂ�ShareDate�S�Ă���PRINTSETTING�P�ʂɕύX�D
		�{�֐�����DLLSHAREDATA�փA�N�Z�X�������ɁCCShareData����PPRINTSETTING�P�ʂŒ���n���Ă��炤�D
*/
void CPrint::SettingInitialize( PRINTSETTING& pPrintSetting, const TCHAR* settingName )
{
	_tcscpy( pPrintSetting.m_szPrintSettingName, settingName );	/* ����ݒ�̖��O */
	_tcscpy( pPrintSetting.m_szPrintFontFaceHan, _T("�l�r ����") );				/* ����t�H���g */
	_tcscpy( pPrintSetting.m_szPrintFontFaceZen, _T("�l�r ����") );				/* ����t�H���g */
	pPrintSetting.m_nPrintFontWidth = 12;  								/* ����t�H���g��(1/10mm�P��) */
	pPrintSetting.m_nPrintFontHeight = pPrintSetting.m_nPrintFontWidth * 2;	/* ����t�H���g����(1/10mm�P�ʒP��) */
	pPrintSetting.m_nPrintDansuu = 1;			/* �i�g�̒i�� */
	pPrintSetting.m_nPrintDanSpace = 70; 		/* �i�ƒi�̌���(1/10mm) */
	pPrintSetting.m_bPrintWordWrap = TRUE;		/* �p�����[�h���b�v���� */
	pPrintSetting.m_bPrintKinsokuHead = FALSE;		/* �s���֑����� */	//@@@ 2002.04.09 MIK
	pPrintSetting.m_bPrintKinsokuTail = FALSE;		/* �s���֑����� */	//@@@ 2002.04.09 MIK
	pPrintSetting.m_bPrintKinsokuRet = FALSE;		/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	pPrintSetting.m_bPrintKinsokuKuto = FALSE;		// 2006.08.14 Moca �������~�X
	pPrintSetting.m_bPrintLineNumber = FALSE;	/* �s�ԍ���������� */
	pPrintSetting.m_nPrintLineSpacing = 30;	/* ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
	pPrintSetting.m_nPrintMarginTY = 100;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	pPrintSetting.m_nPrintMarginBY = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	pPrintSetting.m_nPrintMarginLX = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	pPrintSetting.m_nPrintMarginRX = 100;		/* ����p���}�[�W�� �E(1/10mm�P��) */
	pPrintSetting.m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
	pPrintSetting.m_nPrintPaperSize = DMPAPER_A4;	/* �p���T�C�Y */
	/* �v�����^�ݒ� DEVMODE�p */
	/* �v�����^�ݒ���擾����̂̓R�X�g��������̂ŁA��ق� */
	//	m_cPrint.GetDefaultPrinterInfo( &(pPrintSetting.m_mdmDevMode) );
	pPrintSetting.m_bHeaderUse[0] = TRUE;
	pPrintSetting.m_bHeaderUse[1] = FALSE;
	pPrintSetting.m_bHeaderUse[2] = FALSE;
	wcscpy( pPrintSetting.m_szHeaderForm[0], L"$f" );
	wcscpy( pPrintSetting.m_szHeaderForm[1], L"" );
	wcscpy( pPrintSetting.m_szHeaderForm[2], L"" );
	pPrintSetting.m_bFooterUse[0] = TRUE;
	pPrintSetting.m_bFooterUse[1] = FALSE;
	pPrintSetting.m_bFooterUse[2] = FALSE;
	wcscpy( pPrintSetting.m_szFooterForm[0], L"" );
	wcscpy( pPrintSetting.m_szFooterForm[1], L"- $p -" );
	wcscpy( pPrintSetting.m_szFooterForm[2], L"" );
}


