/*!	@file
	@brief 印刷

	@author Norio Nakatani
	
	@date 2006.08.14 Moca 用紙名一覧の重複削除・情報の統合
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK
	Copyright (C) 2003, かろと

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
#include <stdlib.h>
#include <WinSpool.h>
#include "CPrint.h"
#include "_main/global.h"


// 2006.08.14 Moca 用紙名一覧の重複削除・情報の統合
const PAPER_INFO CPrint::m_paperInfoArr[] = {
	// 	用紙ID, 幅
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
	// メモリ割り当て済みならば、解放する
	// 2003.05.18 かろと
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



/*! @brief プリンタダイアログを表示して、プリンタを選択する
** 
** @param pPD			[i/o]	プリンタダイアログ構造体
** @param pMYDEVMODE 	[i/o] 	印刷設定

	@author かろと
	@date 2003.
*/
BOOL CPrint::PrintDlg( PRINTDLG *pPD, MYDEVMODE *pMYDEVMODE )
{
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* プリンタ設定 DEVNAMES用*/

	// デフォルトプリンタが選択されていなければ、選択する
	if ( m_hDevMode == NULL ) {
		if ( !GetDefaultPrinter( pMYDEVMODE ) ) {
			return FALSE;
		}
	}

	//
	//  現在のプリンタ設定の必要部分を変更
	//
	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;
	// PrintDlg()でReAllocされる事を考えて、呼び出す前にUnlock
	::GlobalUnlock( m_hDevMode );

	/* プリンタダイアログを表示して、プリンタを選択 */
	pPD->lStructSize = sizeof(*pPD);
	pPD->hDevMode = m_hDevMode;
	pPD->hDevNames = m_hDevNames;
	if( !::PrintDlg( pPD ) ){
		// プリンタを変更しなかった
		return FALSE;
	}

	m_hDevMode = pPD->hDevMode;
	m_hDevNames = pPD->hDevNames;

	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( m_hDevNames );

	// プリンタドライバ名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDriverName,
		_countof(pMYDEVMODE->m_szPrinterDriverName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// プリンタデバイス名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDeviceName,
		_countof(pMYDEVMODE->m_szPrinterDeviceName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// プリンタポート名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterOutputName,
		_countof(pMYDEVMODE->m_szPrinterOutputName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// プリンタから得られた、dmFieldsは変更しない
	// プリンタがサポートしないbitをセットすると、プリンタドライバによっては、不安定な動きをする場合がある
	// pMYDEVMODEは、コピーしたいbitで１のものだけセットする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE( _T(" (入力/出力) デバイス ドライバ=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	DEBUG_TRACE( _T(" (入力/出力) デバイス名=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	DEBUG_TRACE( _T("物理出力メディア (出力ポート) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	DEBUG_TRACE( _T("デフォルトのプリンタか=[%d]\n"),          pDEVNAMES->wDefault );

	::GlobalUnlock( m_hDevMode );
	::GlobalUnlock( m_hDevNames );
	return TRUE;
}


/*! @brief デフォルトのプリンタを取得し、MYDEVMODE に設定 
** 
** @param pMYDEVMODE 	[out] 	印刷設定
*/
BOOL CPrint::GetDefaultPrinter( MYDEVMODE* pMYDEVMODE )
{
	PRINTDLG	pd;
	DEVMODE*	pDEVMODE;
	DEVNAMES*	pDEVNAMES;		/* プリンタ設定 DEVNAMES用*/

	// 2009.08.08 印刷で用紙サイズ、横指定が効かない問題対応 syat
	//// すでに DEVMODEを取得済みなら、何もしない
	//if (m_hDevMode != NULL) {
	//	return TRUE;
	//}

	// DEVMODEを取得済みでない場合、取得する
	if( m_hDevMode == NULL ){
		//
		// PRINTDLG構造体を初期化する（ダイアログは表示しないように）
		// PrintDlg()でデフォルトプリンタのデバイス名などを取得する
		//
		memset_raw ( &pd, 0, sizeof(pd) );
		pd.lStructSize	= sizeof(pd);
		pd.Flags		= PD_RETURNDEFAULT;
		if( !::PrintDlg( &pd ) ){
			pMYDEVMODE->m_bPrinterNotFound = TRUE;	/* プリンタがなかったフラグ */
			return FALSE;
		}
		pMYDEVMODE->m_bPrinterNotFound = FALSE;	/* プリンタがなかったフラグ */

		/* 初期化 */
		memset_raw( pMYDEVMODE, 0, sizeof(*pMYDEVMODE) );
		m_hDevMode = pd.hDevMode;
		m_hDevNames = pd.hDevNames;
	}

	// MYDEVMODEへのコピー
	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVNAMES = (DEVNAMES*)::GlobalLock( m_hDevNames );

	// プリンタドライバ名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDriverName,
		_countof(pMYDEVMODE->m_szPrinterDriverName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// プリンタデバイス名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterDeviceName,
		_countof(pMYDEVMODE->m_szPrinterDeviceName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// プリンタポート名
	_tcscpy_s(
		pMYDEVMODE->m_szPrinterOutputName,
		_countof(pMYDEVMODE->m_szPrinterOutputName),
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// プリンタから得られた、dmFieldsは変更しない
	// プリンタがサポートしないbitをセットすると、プリンタドライバによっては、不安定な動きをする場合がある
	// pMYDEVMODEは、コピーしたいbitで１のものだけコピーする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation		= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize			= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength		= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth		= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE( _T(" (入力/出力) デバイス ドライバ=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset );
	DEBUG_TRACE( _T(" (入力/出力) デバイス名=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset );
	DEBUG_TRACE( _T("物理出力メディア (出力ポート) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset );
	DEBUG_TRACE( _T("デフォルトのプリンタか=[%d]\n"),          pDEVNAMES->wDefault );

	::GlobalUnlock( m_hDevMode );
	::GlobalUnlock( m_hDevNames );
	return TRUE;
}

/*! 
** @brief プリンタをオープンし、hDCを作成する
*/
HDC CPrint::CreateDC(
	MYDEVMODE*	pMYDEVMODE,
	TCHAR*		pszErrMsg		/* エラーメッセージ格納場所 */
)
{
	HDC			hdc = NULL;
	HANDLE		hPrinter = NULL;
	DEVMODE*	pDEVMODE;

	// プリンタが選択されていなければ、NULLを返す
	if ( m_hDevMode == NULL ) {
		return NULL;
	}

	//
	// OpenPrinter()で、デバイス名でプリンタハンドルを取得
	//
	if( !::OpenPrinter(
		pMYDEVMODE->m_szPrinterDeviceName,		/* プリンタデバイス名 */
		&hPrinter,					/* プリンタハンドルのポインタ */
		NULL
	) ){
		auto_sprintf(
			pszErrMsg,
			LS(STR_ERR_CPRINT01),
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		goto end_of_func;
	}

	pDEVMODE = (DEVMODE*)::GlobalLock( m_hDevMode );
	pDEVMODE->dmOrientation			= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize			= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength			= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth			= pMYDEVMODE->dmPaperWidth;

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
		pMYDEVMODE->m_szPrinterDriverName,	/* プリンタドライバ名 */
		pMYDEVMODE->m_szPrinterDeviceName,	/* プリンタデバイス名 */
		pMYDEVMODE->m_szPrinterOutputName,	/* プリンタポート名 */
		pDEVMODE
	);

	// pMYDEVMODEは、コピーしたいbitで１のものだけコピーする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
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


/* 印刷/プレビューに必要な情報を取得 */
BOOL CPrint::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
	short*		pnPaperAllWidth,	/* 用紙幅 */
	short*		pnPaperAllHeight,	/* 用紙高さ */
	short*		pnPaperWidth,		/* 用紙印刷可能幅 */
	short*		pnPaperHeight,		/* 用紙印刷可能高さ */
	short*		pnPaperOffsetLeft,	/* 用紙余白左端 */
	short*		pnPaperOffsetTop,	/* 用紙余白上端 */
	TCHAR*		pszErrMsg			/* エラーメッセージ格納場所 */
)
{
	BOOL		bRet;
	HDC			hdc;
	bRet = TRUE;

	/* 現在の設定で、用紙の幅、高さを確定し、CreateDCに渡す */
	if( !GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	// pMYDEVMODEを使って、hdcを取得
	if ( NULL == (hdc = CreateDC( pMYDEVMODE, pszErrMsg )) ){
		return FALSE;
	}

	/* CreateDC実行によって得られた実際のプリンタの用紙の幅、高さを取得 */
	if( !GetPaperSize( pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE ) ){
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	/* マッピング モードの設定 */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC	それぞれの論理単位は 0.1 mm にマップされます。

	/* 最小左マージンと最小上マージンを取得(1mm単位) */
	POINT	po;
	if( 0 < ::Escape( hdc, GETPRINTINGOFFSET, (int)NULL, NULL, (LPPOINT)&po ) ){
		::DPtoLP( hdc, &po, 1 );
		*pnPaperOffsetLeft = (short)abs( po.x );	/* 用紙余白左端 */
		*pnPaperOffsetTop  = (short)abs( po.y );	/* 用紙余白上端 */
	}else{
		*pnPaperOffsetLeft = 0;	/* 用紙余白左端 */
		*pnPaperOffsetTop  = 0;	/* 用紙余白上端 */
	}

	/* 用紙の印刷可能な幅、高さ */
	po.x = ::GetDeviceCaps( hdc, HORZRES );	/* 用紙印刷可能幅←物理ディスプレイの幅 (mm 単位) */
	po.y = ::GetDeviceCaps( hdc, VERTRES );	/* 用紙印刷可能高さ←物理ディスプレイの高さ (mm 単位)  */
	::DPtoLP( hdc, &po, 1 );
	*pnPaperWidth  = (short)abs( po.x );
	*pnPaperHeight = (short)abs( po.y );

	::DeleteDC( hdc );

	return bRet;
}



/* 用紙の幅、高さ */
BOOL CPrint::GetPaperSize(
	short*		pnPaperAllWidth,
	short*		pnPaperAllHeight,
	MYDEVMODE*	pDEVMODE
)
{
	short	nWork;


	if( pDEVMODE->dmFields &  DM_PAPERSIZE ){
		// 2006.08.14 Moca swich/caseテーブルを廃止して 用紙情報を統合
		const PAPER_INFO* pi = FindPaperInfo( pDEVMODE->dmPaperSize );
		if( NULL != pi ){
			*pnPaperAllWidth = pi->m_nAllWidth;
			*pnPaperAllHeight = pi->m_nAllHeight;
		}else{
			// 2001.12.21 hor マウスでクリックしたままリスト外に出るとここにくるけど、
			//	異常ではないので FALSE を返すことにする
			return FALSE;
		}
	}
	if(pDEVMODE->dmFields & DM_PAPERLENGTH && 0 != pDEVMODE->dmPaperLength ){
		/* pDEVMODE->dmPaperLengthは1/10mm単位である */
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	} else {
		pDEVMODE->dmPaperLength = *pnPaperAllHeight;
		pDEVMODE->dmFields |= DM_PAPERLENGTH;
	}
	if(pDEVMODE->dmFields & DM_PAPERWIDTH && 0 != pDEVMODE->dmPaperWidth ){
		/* pDEVMODE->dmPaperWidthは1/10mm単位である */
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
	} else {
		pDEVMODE->dmPaperWidth = *pnPaperAllWidth;
		pDEVMODE->dmFields |= DM_PAPERWIDTH;
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
	TCHAR*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HDC*		phdc,
	TCHAR*		pszErrMsg		/* エラーメッセージ格納場所 */
)
{
	BOOL		bRet;
	HDC			hdc;
	DOCINFO		di;
	bRet = TRUE;
	// 
	// hdcを取得
	//
	if ( NULL == (hdc = CreateDC( pMYDEVMODE, pszErrMsg )) ){
		bRet = FALSE;
		goto end_of_func;
	}

	/* マッピング モードの設定 */
	::SetMapMode( hdc, MM_LOMETRIC );	//MM_LOMETRIC		それぞれの論理単位は、0.1 mm にマップされます。

	//
	//  印刷ジョブ開始
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
			LS(STR_ERR_CPRINT02),
			pMYDEVMODE->m_szPrinterDeviceName	/* プリンタデバイス名 */
		);
		bRet = FALSE;
		goto end_of_func;
	}

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
void CPrint::PrintClose( HDC hdc )
{
	::EndDoc( hdc );
	::DeleteDC( hdc );
}





/* 用紙の名前を取得 */
TCHAR* CPrint::GetPaperName( int nPaperSize, TCHAR* pszPaperName )
{
	// 2006.08.14 Moca 用紙情報の統合
	const PAPER_INFO* paperInfo = FindPaperInfo( nPaperSize );
	if( NULL != paperInfo ){
		_tcscpy( pszPaperName, paperInfo->m_pszName );
	}else{
		_tcscpy( pszPaperName, LS(STR_ERR_CPRINT03) );
	}
	return pszPaperName;
}

/*!
	用紙情報の取得
	@date 2006.08.14 Moca 新規作成 用紙情報の統合
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


/*!	@brief PRINTSETTINGの初期化

	ここではm_mdmDevModeの プリンタ設定は取得・初期化しない

	@date 2006.08.14 Moca  Initializeから名称変更。初期化単位をShareDate全てからPRINTSETTING単位に変更．
		本関数からDLLSHAREDATAへアクセスする代わりに，CShareDataからPPRINTSETTING単位で逐一渡してもらう．
*/
void CPrint::SettingInitialize( PRINTSETTING& pPrintSetting, const TCHAR* settingName )
{
	_tcscpy( pPrintSetting.m_szPrintSettingName, settingName );			/* 印刷設定の名前 */
	_tcscpy( pPrintSetting.m_szPrintFontFaceHan, _T("ＭＳ 明朝") );		/* 印刷フォント */
	_tcscpy( pPrintSetting.m_szPrintFontFaceZen, _T("ＭＳ 明朝") );		/* 印刷フォント */
	pPrintSetting.m_bColorPrint = false;		// カラー印刷			// 2013/4/26 Uchi
	pPrintSetting.m_nPrintFontWidth = 12;		// 印刷フォント幅(1/10mm単位)
	pPrintSetting.m_nPrintFontHeight = pPrintSetting.m_nPrintFontWidth * 2;	/* 印刷フォント高さ(1/10mm単位単位) */
	pPrintSetting.m_nPrintDansuu = 1;			/* 段組の段数 */
	pPrintSetting.m_nPrintDanSpace = 70; 		/* 段と段の隙間(1/10mm) */
	pPrintSetting.m_bPrintWordWrap = true;		// 英文ワードラップする
	pPrintSetting.m_bPrintKinsokuHead = false;	// 行頭禁則する			//@@@ 2002.04.09 MIK
	pPrintSetting.m_bPrintKinsokuTail = false;	// 行末禁則する			//@@@ 2002.04.09 MIK
	pPrintSetting.m_bPrintKinsokuRet  = false;	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	pPrintSetting.m_bPrintKinsokuKuto = false;	// 2006.08.14 Moca 初期化ミス
	pPrintSetting.m_bPrintLineNumber = false;	/* 行番号を印刷する */
	pPrintSetting.m_nPrintLineSpacing = 30;	/* 印刷フォント行間 文字の高さに対する割合(%) */
	pPrintSetting.m_nPrintMarginTY = 100;		/* 印刷用紙マージン 上(1/10mm単位) */
	pPrintSetting.m_nPrintMarginBY = 200;		/* 印刷用紙マージン 下(1/10mm単位) */
	pPrintSetting.m_nPrintMarginLX = 200;		/* 印刷用紙マージン 左(1/10mm単位) */
	pPrintSetting.m_nPrintMarginRX = 100;		/* 印刷用紙マージン 右(1/10mm単位) */
	pPrintSetting.m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2) */
	pPrintSetting.m_nPrintPaperSize = DMPAPER_A4;	/* 用紙サイズ */
	/* プリンタ設定 DEVMODE用 */
	/* プリンタ設定を取得するのはコストがかかるので、後ほど */
	//	m_cPrint.GetDefaultPrinterInfo( &(pPrintSetting.m_mdmDevMode) );
	pPrintSetting.m_bHeaderUse[0] = TRUE;
	pPrintSetting.m_bHeaderUse[1] = FALSE;
	pPrintSetting.m_bHeaderUse[2] = FALSE;
	wcscpy( pPrintSetting.m_szHeaderForm[0], L"$f" );
	pPrintSetting.m_szHeaderForm[1][0] = L'\0';
	pPrintSetting.m_szHeaderForm[2][0] = L'\0';
	pPrintSetting.m_bFooterUse[0] = TRUE;
	pPrintSetting.m_bFooterUse[1] = FALSE;
	pPrintSetting.m_bFooterUse[2] = FALSE;
	pPrintSetting.m_szFooterForm[0][0] = L'\0';
	wcscpy( pPrintSetting.m_szFooterForm[1], L"- $p -" );
	pPrintSetting.m_szFooterForm[2][0] = L'\0';
}


/*!
	印字可能桁数の計算
	@date 2013.05.10 aroka 新規作成
*/
int CPrint::CalculatePrintableColumns( PRINTSETTING *pPS, int nPaperAllWidth, int nLineNumberColumns )
{
	int nPrintablePaperWidth = nPaperAllWidth - pPS->m_nPrintMarginLX - pPS->m_nPrintMarginRX;
	if( nPrintablePaperWidth < 0 ){ return 0; }

	int nPrintSpaceWidth = ( pPS->m_nPrintDansuu - 1 ) * pPS->m_nPrintDanSpace
						 + ( pPS->m_nPrintDansuu ) * ( nLineNumberColumns * pPS->m_nPrintFontWidth );
	if( nPrintablePaperWidth < nPrintSpaceWidth ){ return 0; }

	int nEnableColumns =
		( nPrintablePaperWidth - nPrintSpaceWidth
		) / pPS->m_nPrintFontWidth / pPS->m_nPrintDansuu;	/* 印字可能桁数/ページ */
	return nEnableColumns;
}


/*!
	印字可能行数の計算
	@date 2013.05.10 aroka 新規作成
*/
int CPrint::CalculatePrintableLines( PRINTSETTING *pPS, int nPaperAllHeight )
{
	int nPrintablePaperHeight = nPaperAllHeight - pPS->m_nPrintMarginTY - pPS->m_nPrintMarginBY;
	if( nPrintablePaperHeight < 0 ){ return 0; }

	int nPrintSpaceHeight = ( pPS->m_nPrintFontHeight * pPS->m_nPrintLineSpacing / 100 );

	int nEnableLines =
		( nPrintablePaperHeight - CalcHeaderHeight( pPS )*2 - CalcFooterHeight( pPS )*2 + nPrintSpaceHeight ) /
		( pPS->m_nPrintFontHeight + nPrintSpaceHeight );	// 印字可能行数/ページ
	if( nEnableLines < 0 ){ return 0; }
	return nEnableLines;
}


/*!
	ヘッダ高さの計算(行送り分こみ)
	@date 2013.05.16 Uchi 新規作成
*/
int CPrint::CalcHeaderHeight( PRINTSETTING* pPS )
{
	if (pPS->m_szHeaderForm[0][0] == _T('\0')
	 && pPS->m_szHeaderForm[1][0] == _T('\0')
	 && pPS->m_szHeaderForm[2][0] == _T('\0')) {
		// 使ってなければ 0
		return 0;
	}

	int		nHeight;
	if (pPS->m_lfHeader.lfFaceName[0] == _T('\0')) {
		// フォント指定無し
		nHeight = pPS->m_nPrintFontHeight;
	}
	else {
		// フォントのサイズ計算(pt->1/10mm)
		nHeight = pPS->m_nHeaderPointSize * 254 / 720;
	}
	return nHeight * (pPS->m_nPrintLineSpacing + 100) / 100;	// 行送り計算
}

/*!
	フッタ高さの計算(行送り分こみ)
	@date 2013.05.16 Uchi 新規作成
*/
int CPrint::CalcFooterHeight( PRINTSETTING* pPS )
{
	if (pPS->m_szFooterForm[0][0] == _T('\0')
	 && pPS->m_szFooterForm[1][0] == _T('\0')
	 && pPS->m_szFooterForm[2][0] == _T('\0')) {
		// 使ってなければ 0
		 return 0;
	}

	int		nHeight;
	if (pPS->m_lfFooter.lfFaceName[0] == _T('\0')) {
		// フォント指定無し
		nHeight = pPS->m_nPrintFontHeight;
	}
	else {
		// フォントのサイズ計算(pt->1/10mm)
		nHeight = pPS->m_nFooterPointSize * 254 / 720;
	}
	return nHeight * (pPS->m_nPrintLineSpacing + 100) / 100;	// 行送り計算
}
