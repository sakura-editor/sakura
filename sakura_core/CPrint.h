//	$Id$
/*!	@file
	����֘A

	�I�u�W�F�N�g�w���łȂ��N���X

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CPrint;

#ifndef _CPRINT_H_
#define _CPRINT_H_

#include <windows.h>
#include "debug.h"

struct	MYDEVMODE {
	BOOL	m_bPrinterNotFound;	/* �v�����^���Ȃ������t���O */
	char	m_szPrinterDriverName[_MAX_PATH + 1];	/* �v�����^�h���C�o�� */
	char	m_szPrinterDeviceName[_MAX_PATH + 1];				/* �v�����^�f�o�C�X�� */
	char	m_szPrinterOutputName[_MAX_PATH + 1];	/* �v�����^�|�[�g�� */
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
�N���X�̐錾
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
	static BOOL GetDefaultPrinter( PRINTDLG* );	/* �f�t�H���g�̃v�����^�����擾 */
	static BOOL GetDefaultPrinterInfo( MYDEVMODE* );	/* �f�t�H���g�̃v�����^�ݒ� MYDEVMODE ���擾 */
	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	static BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
//		LOGFONT*	pLOGFONT,
//		int			nMarginTY,			/* �}�[�W�� �� */
//		int			nMarginBY,			/* �}�[�W�� �� */
//		int			nMarginLX,			/* �}�[�W�� �� */
//		int			nMarginRX,			/* �}�[�W�� �E */
//		int			nLineSpacing,		/* �s�� �����̍����ɑ΂��銄�� */
		int*		pnPaperAllWidth,	/* �p���� */
		int*		pnPaperAllHeight,	/* �p������ */
		int*		pnPaperWidth,		/* �p������\�� */
		int*		pnPaperHeight,		/* �p������\���� */
		int*		pnPaperOffsetLeft,	/* �p���]�����[ */
		int*		pnPaperOffsetTop,	/* �p���]����[ */
//		int*		pnCharWidth,		/* ������ */
//		int*		pnCharHeight,		/* �������� */
//		int*		pnAllChars,			/* �������Ɉ󎚉\�Ȍ��� */
//		int*		pnAllLines,			/* �c�����Ɉ󎚉\�ȍs�� */
		char*		pszErrMsg			/* �G���[���b�Z�[�W�i�[�ꏊ */
	);
	/* �p���̕��A���� */
	static BOOL CPrint::GetPaperSize(
		int*		pnPaperAllWidth,
		int*		pnPaperAllHeight,
		DEVMODE*	pDEVMODE
	);


	/* ��� �W���u�J�n */
	static BOOL PrintOpen(
		char*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HANDLE*		phPrinter,
		HDC*		phdc,
		char*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
	);
	static void PrintStartPage( HDC );	/* ��� �y�[�W�J�n */
	static void PrintEndPage( HDC );	/* ��� �y�[�W�I�� */
	static void PrintClose( HANDLE , HDC );	/* ��� �W���u�I�� */
	static char* CPrint::GetPaperName( int , char* );	/* �p���̖��O���擾 */


protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPRINT_H_ */


/*[EOF]*/
