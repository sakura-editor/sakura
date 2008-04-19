/*!	@file
	@brief ����֘A

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CPrint;

#ifndef _CPRINT_H_
#define _CPRINT_H_

#include <winspool.h>
#include <commdlg.h> // PRINTDLG

struct	MYDEVMODE {
	BOOL	m_bPrinterNotFound;	/* �v�����^���Ȃ������t���O */
	TCHAR	m_szPrinterDriverName[_MAX_PATH + 1];	// �v�����^�h���C�o��
	TCHAR	m_szPrinterDeviceName[_MAX_PATH + 1];	// �v�����^�f�o�C�X��
	TCHAR	m_szPrinterOutputName[_MAX_PATH + 1];	// �v�����^�|�[�g��
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

// 2006.08.14 Moca �p�����̓��� PAPER_INFO�V��
//! �p�����
struct PAPER_INFO {
	int				m_nId;			//!< �p��ID
	int				m_nAllWidth;	//!< �� (0.1mm�P��)
	int				m_nAllHeight;	//!< ���� (0.1mm�P��)
	const TCHAR*	m_pszName;		//!< �p������
};

struct PRINTSETTING;


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
	static const PAPER_INFO m_paperInfoArr[];	//!< �p�����ꗗ
	static const int m_nPaperInfoArrNum; //!< �p�����ꗗ�̗v�f��


	/*
	||	static�֐��Q
	*/
	static void SettingInitialize( PRINTSETTING&, const TCHAR* settingName );

	static TCHAR* GetPaperName( int , TCHAR* );	/* �p���̖��O���擾 */
	/* �p���̕��A���� */
	static BOOL GetPaperSize(
		int*		pnPaperAllWidth,
		int*		pnPaperAllHeight,
		MYDEVMODE*	pDEVMODE
	);

public:
	/*
	||  Constructors
	*/
	CPrint();
	~CPrint();

	/*
	||  Attributes & Operations
	*/
	BOOL GetDefaultPrinter( MYDEVMODE *pMYDEVMODE );		/* �f�t�H���g�̃v�����^�����擾 */
	BOOL PrintDlg( PRINTDLG *pd, MYDEVMODE *pMYDEVMODE );				/* �v�����^�����擾 */
	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
		int*		pnPaperAllWidth,	/* �p���� */
		int*		pnPaperAllHeight,	/* �p������ */
		int*		pnPaperWidth,		/* �p������\�� */
		int*		pnPaperHeight,		/* �p������\���� */
		int*		pnPaperOffsetLeft,	/* �p���]�����[ */
		int*		pnPaperOffsetTop,	/* �p���]����[ */
		TCHAR*		pszErrMsg			/* �G���[���b�Z�[�W�i�[�ꏊ */
	);


	/* ��� �W���u�J�n */
	BOOL PrintOpen(
		TCHAR*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HDC*		phdc,
		TCHAR*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
	);
	void PrintStartPage( HDC );	/* ��� �y�[�W�J�n */
	void PrintEndPage( HDC );	/* ��� �y�[�W�I�� */
	void PrintClose( HDC );		/* ��� �W���u�I�� */ // 2003.05.02 ����� �s�v��hPrinter�폜

protected:
	/*
	||  �����w���p�֐�
	*/
	// DC�쐬����(�������܂Ƃ߂�) 2003.05.02 �����
	HDC CreateDC( MYDEVMODE *pMYDEVMODE, TCHAR *pszErrMsg);
	
	static const PAPER_INFO* FindPaperInfo( int id );
private:
	/*
	||  �����o�ϐ�
	*/
	HGLOBAL	m_hDevMode;							//!< ���݃v�����^��DEVMODE�ւ̃������n���h��
	HGLOBAL	m_hDevNames;						//!< ���݃v�����^��DEVNAMES�ւ̃������n���h��
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPRINT_H_ */



