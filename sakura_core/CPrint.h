//	$Id$
/*!	@file
	@brief ����֘A

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CPrint;

#ifndef _CPRINT_H_
#define _CPRINT_H_

#include <windows.h>

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
/*!
	@brief ����֘A�@�\

	�I�u�W�F�N�g�w���łȂ��N���X
*/
class CPrint
{
public:
	/*
	||	static�֐��Q
	*/
	static void Initialize();

public:
	/*
	||  Constructors
	*/
	CPrint();
	~CPrint();

	/*
	||  Attributes & Operations
	*/
	BOOL GetDefaultPrinter( PRINTDLG* );		/* �f�t�H���g�̃v�����^�����擾 */
	BOOL GetDefaultPrinterInfo( MYDEVMODE* );	/* �f�t�H���g�̃v�����^�ݒ� MYDEVMODE ���擾 */
	BOOL GetPrinter( PRINTDLG* );				/* �v�����^�����擾 */
	BOOL GetPrinterInfo( MYDEVMODE* );			/* �v�����^�ݒ� MYDEVMODE ���擾 */
	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	BOOL GetPrintMetrics(
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
	BOOL PrintOpen(
		char*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HANDLE*		phPrinter,
		HDC*		phdc,
		char*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
	);
	void PrintStartPage( HDC );	/* ��� �y�[�W�J�n */
	void PrintEndPage( HDC );	/* ��� �y�[�W�I�� */
	void PrintClose( HANDLE , HDC );	/* ��� �W���u�I�� */
	char* CPrint::GetPaperName( int , char* );	/* �p���̖��O���擾 */

protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPRINT_H_ */


/*[EOF]*/
