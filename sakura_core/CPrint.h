//	$Id$
/************************************************************************

	CPrint.h

印刷関連　オブジェクト指向でないクラス
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CPrint;

#ifndef _CPRINT_H_
#define _CPRINT_H_

#include <windows.h>
#include "debug.h"

struct	MYDEVMODE {
    BOOL	m_bPrinterNotFound;	/* プリンタがなかったフラグ */
	char	m_szPrinterDriverName[_MAX_PATH + 1];	/* プリンタドライバ名 */
	char	m_szPrinterDeviceName[_MAX_PATH + 1];				/* プリンタデバイス名 */
	char	m_szPrinterOutputName[_MAX_PATH + 1];	/* プリンタポート名 */
	DWORD	dmFields; 
    short	dmOrientation; 
    short	dmPaperSize; 
    short	dmPaperLength; 
    short	dmPaperWidth; 
    short	dmScale; 
    short	dmCopies; 
    short	dmDefaultSource; 
    short	dmPrintQuality; 
    short	dmColor; 
    short	dmDuplex; 
    short	dmYResolution; 
    short	dmTTOption; 
    short	dmCollate; 
    BCHAR	dmFormName[CCHFORMNAME]; 
    WORD	dmLogPixels; 
    DWORD	dmBitsPerPel; 
    DWORD	dmPelsWidth; 
    DWORD	dmPelsHeight; 
    DWORD	dmDisplayFlags; 
    DWORD	dmDisplayFrequency; 
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CPrint
{
public:
	/*
	||  Constructors
	*/
	CPrint();
	~CPrint();

	/*
	||  Attributes & Operations
	*/
	static BOOL GetDefaultPrinter( PRINTDLG* );	/* デフォルトのプリンタ情報を取得 */	
	static BOOL GetDefaultPrinterInfo( MYDEVMODE* );	/* デフォルトのプリンタ設定 MYDEVMODE　を取得 */	
	/* 印刷/プレビューに必要な情報を取得 */
	static BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
//		LOGFONT*	pLOGFONT,
//		int			nMarginTY,			/* マージン　上 */
//		int			nMarginBY,			/* マージン　下 */
//		int			nMarginLX,			/* マージン　左 */
//		int			nMarginRX,			/* マージン　右 */
//		int			nLineSpacing,		/* 行間　文字の高さに対する割合 */
		int*		pnPaperAllWidth,	/* 用紙幅 */
		int*		pnPaperAllHeight,	/* 用紙高さ */
		int*		pnPaperWidth,		/* 用紙印刷可能幅 */
		int*		pnPaperHeight,		/* 用紙印刷可能高さ */
		int*		pnPaperOffsetLeft,	/* 用紙余白左端 */
		int*		pnPaperOffsetTop,	/* 用紙余白上端 */
//		int*		pnCharWidth,		/* 文字幅 */
//		int*		pnCharHeight,		/* 文字高さ */
//		int*		pnAllChars,			/* 横方向に印字可能な桁数 */	
//		int*		pnAllLines,			/* 縦方向に印字可能な行数 */
		char*		pszErrMsg			/* エラーメッセージ格納場所 */	
	);
	/* 用紙の幅、高さ */
	static BOOL CPrint::GetPaperSize( 
		int*		pnPaperAllWidth, 
		int*		pnPaperAllHeight, 
		DEVMODE*	pDEVMODE
	);
	
	
	/* 印刷 ジョブ開始 */
	static BOOL PrintOpen(
		char*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HANDLE*		phPrinter,
		HDC*		phdc,
		char*		pszErrMsg		/* エラーメッセージ格納場所 */	
	);
	static void PrintStartPage( HDC );	/* 印刷 ページ開始 */
	static void PrintEndPage( HDC );	/* 印刷 ページ終了 */
	static void PrintClose( HANDLE , HDC );	/* 印刷 ジョブ終了 */
	static char* CPrint::GetPaperName( int , char* );	/* 用紙の名前を取得 */
	

protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPRINT_H_ */

/*[EOF]*/

