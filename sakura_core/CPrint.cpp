//	$Id$
/*!	@file
	@brief 印刷

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



/* デフォルトのプリンタ情報を取得 */
BOOL CPrint::GetDefaultPrinter( PRINTDLG* pPRINTDLG )
{
	//
	// PRINTDLG構造体を初期化する（ダイアログは表示しないように）
	// PrintDlg()でデフォルトプリンタのデバイス名などを取得する
	//
	memset ( (void *)pPRINTDLG, 0, sizeof(PRINTDLG) );
	pPRINTDLG->lStructSize	= sizeof(PRINTDLG);
	pPRINTDLG->hwndOwner	= NULL;
	pPRINTDLG->Flags		= PD_RETURNDEFAULT;
	pPRINTDLG->hInstance	= NULL;
	return ::PrintDlg( pPRINTDLG );
}


/* デフォルトのプリンタ情報を取得 */
BOOL CPrint::GetPrinter( PRINTDLG* pPRINTDLG )
{
	//
	// PRINTDLG構造体を初期化する（ダイアログは表示しないように）
	// PrintDlg()でデフォルトプリンタのデバイス名などを取得する
	//
	memset ( (void *)pPRINTDLG, 0, sizeof(PRINTDLG) );
	pPRINTDLG->lStructSize	= sizeof(PRINTDLG);
	return ::PrintDlg( pPRINTDLG );
}


/* デフォルトのプリンタ設定 MYDEVMODE を取得 */
BOOL CPrint::GetPrinterInfo( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* プリンタ設定 DEVNAMES用*/

	/* 初期化 */
	memset( (void *)pMYDEVMODE, 0, sizeof(MYDEVMODE) );

	/* デフォルトのプリンタ情報を取得 */
	if( FALSE == GetPrinter( &pd ) ){
		pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* プリンタがなかったフラグ */
		return FALSE;
	}
	pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* プリンタがなかったフラグ */

	pDEVMODE = (DEVMODE*)::GlobalLock( pd.hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( pd.hDevNames );

	strcpy( pMYDEVMODE->m_szPrinterDriverName, (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );	/* プリンタドライバ名 */
	strcpy( pMYDEVMODE->m_szPrinterDeviceName, (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );	/* プリンタデバイス名 */
	strcpy( pMYDEVMODE->m_szPrinterOutputName, (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );	/* プリンタポート名 */

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
	MYTRACE( " (入力/出力) デバイス ドライバ=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE( " (入力/出力) デバイス名=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE( "物理出力メディア (出力ポート) =[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE( "デフォルトのプリンタか=[%d]\n", pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( pd.hDevMode );
	::GlobalUnlock( pd.hDevNames );
	return TRUE;
}

/* デフォルトのプリンタ設定 MYDEVMODE を取得 */
BOOL CPrint::GetDefaultPrinterInfo( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* プリンタ設定 DEVNAMES用*/

	/* 初期化 */
	memset( (void *)pMYDEVMODE, 0, sizeof(MYDEVMODE) );

	/* デフォルトのプリンタ情報を取得 */
	if( FALSE == GetDefaultPrinter( &pd ) ){
		pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* プリンタがなかったフラグ */
		return FALSE;
	}
	pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* プリンタがなかったフラグ */

	pDEVMODE = (DEVMODE*)::GlobalLock( pd.hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( pd.hDevNames );

	strcpy( pMYDEVMODE->m_szPrinterDriverName, (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );	/* プリンタドライバ名 */
	strcpy( pMYDEVMODE->m_szPrinterDeviceName, (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );	/* プリンタデバイス名 */
	strcpy( pMYDEVMODE->m_szPrinterOutputName, (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );	/* プリンタポート名 */

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
	MYTRACE( " (入力/出力) デバイス ドライバ=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	MYTRACE( " (入力/出力) デバイス名=[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	MYTRACE( "物理出力メディア (出力ポート) =[%s]\n", (char*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	MYTRACE( "デフォルトのプリンタか=[%d]\n", pDEVNAMES->wDefault );
#endif

	::GlobalUnlock( pd.hDevMode );
	::GlobalUnlock( pd.hDevNames );
	return TRUE;
}


/* 印刷/プレビューに必要な情報を取得 */
BOOL CPrint::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
//	LOGFONT*	pLOGFONT,
//	int			nMarginTY,			/* マージン 上 */
//	int			nMarginBY,			/* マージン 下 */
//	int			nMarginLX,			/* マージン 左 */
//	int			nMarginRX,			/* マージン 右 */
//	int			nLineSpacing,		/* 行間 文字の高さに対する割合 */

	int*		pnPaperAllWidth,	/* 用紙幅 */
	int*		pnPaperAllHeight,	/* 用紙高さ */
	int*		pnPaperWidth,		/* 用紙印刷可能幅 */
	int*		pnPaperHeight,		/* 用紙印刷可能高さ */
	int*		pnPaperOffsetLeft,	/* 用紙余白左端 */
	int*		pnPaperOffsetTop,	/* 用紙余白上端 */
//	int*		pnCharWidth,	/* 文字幅 */
//	int*		pnCharHeight,	/* 文字高さ */
//	int*		pnAllChars,		/* 横方向に印字可能な桁数 */
//	int*		pnAllLines,		/* 縦方向に印字可能な行数 */
	char*		pszErrMsg		/* エラーメッセージ格納場所 */
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
	// OpenPrinter()で、デバイス名でプリンタハンドルを取得
	//
	if( FALSE == ::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		&hPrinter,	/* プリンタハンドルのポインタ */
		(PRINTER_DEFAULTS*)NULL
	) ){
		wsprintf( pszErrMsg,
			"OpenPrinter()に失敗。\nプリンタデバイス名=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	//
	//DocumentProperties()でDEVMODE構造体に必要なメモリサイズを取得
	//（DEVMODE構造体はプリンタドライバごとに追加エリアのサイズが異なる）
	// 必要なサイズのメモリを確保し、ロックする
	//
	if( 0 > ( nDEVMODE_Size = ::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		NULL,
		NULL,
		0
	) ) ){
		wsprintf( pszErrMsg,
			"DocumentProperties()に失敗。\nプリンタデバイス名=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	hgDEVMODE = ::GlobalAlloc( GHND, nDEVMODE_Size );
	pDEVMODE = (DEVMODE*)::GlobalLock( hgDEVMODE );
	//
	//DocumentProperties()で現在のプリンタ設定をDEVMODE構造体に取得
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */,
		pDEVMODE,
		NULL,
		DM_OUT_BUFFER
	);

	//
	//  現在のプリンタ設定の必要部分を変更
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
	//DocumentProperties()でアプリケーション独自のプリンタ設定に変更する
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */,
		pDEVMODE,
		pDEVMODE,
		DM_OUT_BUFFER | DM_IN_BUFFER
	);
	/* 指定デバイスに対するデバイス コンテキストを作成します。 */
	hdc = ::CreateDC(
		(LPCTSTR)pMYDEVMODE->m_szPrinterDriverName,	/* プリンタドライバ名 */
		(LPCTSTR)pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		(LPCTSTR)pMYDEVMODE->m_szPrinterOutputName,	/* プリンタポート名 */
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

	/* マッピング モードの設定 */
//	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC	それぞれの論理単位は 0.1 mm にマップされます。
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_HIMETRIC	それぞれの論理単位は 0.01 mm にマップされます。



	/* 最小左マージンと最小上マージンを取得(1mm単位) */
	POINT	po;
	if( 0 < ::Escape( hdc, GETPRINTINGOFFSET, (int)NULL, NULL, (LPPOINT)&po ) ){
		::DPtoLP( hdc, &po, 1 );
		*pnPaperOffsetLeft = abs( po.x );	/* 用紙余白左端 */
		*pnPaperOffsetTop = abs( po.y );	/* 用紙余白上端 */
	}else{
		*pnPaperOffsetLeft = 0;	/* 用紙余白左端 */
		*pnPaperOffsetTop = 0;	/* 用紙余白上端 */
	}


//	hFont = ::CreateFontIndirect( pLOGFONT );
//	hFontOld = ::SelectObject( hdc, hFont );

	/* 用紙の印刷可能な幅、高さ */
	po.x = ::GetDeviceCaps( hdc, HORZRES );	/* 用紙印刷可能幅←物理ディスプレイの幅 (mm 単位) */
	po.y = ::GetDeviceCaps( hdc, VERTRES );	/* 用紙印刷可能高さ←物理ディスプレイの高さ (mm 単位)  */
	::DPtoLP( hdc, &po, 1 );
	*pnPaperWidth = abs( po.x );
	*pnPaperHeight = abs( po.y );

	/* 用紙の幅、高さ */
	if( FALSE == GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);

//		bRet = FALSE;
		bRet = TRUE;
	}



//	::GetTextMetrics( hdc, &tm );
//	*pnCharWidth = abs( tm.tmAveCharWidth );	/* 文字幅 */
//	*pnCharHeight = abs( tm.tmHeight + tm.tmExternalLeading );	/* 文字高さ */


//	::SelectObject( hdc, hFontOld );
//	::DeleteObject( hFont );
	::DeleteDC( hdc );
	::ClosePrinter( hPrinter );

end_of_func:;

	return bRet;
}



/* 用紙の幅、高さ */
BOOL CPrint::GetPaperSize(
	int*		pnPaperAllWidth,
	int*		pnPaperAllHeight,
	DEVMODE*	pDEVMODE
)
{
	int	nWork;


	if( pDEVMODE->dmFields &  DM_PAPERSIZE ){
		switch( pDEVMODE->dmPaperSize ){
		case DMPAPER_A4:	//	DMPAPER_A4					A4 sheet、210 × 297 mm
			*pnPaperAllWidth =  10 * 210;
			*pnPaperAllHeight = 10 * 297;
			break;
		case DMPAPER_A3:	//	DMPAPER_A3					A3 sheet、297 × 420 mm
			*pnPaperAllWidth =  10 * 297;
			*pnPaperAllHeight = 10 * 420;
			break;
		case DMPAPER_A4SMALL:	//	DMPAPER_A4SMALL			A4 small sheet、210 × 297 mm
			*pnPaperAllWidth =  10 * 210;
			*pnPaperAllHeight = 10 * 297;
			break;
		case DMPAPER_A5:	//	DMPAPER_A5					A5 sheet、148 × 210 mm
			*pnPaperAllWidth =  10 * 148;
			*pnPaperAllHeight = 10 * 210;
			break;
		case DMPAPER_B4:	//	DMPAPER_B4					B4 sheet、250 × 354 mm
			*pnPaperAllWidth =  10 * 250;
			*pnPaperAllHeight = 10 * 354;
			break;
		case DMPAPER_B5:	//	DMPAPER_B5					B5 sheet、182 × 257 mm
			*pnPaperAllWidth =  10 * 182;
			*pnPaperAllHeight = 10 * 257;
			break;
		case DMPAPER_QUARTO:	//	DMPAPER_QUARTO			Quarto、215 × 275 mm
			*pnPaperAllWidth =  10 * 215;
			*pnPaperAllHeight = 10 * 275;
			break;
		case DMPAPER_ENV_DL:	//	DMPAPER_ENV_DL			DL Envelope、110 × 220 mm
			*pnPaperAllWidth =  10 * 110;
			*pnPaperAllHeight = 10 * 220;
			break;
		case DMPAPER_ENV_C5:	//	DMPAPER_ENV_C5			C5 Envelope、162 × 229 mm
			*pnPaperAllWidth =  10 * 162;
			*pnPaperAllHeight = 10 * 229;
			break;
		case DMPAPER_ENV_C3:	//	DMPAPER_ENV_C3			C3 Envelope、324 × 458 mm
			*pnPaperAllWidth =  10 * 324;
			*pnPaperAllHeight = 10 * 458;
			break;
		case DMPAPER_ENV_C4:	//	DMPAPER_ENV_C4			C4 Envelope、229 × 324 mm
			*pnPaperAllWidth =  10 * 229;
			*pnPaperAllHeight = 10 * 324;
			break;
		case DMPAPER_ENV_C6:	//	DMPAPER_ENV_C6			C6 Envelope、114 × 162 mm
			*pnPaperAllWidth =  10 * 114;
			*pnPaperAllHeight = 10 * 162;
			break;
		case DMPAPER_ENV_C65:	//	DMPAPER_ENV_C65			C65 Envelope、114 × 229 mm
			*pnPaperAllWidth =  10 * 114;
			*pnPaperAllHeight = 10 * 229;
			break;
		case DMPAPER_ENV_B4:	//	DMPAPER_ENV_B4			B4 Envelope、250 × 353 mm
			*pnPaperAllWidth =  10 * 250;
			*pnPaperAllHeight = 10 * 353;
			break;
		case DMPAPER_ENV_B5:	//	DMPAPER_ENV_B5			B5 Envelope、176 × 250 mm
			*pnPaperAllWidth =  10 * 176;
			*pnPaperAllHeight = 10 * 250;
			break;
		case DMPAPER_ENV_B6:	//	DMPAPER_ENV_B6			B6 Envelope、176 × 125 mm
			*pnPaperAllWidth =  10 * 176;
			*pnPaperAllHeight = 10 * 125;
			break;
		case DMPAPER_ENV_ITALY:	//	DMPAPER_ENV_ITALY		Italy Envelope、110 × 230 mm
			*pnPaperAllWidth =  10 * 110;
			*pnPaperAllHeight = 10 * 230;
			break;
		case DMPAPER_FOLIO:	//	DMPAPER_FOLIO				Folio、8 1/2 × 13 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3302;
			break;
		case DMPAPER_CSHEET:	//	DMPAPER_CSHEET			C sheet、17 × 22 inch
			*pnPaperAllWidth = 4318;
			*pnPaperAllHeight = 5588;
			break;
		case DMPAPER_DSHEET:	//	DMPAPER_DSHEET			D sheet、22 × 34 inch
			*pnPaperAllWidth = 5588;
			*pnPaperAllHeight = 8634;
			break;
		case DMPAPER_ESHEET:	//	DMPAPER_ESHEET			E sheet、34 × 44 inch
			*pnPaperAllWidth = 8634;
			*pnPaperAllHeight = 11176;
			break;
		case DMPAPER_LETTERSMALL:	//	DMPAPER_LETTERSMALL	Letter Small、8 1/2 × 11 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_TABLOID:	//	DMPAPER_TABLOID			Tabloid、11 × 17 inch
			*pnPaperAllWidth = 2794;
			*pnPaperAllHeight = 4318;
			break;
		case DMPAPER_LEDGER:	//	DMPAPER_LEDGER			Ledger、17 × 11 inch
			*pnPaperAllWidth = 4318;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_STATEMENT:	//	DMPAPER_STATEMENT		Statement、5 1/2 × 8 1/2 inch
			*pnPaperAllWidth = 1397;
			*pnPaperAllHeight =  2159;
			break;
		case DMPAPER_EXECUTIVE:	//	DMPAPER_EXECUTIVE		Executive、7 1/4 × 10 1/2 inch
			*pnPaperAllWidth = 1841;
			*pnPaperAllHeight = 2667;
			break;
		case DMPAPER_LETTER:	//	DMPAPER_LETTER			Letter、8 1/2 × 11 inch
			*pnPaperAllWidth = 2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_LEGAL:	//	MPAPER_LEGAL				Legal、8 1/2 × 14 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3556;
			break;
		case DMPAPER_10X14:	//	DMPAPER_10X14				10 × 14 inch sheet
			*pnPaperAllWidth = 2540;
			*pnPaperAllHeight = 3556;
			break;
		case DMPAPER_11X17:	//	DMPAPER_11X17				11 × 17 inch sheet
			*pnPaperAllWidth = 2794;
			*pnPaperAllHeight = 4318;
			break;
		case DMPAPER_NOTE:	//	DMPAPER_NOTE				Note、 8 1/2 × 11 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_ENV_9:	//	DMPAPER_ENV_9				#9 Envelope、3 7/8 × 8 7/8 inch
			*pnPaperAllWidth = 984;
			*pnPaperAllHeight = 2254;
			break;
		case DMPAPER_ENV_10:	//	DMPAPER_ENV_10			#10 Envelope、4 1/8 × 9 1/2 inch
			*pnPaperAllWidth = 1047;
			*pnPaperAllHeight = 2413;
			break;
		case DMPAPER_ENV_11:	//	DMPAPER_ENV_11			#11 Envelope、4 1/2 × 10 3/8 inch
			*pnPaperAllWidth = 1143;
			*pnPaperAllHeight = 2635;
			break;
		case DMPAPER_ENV_12:	//	DMPAPER_ENV_12			#12 Envelope、4 3/4 × 11 inch
			*pnPaperAllWidth = 1206;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_ENV_14:	//	DMPAPER_ENV_14			#14 Envelope、5 × 11 1/2 inch
			*pnPaperAllWidth = 1270;
			*pnPaperAllHeight = 2921;
			break;
		case DMPAPER_ENV_MONARCH:	//	DMPAPER_ENV_MONARCH	Monarch Envelope、3 7/8 × 7 1/2 inch
			*pnPaperAllWidth = 984;
			*pnPaperAllHeight = 1905;
			break;
		case DMPAPER_ENV_PERSONAL:	//	DMPAPER_ENV_PERSONAL	6 3/4 Envelope、3 5/8 × 6 1/2 inch
			*pnPaperAllWidth = 920;
			*pnPaperAllHeight = 1651;
			break;
		case DMPAPER_FANFOLD_US:	//	DMPAPER_FANFOLD_US	US Std Fanfold、14 7/8 × 11 inch
			*pnPaperAllWidth = 3778;
			*pnPaperAllHeight = 2794;
			break;
		case DMPAPER_FANFOLD_STD_GERMAN:	//	DMPAPER_FANFOLD_STD_GERMAN	German Std Fanfold、8 1/2 × 12 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3048;
			break;
		case DMPAPER_FANFOLD_LGL_GERMAN:	//	DMPA PER_FANFOLD_LGL_GERMAN	German Legal Fanfold、8 1/2 × 13 inch
			*pnPaperAllWidth =  2159;
			*pnPaperAllHeight = 3302;
			break;
		default:
		// 2001.12.21 hor マウスでクリックしたままリスト外に出るとここにくるけど、
		//	異常ではないので FALSE を返すことにする
		//	::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
		//	"不明な用紙。処理できません。\nプログラムバグ。\n%s"
		//	);
			return FALSE;
		}
	}
	if(pDEVMODE->dmFields & DM_PAPERLENGTH && 0 != pDEVMODE->dmPaperLength ){
		/* pDEVMODE->dmPaperLengthは1/10mm単位である */
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	}
	if(pDEVMODE->dmFields & DM_PAPERWIDTH && 0 != pDEVMODE->dmPaperWidth ){
		/* pDEVMODE->dmPaperWidthは1/10mm単位である */
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
	}
	/* 用紙の方向 */
	if( DMORIENT_LANDSCAPE == pDEVMODE->dmOrientation ){
		nWork = *pnPaperAllWidth;
		*pnPaperAllWidth = *pnPaperAllHeight;
		*pnPaperAllHeight = nWork;
	}
	return TRUE;
}







/* 印刷 ジョブ開始 */
BOOL CPrint::PrintOpen(
	char*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HANDLE*		phPrinter,
	HDC*		phdc,
	char*		pszErrMsg		/* エラーメッセージ格納場所 */
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
	//OpenPrinter()で、デバイス名でプリンタハンドルを取得
	//
	if( FALSE == ::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		&hPrinter,	/* プリンタハンドルのポインタ */
		(PRINTER_DEFAULTS*)NULL
	) ){
		wsprintf( pszErrMsg,
			"OpenPrinter()に失敗。\nプリンタデバイス名=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	//
	//DocumentProperties()でDEVMODE構造体に必要なメモリサイズを取得
	//（DEVMODE構造体はプリンタドライバごとに追加エリアのサイズが異なる）
	//必要なサイズのメモリを確保し、ロックする
	//
	if( 0 > ( nDEVMODE_Size = ::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		NULL,
		NULL,
		0
	) ) ){
		wsprintf( pszErrMsg,
			"DocumentProperties()に失敗。\nプリンタデバイス名=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}
	hgDEVMODE = ::GlobalAlloc( GHND, nDEVMODE_Size );
	pDEVMODE = (DEVMODE*)::GlobalLock( hgDEVMODE );
	//
	//DocumentProperties()で現在のプリンタ設定をDEVMODE構造体に取得
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */,
		pDEVMODE,
		NULL,
		DM_OUT_BUFFER
	);

	//
	//  現在のプリンタ設定の必要部分を変更
	//
	pDEVMODE->dmFields				|= pMYDEVMODE->dmFields;
// From Here 2001.12.03 hor
//
// 印刷で紙を横向きに指定しても縦方向に印刷される ・・・ の修正
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
	//DocumentProperties()でアプリケーション独自のプリンタ設定に変更する
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */,
		pDEVMODE,
		pDEVMODE,
		DM_OUT_BUFFER | DM_IN_BUFFER
	);
	/* 指定デバイスに対するデバイス コンテキストを作成します。 */
	hdc = ::CreateDC(
		(LPCTSTR)pMYDEVMODE->m_szPrinterDriverName,	/* プリンタドライバ名 */
		(LPCTSTR)pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		(LPCTSTR)pMYDEVMODE->m_szPrinterOutputName,	/* プリンタポート名 */
		pDEVMODE
	);
	::GlobalUnlock( hgDEVMODE );
	::GlobalFree( hgDEVMODE );

	/* マッピング モードの設定 */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_HIMETRIC		それぞれの論理単位は、0.01 mm にマップされます。
//	::SetMapMode( hdc, MM_ANISOTROPIC ); //MM_ANISOTROPIC	論理単位は、任意にスケーリングされた軸上の任意の単位にマップされます。


	//
	//  印刷ジョブ開始
	//
	memset( (void *) &di, 0, sizeof( DOCINFO ) );
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = pszJobName;
	di.lpszOutput  = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if( 0 >= ::StartDoc( hdc, &di ) ){
		wsprintf( pszErrMsg,
			"StartDoc()に失敗。\nプリンタデバイス名=[%s]",
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}

	*phPrinter = hPrinter;
	*phdc = hdc;

end_of_func:;

	return bRet;
}



/* 印刷 ページ開始 */
void CPrint::PrintStartPage( HDC hdc )
{
	::StartPage( hdc );

}



/* 印刷 ページ終了 */
void CPrint::PrintEndPage( HDC hdc )
{
	::EndPage( hdc );

}


/* 印刷 ジョブ終了 */
void CPrint::PrintClose( HANDLE hPrinter, HDC hdc )
{
	::EndDoc( hdc );
	::DeleteDC( hdc );
	::ClosePrinter( hPrinter );
}

/*[EOF]*/




/* 用紙の名前を取得 */
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
		"Letter、8 1/2x11 inch",
		"Legal、8 1/2x14 inch",
		"C sheet、17x22 inch",
		"D sheet、22x34 inch",
		"E sheet、34x44 inch",
		"Letter Small、8 1/2x11 inch",
		"Tabloid、11x17 inch",
		"Ledger、17x11 inch",
		"Statement、5 1/2x8 1/2 inch",
		"Executive、7 1/4x10 1/2 inch",
		"Folio、8 1/2x13 inch",
		"10x14 inch sheet",
		"11x17 inch sheet",
		"Note、 8 1/2x11 inch",
		"#9 Envelope、3 7/8x8 7/8 inch",
		"#10 Envelope、4 1/8x9 1/2 inch",
		"#11 Envelope、4 1/2x10 3/8 inch",
		"#12 Envelope、4 3/4x11 inch",
		"#14 Envelope、5x11 1/2 inch",
		"Monarch Envelope、3 7/8x7 1/2 inch",
		"6 3/4 Envelope、3 5/8x6 1/2 inch",
		"US Std Fanfold、14 7/8x11 inch",
		"German Std Fanfold、8 1/2x12 inch",
		"German Legal Fanfold、8 1/2x13 inch",
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
		strcpy( pszPaperName, "不明" );
	}
	return pszPaperName;



}

void CPrint::Initialize()
{
	/* 時間がかからないものだけ初期化 */
	int i = 0;
	DLLSHAREDATA*	m_pShareData = CShareData::getInstance()->GetShareData();
	wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "印刷設定 %d", i + 1 );	/* 印刷設定の名前 */
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan, "ＭＳ 明朝" );				/* 印刷フォント */
	strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen, "ＭＳ 明朝" );				/* 印刷フォント */
	m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth = 12;  								/* 印刷フォント幅(1/10mm単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight = m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth * 2;	/* 印刷フォント高さ(1/10mm単位単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu = 1;			/* 段組の段数 */
	m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace = 70; 		/* 段と段の隙間(1/10mm) */
	m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap = TRUE;		/* 英文ワードラップする */
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuHead = FALSE;		/* 行頭禁則する */	//@@@ 2002.04.09 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuTail = FALSE;		/* 行末禁則する */	//@@@ 2002.04.09 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuRet = FALSE;		/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber = FALSE;	/* 行番号を印刷する */
	m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing = 30;	/* 印刷フォント行間 文字の高さに対する割合(%) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY = 100;		/* 印刷用紙マージン 上(1/10mm単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY = 200;		/* 印刷用紙マージン 下(1/10mm単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX = 200;		/* 印刷用紙マージン 左(1/10mm単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX = 100;		/* 印刷用紙マージン 右(1/10mm単位) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2) */
	m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize = DMPAPER_A4;	/* 用紙サイズ */
	/* プリンタ設定 DEVMODE用 */
	/* プリンタ設定を取得するのはコストがかかるので、後ほど */
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
		wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "印刷設定 %d", i + 1 );	/* 印刷設定の名前 */
	}
}

/*[EOF]*/
