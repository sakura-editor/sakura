//	$Id$
/*!	@file
	@brief ���

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//#include <stdio.h>
#include <stdlib.h>
#include "CPrint.h"
#include "global.h"
#include "debug.h" // 2002/2/10 aroka
#include "CShareData.h"


CPrint::CPrint( void )
{
	return;
}

CPrint::~CPrint( void )
{
	return;
}



/* �f�t�H���g�̃v�����^�����擾 */
BOOL CPrint::GetDefaultPrinter( PRINTDLG* pPRINTDLG )
{
	//
	// PRINTDLG�\���̂�����������i�_�C�A���O�͕\�����Ȃ��悤�Ɂj
	// PrintDlg()�Ńf�t�H���g�v�����^�̃f�o�C�X���Ȃǂ��擾����
	//
	memset ( (void *)pPRINTDLG, 0, sizeof(PRINTDLG) );
	pPRINTDLG->lStructSize	= sizeof(PRINTDLG);
	pPRINTDLG->hwndOwner	= NULL;
	pPRINTDLG->Flags		= PD_RETURNDEFAULT;
	pPRINTDLG->hInstance	= NULL;
	return ::PrintDlg( pPRINTDLG );
}


/* �f�t�H���g�̃v�����^�����擾 */
BOOL CPrint::GetPrinter( PRINTDLG* pPRINTDLG )
{
	//
	// PRINTDLG�\���̂�����������i�_�C�A���O�͕\�����Ȃ��悤�Ɂj
	// PrintDlg()�Ńf�t�H���g�v�����^�̃f�o�C�X���Ȃǂ��擾����
	//
	memset ( (void *)pPRINTDLG, 0, sizeof(PRINTDLG) );
	pPRINTDLG->lStructSize	= sizeof(PRINTDLG);
	return ::PrintDlg( pPRINTDLG );
}


/* �f�t�H���g�̃v�����^�ݒ� MYDEVMODE ���擾 */
BOOL CPrint::GetPrinterInfo( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* �v�����^�ݒ� DEVNAMES�p*/

	/* ������ */
	memset( (void *)pMYDEVMODE, 0, sizeof(MYDEVMODE) );

	/* �f�t�H���g�̃v�����^�����擾 */
	if( FALSE == GetPrinter( &pd ) ){
		pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* �v�����^���Ȃ������t���O */
		return FALSE;
	}
	pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* �v�����^���Ȃ������t���O */

	pDEVMODE = (DEVMODE*)::GlobalLock( pd.hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( pd.hDevNames );

	strcpy( pMYDEVMODE->m_szPrinterDriverName, (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );	/* �v�����^�h���C�o�� */
	strcpy( pMYDEVMODE->m_szPrinterDeviceName, (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );	/* �v�����^�f�o�C�X�� */
	strcpy( pMYDEVMODE->m_szPrinterOutputName, (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );	/* �v�����^�|�[�g�� */

	pMYDEVMODE->dmFields = DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH | DM_SCALE | DM_COPIES | DM_DEFAULTSOURCE | DM_PRINTQUALITY | DM_COLOR | DM_DUPLEX |
		DM_YRESOLUTION | DM_TTOPTION | DM_COLLATE | DM_FORMNAME | DM_LOGPIXELS |
		DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS |
		DM_DISPLAYFREQUENCY;
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;
	pMYDEVMODE->dmScale				= pDEVMODE->dmScale;
	pMYDEVMODE->dmCopies			= pDEVMODE->dmCopies;
	pMYDEVMODE->dmDefaultSource		= pDEVMODE->dmDefaultSource;
	pMYDEVMODE->dmPrintQuality		= pDEVMODE->dmPrintQuality;
	pMYDEVMODE->dmColor				= pDEVMODE->dmColor;
	pMYDEVMODE->dmDuplex			= pDEVMODE->dmDuplex;
	pMYDEVMODE->dmYResolution		= pDEVMODE->dmYResolution;
	pMYDEVMODE->dmTTOption			= pDEVMODE->dmTTOption;
	pMYDEVMODE->dmCollate			= pDEVMODE->dmCollate;
	pMYDEVMODE->dmLogPixels			= pDEVMODE->dmLogPixels;
	pMYDEVMODE->dmBitsPerPel		= pDEVMODE->dmBitsPerPel;
	pMYDEVMODE->dmPelsWidth			= pDEVMODE->dmPelsWidth;
	pMYDEVMODE->dmPelsHeight		= pDEVMODE->dmPelsHeight;
	pMYDEVMODE->dmDisplayFlags		= pDEVMODE->dmDisplayFlags;
	pMYDEVMODE->dmDisplayFrequency  = pDEVMODE->dmDisplayFrequency ;
	strcpy( (char *)pMYDEVMODE->dmFormName, (char *)pDEVMODE->dmFormName );

#ifdef _DEBUG
	MYTRACE( " (����/�o��) �f�o�C�X �h���C�o=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE( " (����/�o��) �f�o�C�X��=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE( "�����o�̓��f�B�A (�o�̓|�[�g) =[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE( "�f�t�H���g�̃v�����^��=[%d]\n", pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( pd.hDevMode );
	::GlobalUnlock( pd.hDevNames );
	return TRUE;
}

/* �f�t�H���g�̃v�����^�ݒ� MYDEVMODE ���擾 */
BOOL CPrint::GetDefaultPrinterInfo( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* �v�����^�ݒ� DEVNAMES�p*/

	/* ������ */
	memset( (void *)pMYDEVMODE, 0, sizeof(MYDEVMODE) );

	/* �f�t�H���g�̃v�����^�����擾 */
	if( FALSE == GetDefaultPrinter( &pd ) ){
		pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* �v�����^���Ȃ������t���O */
		return FALSE;
	}
	pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* �v�����^���Ȃ������t���O */

	pDEVMODE = (DEVMODE*)::GlobalLock( pd.hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( pd.hDevNames );

	strcpy( pMYDEVMODE->m_szPrinterDriverName, (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );	/* �v�����^�h���C�o�� */
	strcpy( pMYDEVMODE->m_szPrinterDeviceName, (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );	/* �v�����^�f�o�C�X�� */
	strcpy( pMYDEVMODE->m_szPrinterOutputName, (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );	/* �v�����^�|�[�g�� */

	pMYDEVMODE->dmFields = DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH | DM_SCALE | DM_COPIES | DM_DEFAULTSOURCE | DM_PRINTQUALITY | DM_COLOR | DM_DUPLEX |
		DM_YRESOLUTION | DM_TTOPTION | DM_COLLATE | DM_FORMNAME | DM_LOGPIXELS |
		DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS |
		DM_DISPLAYFREQUENCY;
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;
	pMYDEVMODE->dmScale				= pDEVMODE->dmScale;
	pMYDEVMODE->dmCopies			= pDEVMODE->dmCopies;
	pMYDEVMODE->dmDefaultSource		= pDEVMODE->dmDefaultSource;
	pMYDEVMODE->dmPrintQuality		= pDEVMODE->dmPrintQuality;
	pMYDEVMODE->dmColor				= pDEVMODE->dmColor;
	pMYDEVMODE->dmDuplex			= pDEVMODE->dmDuplex;
	pMYDEVMODE->dmYResolution		= pDEVMODE->dmYResolution;
	pMYDEVMODE->dmTTOption			= pDEVMODE->dmTTOption;
	pMYDEVMODE->dmCollate			= pDEVMODE->dmCollate;
	pMYDEVMODE->dmLogPixels			= pDEVMODE->dmLogPixels;
	pMYDEVMODE->dmBitsPerPel		= pDEVMODE->dmBitsPerPel;
	pMYDEVMODE->dmPelsWidth			= pDEVMODE->dmPelsWidth;
	pMYDEVMODE->dmPelsHeight		= pDEVMODE->dmPelsHeight;
	pMYDEVMODE->dmDisplayFlags		= pDEVMODE->dmDisplayFlags;
	pMYDEVMODE->dmDisplayFrequency  = pDEVMODE->dmDisplayFrequency ;
	strcpy( (char *)pMYDEVMODE->dmFormName, (char *)pDEVMODE->dmFormName );

#ifdef _DEBUG
	MYTRACE( " (����/�o��) �f�o�C�X �h���C�o=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE( " (����/�o��) �f�o�C�X��=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE( "�����o�̓��f�B�A (�o�̓|�[�g) =[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE( "�f�t�H���g�̃v�����^��=[%d]\n", pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( pd.hDevMode );
	::GlobalUnlock( pd.hDevNames );
	return TRUE;
}


/* ���/�v���r���[�ɕK�v�ȏ����擾 */
BOOL CPrint::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
//	LOGFONT*	pLOGFONT,
//	int			nMarginTY,			/* �}�[�W�� �� */
//	int			nMarginBY,			/* �}�[�W�� �� */
//	int			nMarginLX,			/* �}�[�W�� �� */
//	int			nMarginRX,			/* �}�[�W�� �E */
//	int			nLineSpacing,		/* �s�� �����̍����ɑ΂��銄�� */

	int*		pnPaperAllWidth,	/* �p���� */
	int*		pnPaperAllHeight,	/* �p������ */
	int*		pnPaperWidth,		/* �p������\�� */
	int*		pnPaperHeight,		/* �p������\���� */
	int*		pnPaperOffsetLeft,	/* �p���]�����[ */
	int*		pnPaperOffsetTop,	/* �p���]����[ */
//	int*		pnCharWidth,	/* ������ */
//	int*		pnCharHeight,	/* �������� */
//	int*		pnAllChars,		/* �������Ɉ󎚉\�Ȍ��� */
//	int*		pnAllLines,		/* �c�����Ɉ󎚉\�ȍs�� */
	char*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
)
{
	BOOL		bRet;
	HDC			hdc;
//	HFONT		hFont, hFontOld;
	HANDLE		hPrinter;
	HGLOBAL		hgDEVMODE;
	DEVMODE*	pDEVMODE;
	int			nDEVMODE_Size;
//	TEXTMETRIC	tm;
	bRet = TRUE;
	//
	// OpenPrinter()�ŁA�f�o�C�X���Ńv�����^�n���h�����擾
	//
	if( FALSE == ::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		&hPrinter,	/* �v�����^�n���h���̃|�C���^ */
		(PRINTER_DEFAULTS*)NULL
	) ){
		wsprintf( pszErrMsg,
			"OpenPrinter()�Ɏ��s�B\n�v�����^�f�o�C�X��=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	//
	//DocumentProperties()��DEVMODE�\���̂ɕK�v�ȃ������T�C�Y���擾
	//�iDEVMODE�\���̂̓v�����^�h���C�o���Ƃɒǉ��G���A�̃T�C�Y���قȂ�j
	// �K�v�ȃT�C�Y�̃��������m�ۂ��A���b�N����
	//
	if( 0 > ( nDEVMODE_Size = ::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		NULL,
		NULL,
		0
	) ) ){
		wsprintf( pszErrMsg,
			"DocumentProperties()�Ɏ��s�B\n�v�����^�f�o�C�X��=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	hgDEVMODE = ::GlobalAlloc( GHND, nDEVMODE_Size );
	pDEVMODE = (DEVMODE*)::GlobalLock( hgDEVMODE );
	//
	//DocumentProperties()�Ō��݂̃v�����^�ݒ��DEVMODE�\���̂Ɏ擾
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */,
		pDEVMODE,
		NULL,
		DM_OUT_BUFFER
	);

	//
	//  ���݂̃v�����^�ݒ�̕K�v������ύX
	//
	pDEVMODE->dmFields				|= pMYDEVMODE->dmFields;
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;
	pDEVMODE->dmScale				= pMYDEVMODE->dmScale;
	pDEVMODE->dmCopies				= pMYDEVMODE->dmCopies;
	pDEVMODE->dmDefaultSource		= pMYDEVMODE->dmDefaultSource;
	pDEVMODE->dmPrintQuality		= pMYDEVMODE->dmPrintQuality;
	pDEVMODE->dmColor				= pMYDEVMODE->dmColor;
	pDEVMODE->dmDuplex				= pMYDEVMODE->dmDuplex;
	pDEVMODE->dmYResolution			= pMYDEVMODE->dmYResolution;
	pDEVMODE->dmTTOption			= pMYDEVMODE->dmTTOption;
	pDEVMODE->dmCollate				= pMYDEVMODE->dmCollate;
	pDEVMODE->dmLogPixels			= pMYDEVMODE->dmLogPixels;
	pDEVMODE->dmBitsPerPel			= pMYDEVMODE->dmBitsPerPel;
	pDEVMODE->dmPelsWidth			= pMYDEVMODE->dmPelsWidth;
	pDEVMODE->dmPelsHeight			= pMYDEVMODE->dmPelsHeight;
	pDEVMODE->dmDisplayFlags		= pMYDEVMODE->dmDisplayFlags;
	pDEVMODE->dmDisplayFrequency	= pMYDEVMODE->dmDisplayFrequency ;
	strcpy( (char *)pDEVMODE->dmFormName, (char *)pMYDEVMODE->dmFormName );

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
		(LPCTSTR)pMYDEVMODE->m_szPrinterDriverName,	/* �v�����^�h���C�o�� */
		(LPCTSTR)pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		(LPCTSTR)pMYDEVMODE->m_szPrinterOutputName,	/* �v�����^�|�[�g�� */
		pDEVMODE
	);

	pMYDEVMODE->dmFields = DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH | DM_SCALE | DM_COPIES | DM_DEFAULTSOURCE | DM_PRINTQUALITY | DM_COLOR | DM_DUPLEX |
		DM_YRESOLUTION | DM_TTOPTION | DM_COLLATE | DM_FORMNAME | DM_LOGPIXELS |
		DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS |
		DM_DISPLAYFREQUENCY;
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;
	pMYDEVMODE->dmScale				= pDEVMODE->dmScale;
	pMYDEVMODE->dmCopies			= pDEVMODE->dmCopies;
	pMYDEVMODE->dmDefaultSource		= pDEVMODE->dmDefaultSource;
	pMYDEVMODE->dmPrintQuality		= pDEVMODE->dmPrintQuality;
	pMYDEVMODE->dmColor				= pDEVMODE->dmColor;
	pMYDEVMODE->dmDuplex			= pDEVMODE->dmDuplex;
	pMYDEVMODE->dmYResolution		= pDEVMODE->dmYResolution;
	pMYDEVMODE->dmTTOption			= pDEVMODE->dmTTOption;
	pMYDEVMODE->dmCollate			= pDEVMODE->dmCollate;
	pMYDEVMODE->dmLogPixels			= pDEVMODE->dmLogPixels;
	pMYDEVMODE->dmBitsPerPel		= pDEVMODE->dmBitsPerPel;
	pMYDEVMODE->dmPelsWidth			= pDEVMODE->dmPelsWidth;
	pMYDEVMODE->dmPelsHeight		= pDEVMODE->dmPelsHeight;
	pMYDEVMODE->dmDisplayFlags		= pDEVMODE->dmDisplayFlags;
	pMYDEVMODE->dmDisplayFrequency  = pDEVMODE->dmDisplayFrequency ;
	strcpy( (char *)pMYDEVMODE->dmFormName, (char *)pDEVMODE->dmFormName );

	::GlobalUnlock( hgDEVMODE );
	::GlobalFree( hgDEVMODE );

	/* �}�b�s���O ���[�h�̐ݒ� */
//	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC	���ꂼ��̘_���P�ʂ� 0.1 mm �Ƀ}�b�v����܂��B
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_HIMETRIC	���ꂼ��̘_���P�ʂ� 0.01 mm �Ƀ}�b�v����܂��B



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


//	hFont = ::CreateFontIndirect( pLOGFONT );
//	hFontOld = ::SelectObject( hdc, hFont );

	/* �p���̈���\�ȕ��A���� */
	po.x = ::GetDeviceCaps( hdc, HORZRES );	/* �p������\���������f�B�X�v���C�̕� (mm �P��) */
	po.y = ::GetDeviceCaps( hdc, VERTRES );	/* �p������\�����������f�B�X�v���C�̍��� (mm �P��)  */
	::DPtoLP( hdc, &po, 1 );
	*pnPaperWidth = abs( po.x );
	*pnPaperHeight = abs( po.y );

	/* �p���̕��A���� */
	if( FALSE == GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);

//		bRet = FALSE;
		bRet = TRUE;
	}



//	::GetTextMetrics( hdc, &tm );
//	*pnCharWidth = abs( tm.tmAveCharWidth );	/* ������ */
//	*pnCharHeight = abs( tm.tmHeight + tm.tmExternalLeading );	/* �������� */


//	::SelectObject( hdc, hFontOld );
//	::DeleteObject( hFont );
	::DeleteDC( hdc );
	::ClosePrinter( hPrinter );

end_of_func:;

	return bRet;
}



/* �p���̕��A���� */
BOOL CPrint::GetPaperSize(
	int*		pnPaperAllWidth,
	int*		pnPaperAllHeight,
	DEVMODE*	pDEVMODE
)
{
	int	nWork;


	if( pDEVMODE->dmFields &  DM_PAPERSIZE ){
		switch( pDEVMODE->dmPaperSize ){
		case DMPAPER_A4:	//	DMPAPER_A4					A4 sheet�A210 �~ 297 mm
			*pnPaperAllWidth =  10 * 210;
			*pnPaperAllHeight = 10 * 297;
			break;
		case DMPAPER_A3:	//	DMPAPER_A3					A3 sheet�A297 �~ 420 mm
			*pnPaperAllWidth =  10 * 297;
			*pnPaperAllHeight = 10 * 420;
			break;
		case DMPAPER_A4SMALL:	//	DMPAPER_A4SMALL			A4 small sheet�A210 �~ 297 mm
			*pnPaperAllWidth =  10 * 210;
			*pnPaperAllHeight = 10 * 297;
			break;
		case DMPAPER_A5:	//	DMPAPER_A5					A5 sheet�A148 �~ 210 mm
			*pnPaperAllWidth =  10 * 148;
			*pnPaperAllHeight = 10 * 210;
			break;
		case DMPAPER_B4:	//	DMPAPER_B4					B4 sheet�A250 �~ 354 mm
			*pnPaperAllWidth =  10 * 250;
			*pnPaperAllHeight = 10 * 354;
			break;
		case DMPAPER_B5:	//	DMPAPER_B5					B5 sheet�A182 �~ 257 mm
			*pnPaperAllWidth =  10 * 182;
			*pnPaperAllHeight = 10 * 257;
			break;
		case DMPAPER_QUARTO:	//	DMPAPER_QUARTO			Quarto�A215 �~ 275 mm
			*pnPaperAllWidth =  10 * 215;
			*pnPaperAllHeight = 10 * 275;
			break;
		case DMPAPER_ENV_DL:	//	DMPAPER_ENV_DL			DL Envelope�A110 �~ 220 mm
			*pnPaperAllWidth =  10 * 110;
			*pnPaperAllHeight = 10 * 220;
			break;
		case DMPAPER_ENV_C5:	//	DMPAPER_ENV_C5			C5 Envelope�A162 �~ 229 mm
			*pnPaperAllWidth =  10 * 162;
			*pnPaperAllHeight = 10 * 229;
			break;
		case DMPAPER_ENV_C3:	//	DMPAPER_ENV_C3			C3 Envelope�A324 �~ 458 mm
			*pnPaperAllWidth =  10 * 324;
			*pnPaperAllHeight = 10 * 458;
			break;
		case DMPAPER_ENV_C4:	//	DMPAPER_ENV_C4			C4 Envelope�A229 �~ 324 mm
			*pnPaperAllWidth =  10 * 229;
			*pnPaperAllHeight = 10 * 324;
			break;
		case DMPAPER_ENV_C6:	//	DMPAPER_ENV_C6			C6 Envelope�A114 �~ 162 mm
			*pnPaperAllWidth =  10 * 114;
			*pnPaperAllHeight = 10 * 162;
			break;
		case DMPAPER_ENV_C65:	//	DMPAPER_ENV_C65			C65 Envelope�A114 �~ 229 mm
			*pnPaperAllWidth =  10 * 114;
			*pnPaperAllHeight = 10 * 229;
			break;
		case DMPAPER_ENV_B4:	//	DMPAPER_ENV_B4			B4 Envelope�A250 �~ 353 mm
			*pnPaperAllWidth =  10 * 250;
			*pnPaperAllHeight = 10 * 353;
			break;
		case DMPAPER_ENV_B5:	//	DMPAPER_ENV_B5			B5 Envelope�A176 �~ 250 mm
			*pnPaperAllWidth =  10 * 176;
			*pnPaperAllHeight = 10 * 250;
			break;
		case DMPAPER_ENV_B6:	//	DMPAPER_ENV_B6			B6 Envelope�A176 �~ 125 mm
			*pnPaperAllWidth =  10 * 176;
			*pnPaperAllHeight = 10 * 125;
			break;
		case DMPAPER_ENV_ITALY:	//	DMPAPER_ENV_ITALY		Italy Envelope�A110 �~ 230 mm
			*pnPaperAllWidth =  10 * 110;
			*pnPaperAllHeight = 10 * 230;
			break;
		case DMPAPER_FOLIO:	//	DMPAPER_FOLIO				Folio�A8 1/2 �~ 13 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3302;
			break;
		case DMPAPER_CSHEET:	//	DMPAPER_CSHEET			C sheet�A17 �~ 22 inch
			*pnPaperAllWidth = 4318;
			*pnPaperAllHeight = 5588;
			break;
		case DMPAPER_DSHEET:	//	DMPAPER_DSHEET			D sheet�A22 �~ 34 inch
			*pnPaperAllWidth = 5588;
			*pnPaperAllHeight = 8634;
			break;
		case DMPAPER_ESHEET:	//	DMPAPER_ESHEET			E sheet�A34 �~ 44 inch
			*pnPaperAllWidth = 8634;
			*pnPaperAllHeight = 11176;
			break;
		case DMPAPER_LETTERSMALL:	//	DMPAPER_LETTERSMALL	Letter Small�A8 1/2 �~ 11 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_TABLOID:	//	DMPAPER_TABLOID			Tabloid�A11 �~ 17 inch
			*pnPaperAllWidth = 2794;
			*pnPaperAllHeight = 4318;
			break;
		case DMPAPER_LEDGER:	//	DMPAPER_LEDGER			Ledger�A17 �~ 11 inch
			*pnPaperAllWidth = 4318;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_STATEMENT:	//	DMPAPER_STATEMENT		Statement�A5 1/2 �~ 8 1/2 inch
			*pnPaperAllWidth = 1397;
			*pnPaperAllHeight =  2159;
			break;
		case DMPAPER_EXECUTIVE:	//	DMPAPER_EXECUTIVE		Executive�A7 1/4 �~ 10 1/2 inch
			*pnPaperAllWidth = 1841;
			*pnPaperAllHeight = 2667;
			break;
		case DMPAPER_LETTER:	//	DMPAPER_LETTER			Letter�A8 1/2 �~ 11 inch
			*pnPaperAllWidth = 2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_LEGAL:	//	MPAPER_LEGAL				Legal�A8 1/2 �~ 14 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3556;
			break;
		case DMPAPER_10X14:	//	DMPAPER_10X14				10 �~ 14 inch sheet
			*pnPaperAllWidth = 2540;
			*pnPaperAllHeight = 3556;
			break;
		case DMPAPER_11X17:	//	DMPAPER_11X17				11 �~ 17 inch sheet
			*pnPaperAllWidth = 2794;
			*pnPaperAllHeight = 4318;
			break;
		case DMPAPER_NOTE:	//	DMPAPER_NOTE				Note�A 8 1/2 �~ 11 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_ENV_9:	//	DMPAPER_ENV_9				#9 Envelope�A3 7/8 �~ 8 7/8 inch
			*pnPaperAllWidth = 984;
			*pnPaperAllHeight = 2254;
			break;
		case DMPAPER_ENV_10:	//	DMPAPER_ENV_10			#10 Envelope�A4 1/8 �~ 9 1/2 inch
			*pnPaperAllWidth = 1047;
			*pnPaperAllHeight = 2413;
			break;
		case DMPAPER_ENV_11:	//	DMPAPER_ENV_11			#11 Envelope�A4 1/2 �~ 10 3/8 inch
			*pnPaperAllWidth = 1143;
			*pnPaperAllHeight = 2635;
			break;
		case DMPAPER_ENV_12:	//	DMPAPER_ENV_12			#12 Envelope�A4 3/4 �~ 11 inch
			*pnPaperAllWidth = 1206;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_ENV_14:	//	DMPAPER_ENV_14			#14 Envelope�A5 �~ 11 1/2 inch
			*pnPaperAllWidth = 1270;
			*pnPaperAllHeight = 2921;
			break;
		case DMPAPER_ENV_MONARCH:	//	DMPAPER_ENV_MONARCH	Monarch Envelope�A3 7/8 �~ 7 1/2 inch
			*pnPaperAllWidth = 984;
			*pnPaperAllHeight = 1905;
			break;
		case DMPAPER_ENV_PERSONAL:	//	DMPAPER_ENV_PERSONAL	6 3/4 Envelope�A3 5/8 �~ 6 1/2 inch
			*pnPaperAllWidth = 920;
			*pnPaperAllHeight = 1651;
			break;
		case DMPAPER_FANFOLD_US:	//	DMPAPER_FANFOLD_US	US Std Fanfold�A14 7/8 �~ 11 inch
			*pnPaperAllWidth = 3778;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_FANFOLD_STD_GERMAN:	//	DMPAPER_FANFOLD_STD_GERMAN	German Std Fanfold�A8 1/2 �~ 12 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3048;
			break;
		case DMPAPER_FANFOLD_LGL_GERMAN:	//	DMPA PER_FANFOLD_LGL_GERMAN	German Legal Fanfold�A8 1/2 �~ 13 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3302;
			break;
		default:
		// 2001.12.21 hor �}�E�X�ŃN���b�N�����܂܃��X�g�O�ɏo��Ƃ����ɂ��邯�ǁA
		//	�ُ�ł͂Ȃ��̂� FALSE ��Ԃ����Ƃɂ���
		//	::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
		//	"�s���ȗp���B�����ł��܂���B\n�v���O�����o�O�B\n%s"
		//	);
			return FALSE;
		}
	}
	if(pDEVMODE->dmFields & DM_PAPERLENGTH && 0 != pDEVMODE->dmPaperLength ){
		/* pDEVMODE->dmPaperLength��1/10mm�P�ʂł��� */
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	}
	if(pDEVMODE->dmFields & DM_PAPERWIDTH && 0 != pDEVMODE->dmPaperWidth ){
		/* pDEVMODE->dmPaperWidth��1/10mm�P�ʂł��� */
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
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
	char*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HANDLE*		phPrinter,
	HDC*		phdc,
	char*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
)
{
	BOOL		bRet;
	HDC			hdc;
	HANDLE		hPrinter;
	HGLOBAL		hgDEVMODE;
	DEVMODE*	pDEVMODE;
	int			nDEVMODE_Size;
	DOCINFO		di;
	bRet = TRUE;
	//
	//OpenPrinter()�ŁA�f�o�C�X���Ńv�����^�n���h�����擾
	//
	if( FALSE == ::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		&hPrinter,	/* �v�����^�n���h���̃|�C���^ */
		(PRINTER_DEFAULTS*)NULL
	) ){
		wsprintf( pszErrMsg,
			"OpenPrinter()�Ɏ��s�B\n�v�����^�f�o�C�X��=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	//
	//DocumentProperties()��DEVMODE�\���̂ɕK�v�ȃ������T�C�Y���擾
	//�iDEVMODE�\���̂̓v�����^�h���C�o���Ƃɒǉ��G���A�̃T�C�Y���قȂ�j
	//�K�v�ȃT�C�Y�̃��������m�ۂ��A���b�N����
	//
	if( 0 > ( nDEVMODE_Size = ::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		NULL,
		NULL,
		0
	) ) ){
		wsprintf( pszErrMsg,
			"DocumentProperties()�Ɏ��s�B\n�v�����^�f�o�C�X��=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	hgDEVMODE = ::GlobalAlloc( GHND, nDEVMODE_Size );
	pDEVMODE = (DEVMODE*)::GlobalLock( hgDEVMODE );
	//
	//DocumentProperties()�Ō��݂̃v�����^�ݒ��DEVMODE�\���̂Ɏ擾
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */,
		pDEVMODE,
		NULL,
		DM_OUT_BUFFER
	);

	//
	//  ���݂̃v�����^�ݒ�̕K�v������ύX
	//
	pDEVMODE->dmFields				|= pMYDEVMODE->dmFields;
// From Here 2001.12.03 hor
//
// ����Ŏ����������Ɏw�肵�Ă��c�����Ɉ������� �E�E�E �̏C��
//
//	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
//	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
//	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
//	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;
//	pMYDEVMODE->dmScale				= pDEVMODE->dmScale;
//	pMYDEVMODE->dmCopies			= pDEVMODE->dmCopies;
//	pMYDEVMODE->dmDefaultSource		= pDEVMODE->dmDefaultSource;
//	pMYDEVMODE->dmPrintQuality		= pDEVMODE->dmPrintQuality;
//	pMYDEVMODE->dmColor				= pDEVMODE->dmColor;
//	pMYDEVMODE->dmDuplex			= pDEVMODE->dmDuplex;
//	pMYDEVMODE->dmYResolution		= pDEVMODE->dmYResolution;
//	pMYDEVMODE->dmTTOption			= pDEVMODE->dmTTOption;
//	pMYDEVMODE->dmCollate			= pDEVMODE->dmCollate;
//	pMYDEVMODE->dmLogPixels			= pDEVMODE->dmLogPixels;
//	pMYDEVMODE->dmBitsPerPel		= pDEVMODE->dmBitsPerPel;
//	pMYDEVMODE->dmPelsWidth			= pDEVMODE->dmPelsWidth;
//	pMYDEVMODE->dmPelsHeight		= pDEVMODE->dmPelsHeight;
//	pMYDEVMODE->dmDisplayFlags		= pDEVMODE->dmDisplayFlags;
//	pMYDEVMODE->dmDisplayFrequency  = pDEVMODE->dmDisplayFrequency;
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;
	pDEVMODE->dmScale				= pMYDEVMODE->dmScale;
	pDEVMODE->dmCopies				= pMYDEVMODE->dmCopies;
	pDEVMODE->dmDefaultSource		= pMYDEVMODE->dmDefaultSource;
	pDEVMODE->dmPrintQuality		= pMYDEVMODE->dmPrintQuality;
	pDEVMODE->dmColor				= pMYDEVMODE->dmColor;
	pDEVMODE->dmDuplex				= pMYDEVMODE->dmDuplex;
	pDEVMODE->dmYResolution			= pMYDEVMODE->dmYResolution;
	pDEVMODE->dmTTOption			= pMYDEVMODE->dmTTOption;
	pDEVMODE->dmCollate				= pMYDEVMODE->dmCollate;
	pDEVMODE->dmLogPixels			= pMYDEVMODE->dmLogPixels;
	pDEVMODE->dmBitsPerPel			= pMYDEVMODE->dmBitsPerPel;
	pDEVMODE->dmPelsWidth			= pMYDEVMODE->dmPelsWidth;
	pDEVMODE->dmPelsHeight			= pMYDEVMODE->dmPelsHeight;
	pDEVMODE->dmDisplayFlags		= pMYDEVMODE->dmDisplayFlags;
	pDEVMODE->dmDisplayFrequency	= pMYDEVMODE->dmDisplayFrequency;
// To Here 2001.12.03 hor
	strcpy( (char *)pDEVMODE->dmFormName, (char *)pMYDEVMODE->dmFormName );

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
		(LPCTSTR)pMYDEVMODE->m_szPrinterDriverName,	/* �v�����^�h���C�o�� */
		(LPCTSTR)pMYDEVMODE->m_szPrinterDeviceName,	/* �v�����^�f�o�C�X�� */
		(LPCTSTR)pMYDEVMODE->m_szPrinterOutputName,	/* �v�����^�|�[�g�� */
		pDEVMODE
	);
	::GlobalUnlock( hgDEVMODE );
	::GlobalFree( hgDEVMODE );

	/* �}�b�s���O ���[�h�̐ݒ� */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_HIMETRIC		���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂��B
//	::SetMapMode( hdc, MM_ANISOTROPIC ); //MM_ANISOTROPIC	�_���P�ʂ́A�C�ӂɃX�P�[�����O���ꂽ����̔C�ӂ̒P�ʂɃ}�b�v����܂��B


	//
	//  ����W���u�J�n
	//
	memset( (void *) &di, 0, sizeof( DOCINFO ) );
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = pszJobName;
	di.lpszOutput  = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if( 0 >= ::StartDoc( hdc, &di ) ){
		wsprintf( pszErrMsg,
			"StartDoc()�Ɏ��s�B\n�v�����^�f�o�C�X��=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* �v�����^�f�o�C�X�� */
		);
		bRet = FALSE;
		goto end_of_func;
	}

	*phPrinter = hPrinter;
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
void CPrint::PrintClose( HANDLE hPrinter, HDC hdc )
{
	::EndDoc( hdc );
	::DeleteDC( hdc );
	::ClosePrinter( hPrinter );
}

/*[EOF]*/




/* �p���̖��O���擾 */
char* CPrint::GetPaperName( int nPaperSize, char* pszPaperName )
{
	char*	pszPaperNameArr[] = {
		"A4 (210x297 mm)",
		"A3 (297x420 mm)",
		"A4 small(210x297 mm)",
		"A5 (148x210 mm)",
		"B4 (250x354 mm)",
		"B5 (182x257 mm)",
		"Quarto(215x275 mm)",
		"DL Envelope(110x220 mm)",
		"C5 Envelope(162x229 mm)",
		"C3 Envelope(324x458 mm)",
		"C4 Envelope(229x324 mm)",
		"C6 Envelope(114x162 mm)",
		"C65 Envelope(114x229 mm)",
		"B4 Envelope(250x353 mm)",
		"B5 Envelope(176x250 mm)",
		"B6 Envelope(176x125 mm)",
		"Italy Envelope(110x230 mm)",
		"Letter�A8 1/2x11 inch",
		"Legal�A8 1/2x14 inch",
		"C sheet�A17x22 inch",
		"D sheet�A22x34 inch",
		"E sheet�A34x44 inch",
		"Letter Small�A8 1/2x11 inch",
		"Tabloid�A11x17 inch",
		"Ledger�A17x11 inch",
		"Statement�A5 1/2x8 1/2 inch",
		"Executive�A7 1/4x10 1/2 inch",
		"Folio�A8 1/2x13 inch",
		"10x14 inch sheet",
		"11x17 inch sheet",
		"Note�A 8 1/2x11 inch",
		"#9 Envelope�A3 7/8x8 7/8 inch",
		"#10 Envelope�A4 1/8x9 1/2 inch",
		"#11 Envelope�A4 1/2x10 3/8 inch",
		"#12 Envelope�A4 3/4x11 inch",
		"#14 Envelope�A5x11 1/2 inch",
		"Monarch Envelope�A3 7/8x7 1/2 inch",
		"6 3/4 Envelope�A3 5/8x6 1/2 inch",
		"US Std Fanfold�A14 7/8x11 inch",
		"German Std Fanfold�A8 1/2x12 inch",
		"German Legal Fanfold�A8 1/2x13 inch",
	};
	int		nPaerNameArrNum = sizeof( pszPaperNameArr ) / sizeof( pszPaperNameArr[0] );
	int		nPaperIdArr[] = {
		DMPAPER_A4		,
		DMPAPER_A3		,
		DMPAPER_A4SMALL	,
		DMPAPER_A5		,
		DMPAPER_B4		,
		DMPAPER_B5		,
		DMPAPER_QUARTO	,
		DMPAPER_ENV_DL	,
		DMPAPER_ENV_C5	,
		DMPAPER_ENV_C3	,
		DMPAPER_ENV_C4	,
		DMPAPER_ENV_C6	,
		DMPAPER_ENV_C65	,
		DMPAPER_ENV_B4	,
		DMPAPER_ENV_B5	,
		DMPAPER_ENV_B6	,
		DMPAPER_ENV_ITALY,
		DMPAPER_LETTER,
		DMPAPER_LEGAL,
		DMPAPER_CSHEET,
		DMPAPER_DSHEET,
		DMPAPER_ESHEET,
		DMPAPER_LETTERSMALL,
		DMPAPER_TABLOID,
		DMPAPER_LEDGER,
		DMPAPER_STATEMENT,
		DMPAPER_EXECUTIVE,
		DMPAPER_FOLIO,
		DMPAPER_10X14,
		DMPAPER_11X17,
		DMPAPER_NOTE,
		DMPAPER_ENV_9,
		DMPAPER_ENV_10,
		DMPAPER_ENV_11,
		DMPAPER_ENV_12,
		DMPAPER_ENV_14,
		DMPAPER_ENV_MONARCH,
		DMPAPER_ENV_PERSONAL,
		DMPAPER_FANFOLD_US,
		DMPAPER_FANFOLD_STD_GERMAN,
		DMPAPER_FANFOLD_LGL_GERMAN
	};
	int i;
	for( i = 0; i < nPaerNameArrNum; ++i ){
		if( nPaperIdArr[i] == nPaperSize ){
			break;
		}
	}
	if( i < nPaerNameArrNum ){
		strcpy( pszPaperName, pszPaperNameArr[i] );
	}else{
		strcpy( pszPaperName, "�s��" );
	}
	return pszPaperName;



}

void CPrint::Initialize()
{
	/* ���Ԃ�������Ȃ����̂��������� */
	int i = 0;
	DLLSHAREDATA*	m_pShareData = CShareData::getInstance()->GetShareData();
	wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "����ݒ� %d", i + 1 );	/* ����ݒ�̖��O */
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan, "�l�r ����" );				/* ����t�H���g */
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen, "�l�r ����" );				/* ����t�H���g */
	m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth = 12;  								/* ����t�H���g��(1/10mm�P��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight = m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth * 2;	/* ����t�H���g����(1/10mm�P�ʒP��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu = 1;			/* �i�g�̒i�� */
	m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace = 70; 		/* �i�ƒi�̌���(1/10mm) */
	m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap = TRUE;		/* �p�����[�h���b�v���� */
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuHead = FALSE;		/* �s���֑����� */	//@@@ 2002.04.09 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuTail = FALSE;		/* �s���֑����� */	//@@@ 2002.04.09 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuRet = FALSE;		/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber = FALSE;	/* �s�ԍ���������� */
	m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing = 30;	/* ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY = 100;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX = 100;		/* ����p���}�[�W�� �E(1/10mm�P��) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize = DMPAPER_A4;	/* �p���T�C�Y */
	/* �v�����^�ݒ� DEVMODE�p */
	/* �v�����^�ݒ���擾����̂̓R�X�g��������̂ŁA��ق� */
	//	m_cPrint.GetDefaultPrinterInfo( &(m_pShareData->m_PrintSettingArr[i].m_mdmDevMode) );
	m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0] = TRUE;
	m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1] = FALSE;
	m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2] = FALSE;
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0], "$f" );
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[1], "" );
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[2], "" );
	m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0] = TRUE;
	m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1] = FALSE;
	m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2] = FALSE;
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0], "" );
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[1], "- $p -" );
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[2], "" );
	for( i = 1; i < MAX_PRINTSETTINGARR; ++i ){
		m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
		wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "����ݒ� %d", i + 1 );	/* ����ݒ�̖��O */
	}
}

/*[EOF]*/
