//	$Id$
/*!	@file
	�����E�B���h�E�̊Ǘ�
	
	@author Norio Nakatani
	@date	1998/03/13 �쐬
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

class CEditView;

#include "CEditDoc.h"


#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_


#define _CARETMARGINRATE 20

#include <windows.h>
#include "CDropTarget.h"
#include "CMemory.h"
#include "CDocLineMgr.h"
#include "CLayoutMgr.h"
#include "COpe.h"
#include "COpeBlk.h"
#include "COpeBuf.h"
#include "CDlgFind.h"
//@@#include "CProp1.h"
#include "CShareData.h"
#include "CFuncInfoArr.h"
#include "CSplitBoxWnd.h"
#include "CSplitterWnd.h"
#include "CDlgCancel.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CDropTarget.h"
#include "CMarkMgr.h"

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif

struct GrepParam {
	void*	pCEditView;
	char*	pszGrepKey;
	char*	pszGrepFile;
	char*	pszGrepFolder;
	BOOL	bGrepSubFolder;
	BOOL	bGrepLoHiCase;
	BOOL	bGrepRegularExp;
	BOOL	bKanjiCode_AutoDetect;
	BOOL	bGrepOutputLine;
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

class SAKURA_CORE_API CEditView/* : public CDropTarget*/
{
public:
	/* Constructors */
	CEditView();
	~CEditView();
	/* �������n�����o�֐� */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	/* ��� */
	BOOL IsTextSelected( void );	/* �e�L�X�g���I������Ă��邩 */
	BOOL IsTextSelecting( void );	/* �e�L�X�g�̑I�𒆂� */
	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	VOID OnTimer( HWND, UINT, UINT, DWORD );
	BOOL HandleCommand( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );
	/* �R�}���h���� */
	void CaretUnderLineON( BOOL );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( BOOL );								/* �J�[�\���s�A���_�[���C����OFF */
	void AdjustScrollBars( void );								/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	int  MoveCursor( int, int, BOOL, int = _CARETMARGINRATE );	/* �s���w��ɂ��J�[�\���ړ� */
	BOOL DetectWidthOfLineNumberArea( BOOL );					/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	int DetectWidthOfLineNumberArea_calculate( void );			/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
	void DisableSelectArea( BOOL );								/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	void OnChangeSetting( void );								/* �ݒ�ύX�𔽉f������ */
	void SetFont( void );										/* �t�H���g�̕ύX */
	void RedrawAll( void );										/* �t�H�[�J�X�ړ����̍ĕ`�� */
	void Redraw( void );										// 2001/06/21 asa-o �ĕ`��
	void CopyViewStatus( CEditView* );							/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
	DWORD DoGrep( CMemory*, CMemory*, CMemory*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int );/* Grep���s */
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int DoGrepTree( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int, CBregexp*, int, int* );
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int DoGrepFile( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int, CBregexp*, int, int*, const char*, CMemory& );
	/* Grep���s */
	void CEditView::DoGrep_Thread(
		DWORD	dwGrepParam
	);

	//	Jun. 16, 2000 genta
	bool  SearchBracket( int PosX, int PosY, int* NewX, int* NewY );	//	�Ί��ʂ̌���
	bool  SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
						int upChar, int dnChar );	//	�Ί��ʂ̑O������
	bool  SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									int dnChar, int upChar );
//@@@ 2001.02.03 Start by MIK: �S�p�̑Ί���
	bool  SearchBracketForward2( int PosX, int PosY, int* NewX, int* NewY,
						         char* upChar, char* dnChar );	//	�Ί��ʂ̑O������
	bool  SearchBracketBackward2( int PosX, int PosY, int* NewX, int* NewY,
								  char* dnChar, char* upChar );
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��


public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	CDropTarget*	m_pcDropTarget;
	BOOL			m_bDrawSWITCH;
	BOOL			m_bDragSource;	/* �I���e�L�X�g�̃h���b�O���� */
	BOOL			m_bDragMode;	/* �I���e�L�X�g�̃h���b�O���� */

	/* �P�ꌟ���̏�� */
	
	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	m_CurRegexp;			/*!< �R���p�C���f�[�^ */
	BOOL	m_bCurSrchKeyMark;			/* ����������̃}�[�N */
	char	m_szCurSrchKey[_MAX_PATH];	/* ���������� */
	int		m_bCurSrchRegularExp;		/* �����^�u��  1==���K�\�� */
	int		m_bCurSrchLoHiCase;			/* �����^�u��  1==�p�啶���������̋�� */
	int		m_bCurSrchWordOnly;			/* �����^�u��  1==�P��̂݌��� */

	BOOL	m_bExecutingKeyMacro;		/* �L�[�{�[�h�}�N���̎��s�� */
//	BOOL	m_bGrepRunning;
	HANDLE	m_hThreadGrep;
	HWND	m_hWnd;				/* �ҏW�E�B���h�E�n���h�� */
	int		m_nViewTopLine;		/* �\����̈�ԏ�̍s(0�J�n) */
	int		m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */
	int		m_nCaretPosX_Prev;	/* �r���[���[����̃J�[�\�����ʒu�i�O�I���W���j*/
	int		m_nCaretPosX;		/* �r���[���[����̃J�[�\�����ʒu�i�O�J�n�j*/
	int		m_nCaretPosY;		/* �r���[��[����̃J�[�\���s�ʒu�i�O�J�n�j*/
	int		m_nCaretPosX_PHY;	/* �J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	int		m_nCaretPosY_PHY;	/* �J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	/*
	||  �����o�ϐ�
	*/
	char*			m_pszAppName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HINSTANCE		m_hInstance;	/* �C���X�^���X�n���h�� */
	HWND			m_hwndParent;	/* �e�E�B���h�E�n���h�� */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	CEditDoc*		m_pcEditDoc;		/* �h�L�������g */
	COpeBlk*		m_pcOpeBlk;			/* ����u���b�N */
	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	int				m_nVScrollRate;		/* �����X�N���[���o�[�̏k�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* ���������{�b�N�X */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* ���������{�b�N�X */
	HFONT			m_hFontCaretPosInfo;/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	int				m_nCaretPosInfoCharWidth;
	int				m_nCaretPosInfoCharHeight;
	int				m_pnCaretPosInfoDx[64];	/* ������`��p�������z�� */
	HDC				m_hdcCompatDC;		/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */
	int				m_pnDx[10240 + 10];	/* ������`��p�������z�� */
	HFONT			m_hFont_HAN;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT;	/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_UL;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT_UL;	/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_ZEN;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* �R�}���h�̎��s�� */
	/* ������� */
	int				m_nMyIndex;
	/* �L�[���s�[�g��� */
	int				m_bPrevCommand;
	/* ���͏�� */
	int		m_nCaretWidth;			/* �L�����b�g�̕� */
	int		m_nCaretHeight;			/* �L�����b�g�̍��� */
	int		m_nOldUnderLineY;
	/* �L�[��� */
	int		m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	int		m_bBeginSelect;			/* �͈͑I�� */
	int		m_bBeginBoxSelect;		/* ��`�͈͑I�� */
	int		m_bBeginLineSelect;		/* �s�P�ʑI�� */
	int		m_bBeginWordSelect;		/* �P��P�ʑI�� */
//	int		m_nSelectLineBgn;		/* �͈͑I���J�n�s(���_) */
//	int		m_nSelectColmBgn;		/* �͈͑I���J�n��(���_) */
	int		m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
	int		m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
	int		m_nSelectLineBgnTo;		/* �͈͑I���J�n�s(���_) */
	int		m_nSelectColmBgnTo;		/* �͈͑I���J�n��(���_) */
	int		m_nSelectLineFrom;		/* �͈͑I���J�n�s */
	int		m_nSelectColmFrom;		/* �͈͑I���J�n�� */
	int		m_nSelectLineTo;		/* �͈͑I���I���s */
	int		m_nSelectColmTo;		/* �͈͑I���I���� */
	int		m_nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int		m_nSelectColmFromOld;	/* �͈͑I���J�n�� */
	int		m_nSelectLineToOld;		/* �͈͑I���I���s */
	int		m_nSelectColmToOld;		/* �͈͑I���I���� */
	int		m_nMouseRollPosXOld;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
	int		m_nMouseRollPosYOld;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	/* ��ʏ�� */
	int		m_nViewAlignLeft;		/* �\����̍��[���W */
	int		m_nViewAlignLeftCols;	/* �s�ԍ���̌��� */
	int		m_nViewAlignTop;		/* �\����̏�[���W */
	int		m_nTopYohaku;
	int		m_nViewCx;				/* �\����̕� */
	int		m_nViewCy;				/* �\����̍��� */
	int		m_nViewColNum;			/* �\����̌��� */
	int		m_nViewRowNum;			/* �\����̍s�� */
	/* �\�����@ */
	int		m_nCharWidth;			/* ���p�����̕� */
	int		m_nCharHeight;			/* �����̍��� */
	/* ����Tip�֘A */
	DWORD	m_dwTipTimer;			/* Tip�N���^�C�}�[ */
	CTipWnd	m_cTipWnd;				/* Tip�\���E�B���h�E */
	POINT	m_poTipCurPos;			/* Tip�N�����̃}�E�X�J�[�\���ʒu */
	BOOL	m_bInMenuLoop;			/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
	CDicMgr	m_cDicMgr;				/* �����}�l�[�W�� */
	/* ���͕⊮ */
//	CHokanMgr	m_cHokanMgr;
	BOOL		m_bHokan;
	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump����
	/*
	||  �����w���p�֐�
	*/
protected:
	CEOL GetCurrentInsertEOL( void );					/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */

	void GetCurrentTextForSearch( CMemory& );			/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	BOOL MyGetClipboardData( CMemory&, BOOL* );			/* �N���b�v�{�[�h����f�[�^���擾 */
	BOOL MySetClipboardData( const char*, int, BOOL );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	int GetLeftWord( CMemory*, int );					/* �J�[�\�����O�̒P����擾 */
//	void PrintBitmap( HDC, int, int, const char* );		/* �r�b�g�}�b�v�t�@�C���\�� */
//	HANDLE OpenDIB ( LPCSTR );							/* DIB�t�@�C�����J���ă�����DIB���쐬 */
//	HANDLE ReadDibBitmapInfo ( int );					/* DIB�`���̃t�@�C����ǂ� */
//	BOOL DibInfo ( HANDLE, LPBITMAPINFOHEADER );		/* CF_DIB�`���̃������u���b�N�Ɋ֘A�t�����Ă���DIB�����擾���܂� */
//	WORD PaletteSize ( VOID FAR * pv );					/* �p���b�g�̃o�C�g�����v�Z���܂� */
//	WORD DibNumColors ( VOID FAR * );					/* ���u���b�N��BitCount�����o���Q�Ƃ��āADIB�̐F���𔻒f���܂� */
//	DWORD lread ( int, void*, DWORD );					/* �f�[�^�����ׂēǂݎ�� */
//	void TraceRgn( HRGN );								/* �f�o�b�O�p ���[�W������`�̃_���v */
//	void OnPaintOld( HDC, PAINTSTRUCT *, BOOL );		/* �ʏ�̕`�揈�� */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* �ʏ�̕`�揈�� */
//	int DispLine( HDC, int, int, int, const unsigned char*, int, BOOL );/		* �s�̃e�L�X�g�^�I����Ԃ̕`�� */
	int DispLineNew( HDC, const CLayout*, int&, int, int&, BOOL, int, BOOL );	/* �s�̃e�L�X�g�^�I����Ԃ̕`�� */
	void DispLineNumber( HDC, const CLayout*, int, int );		/* �s�ԍ��\�� */
	void SetCurrentColor( HDC, int );							/* ���݂̐F���w�� */
	void DispRuler( HDC );										/* ���[���[�`�� */
//	void DispRulerEx( HDC );									/* ���[���[�`�� */
	int	DispText( HDC, int, int, const unsigned char*, int );	/* �e�L�X�g�\�� */
	void DispTextSelected( HDC, int, int, int, int );			/* �e�L�X�g���] */
	void TwoPointToRect( RECT*, int, int, int, int );			/* 2�_��Ίp�Ƃ����`�����߂� */
	void DrawSelectArea( void );								/* �w��s�̑I��̈�̕`�� */
	void DrawSelectAreaLine( HDC, int, int, int, int, int );	/* �w��s�̑I��̈�̕`�� */
	int  LineColmnToIndex( const char*, int, int );				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
	int  LineColmnToIndex2( const char*, int, int, int* );		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */
	int  LineIndexToColmn( const char*, int, int );				/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
	BOOL GetSelectedData( CMemory&, BOOL, const char*, BOOL, enumEOLType neweol = EOL_UNKNOWN);/* �I��͈͂̃f�[�^���擾 */
	void CopySelectedAllLines( const char*, BOOL );				/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	void ConvSelectedArea( int );								/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	void ConvMemory( CMemory*, int );							/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
	void ShowEditCaret( void );									/* �L�����b�g�̕\���E�X�V */
	void OnSize( int, int );									/* �E�B���h�E�T�C�Y�̕ύX���� */
	void OnVScroll( int, int, HWND );							/* �����X�N���[���o�[���b�Z�[�W���� */
	void OnHScroll( int, int, HWND );							/* �����X�N���[���o�[���b�Z�[�W���� */
	void OnLBUTTONDOWN( WPARAM, int, int );						/* �}�E�X���{�^������ */
	void OnMOUSEMOVE( WPARAM, int, int );						/* �}�E�X�ړ��̃��b�Z�[�W���� */
	void OnLBUTTONUP( WPARAM, int, int );						/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
	void OnLBUTTONDBLCLK( WPARAM, int , int );					/* �}�E�X���{�^���_�u���N���b�N */
	void OnRBUTTONDOWN( WPARAM, int, int );						/* �}�E�X�E�{�^������ */
	void OnRBUTTONUP( WPARAM, int, int );						/* �}�E�X�E�{�^���J�� */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );						/* �}�E�X�z�C�[���̃��b�Z�[�W���� */
	void BeginSelectArea( void );								/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	void ChangeSelectAreaByCurrentCursor( int, int );			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
	void ChangeSelectAreaByCurrentCursorTEST( int, int, int&, int&, int&, int& );/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
	int  MoveCursorToPoint( int, int );							/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ� */
	void ScrollAtV( int );										/* �w���[�s�ʒu�փX�N���[�� */
	void ScrollAtH( int );										/* �w�荶�[���ʒu�փX�N���[�� */
	int Cursor_UPDOWN( int, int );								/* �J�[�\���㉺�ړ����� */
	void SetIMECompFormPos( void );								/* IME�ҏW�G���A�̈ʒu��ύX */
	void SetIMECompFormFont( void );							/* IME�ҏW�G���A�̕\���t�H���g��ύX */
	void SetParentCaption( BOOL = FALSE );						/* �e�E�B���h�E�̃^�C�g�����X�V */
	void DrawCaretPosInfo( void );								/* �L�����b�g�̍s���ʒu��\������ */
//	void Draw3dRect( HDC, int, int, int, int, COLORREF, COLORREF );
//	void FillSolidRect( HDC, int, int, int, int, COLORREF );
	BOOL IsCurrentPositionURL( int, int, int*, int*, int*, char** );/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	int IsCurrentPositionSelected( int, int );					/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	int IsCurrentPositionSelectedTEST( int, int, int, int, int, int );/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	BOOL IsSeaechString( const char*, int, int, int* );			/* ���݈ʒu������������ɊY�����邩 */
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine );		/* �t�H���g��I�� */
	void ExecCmd(const char*, BOOL ) ;							// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	void AddToCmdArr( const char* );


public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* IDropTarget���� */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
protected:
//	LPDATAOBJECT	m_pDataObject;
//	REFIID			m_owniid;

	/* �t�@�C������n */
	void Command_FILENEW( void );				/* �V�K�쐬 */
	void Command_FILEOPEN( void );				/* �t�@�C�����J�� */
	BOOL Command_FILESAVE( void );				/* �㏑���ۑ� */
	BOOL Command_FILESAVEAS( void );			/* ���O��t���ĕۑ� */
	void Command_FILECLOSE( void );				/* �J����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	void Command_FILECLOSE_OPEN( void );		/* ���ĊJ�� */
	void ReOpen_XXX( int );			/* �ăI�[�v�� */
	void Command_FILE_REOPEN_SJIS( void );		/* SJIS�ŊJ������ */
	void Command_FILE_REOPEN_JIS( void );		/* JIS�ŊJ������ */
	void Command_FILE_REOPEN_EUC( void );		/* EUC�ŊJ������ */
	void Command_FILE_REOPEN_UNICODE( void );	/* Unicode�ŊJ������ */
	void Command_FILE_REOPEN_UTF8( void );		/* UTF-8�ŊJ������ */
	void Command_FILE_REOPEN_UTF7( void );		/* UTF-7�ŊJ������ */
	void Command_PRINT( void );					/* ���*/
	void Command_PRINT_PREVIEW( void );			/* ����v���r���[*/
	void Command_PRINT_PAGESETUP( void );		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	BOOL Command_OPEN_HfromtoC( BOOL );			/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
	BOOL Command_OPEN_HHPP( BOOL );				/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	BOOL Command_OPEN_CCPP( BOOL );				/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plus�Ŏ��s */
	void Command_BROWSE( void );				/* �u���E�Y */
	void Command_PROPERTY_FILE( void );			/* �t�@�C���̃v���p�e�B */
	void Command_EXITALL( void );				/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	void Command_CHAR( char );				/* �������� */
	void Command_IME_CHAR( WORD );			/* �S�p�������� */
	void Command_UNDO( void );				/* ���ɖ߂�(Undo) */
	void Command_REDO( void );				/* ��蒼��(Redo) */
	void Command_DELETE( void );			/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	void Command_DELETE_BACK( void );		/* �J�[�\���̑O���폜 */
	void Command_WordDeleteToStart( void );	/* �P��̍��[�܂ō폜 */
	void Command_WordDeleteToEnd( void );	/* �P��̉E�[�܂ō폜 */
	void Command_WordCut( void );			/* �P��؂��� */
	void Command_WordDelete( void );		/* �P��폜 */
	void Command_LineCutToStart( void );   	//�s���܂Ő؂���(���s�P��)
	void Command_LineCutToEnd( void );   	//�s���܂Ő؂���(���s�P��)
	void Command_LineDeleteToStart( void );	/* �s���܂ō폜(���s�P��) */
	void Command_LineDeleteToEnd( void );  	//�s���܂ō폜(���s�P��)
	void Command_CUT_LINE( void );			/* �s�؂���(�܂�Ԃ��P��) */
	void Command_DELETE_LINE( void );		/* �s�폜(�܂�Ԃ��P��) */
	void Command_DUPLICATELINE( void );		/* �s�̓�d��(�܂�Ԃ��P��) */
	void Command_INDENT( char cChar );		/* �C���f���g ver 1 */
	void Command_INDENT( const char*, int );/* �C���f���g ver0 */
	void Command_UNINDENT( char cChar );	/* �t�C���f���g */
	void Command_WORDSREFERENCE( void );	/* �P�ꃊ�t�@�����X */

	void DeleteData2( int, int, int, CMemory*, COpe*, BOOL, BOOL, BOOL = FALSE );/* �w��ʒu�̎w�蒷�f�[�^�폜 */
	void DeleteData( BOOL, BOOL = FALSE );/* ���݈ʒu�̃f�[�^�폜 */
	void InsertData_CEditView( int, int, const char*, int, int*, int*, COpe*, BOOL, BOOL = FALSE );/* ���݈ʒu�Ƀf�[�^��}�� */
	void SmartIndent_CPP( char );	/* C/C++�X�}�[�g�C���f���g���� */

	/* �J�[�\���ړ��n */
	int Command_UP( int, BOOL );			/* �J�[�\����ړ� */
	int Command_DOWN( int, BOOL );			/* �J�[�\�����ړ� */
	int  Command_LEFT( int, BOOL );			/* �J�[�\�����ړ� */
	void Command_RIGHT( int, int, BOOL );	/* �J�[�\���E�ړ� */
	void Command_UP2( int );				/* �J�[�\����ړ��i�Q�s�Âj */
	void Command_DOWN2( int );				/* �J�[�\�����ړ��i�Q�s�Âj */
	void Command_WORDLEFT( int );			/* �P��̍��[�Ɉړ� */
	void Command_WORDRIGHT( int );			/* �P��̉E�[�Ɉړ� */
	void Command_GOLINETOP( int, BOOL );	/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
	void Command_GOLINEEND( int, int );		/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
//	void Command_ROLLDOWN( int );			/* �X�N���[���_�E�� */
//	void Command_ROLLUP( int );				/* �X�N���[���A�b�v */
	void Command_HalfPageUp( int );			//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_HalfPageDown( int );		//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_1PageUp( int );			//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	void Command_1PageDown( int );			//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	void Command_GOFILETOP( int );			/* �t�@�C���̐擪�Ɉړ� */
	void Command_GOFILEEND( int );			/* �t�@�C���̍Ō�Ɉړ� */
	void Command_CURLINECENTER( void );		/* �J�[�\���s���E�B���h�E������ */
	void Command_JUMPPREV(void);			// �ړ�����: �O��
	void Command_JUMPNEXT(void);			// �ړ�����: ����
	void Command_WndScrollDown(void);		// �e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// �e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o

	/* �I���n */
	void Command_SELECTWORD( void );		/* ���݈ʒu�̒P��I�� */
	void Command_SELECTALL( void );			/* ���ׂđI�� */
	void Command_BEGIN_SELECT( void );		/* �͈͑I���J�n */

	/* ��`�I���n */
//	void Command_BOXSELECTALL( void );		/* ��`�ł��ׂđI�� */
	void Command_BEGIN_BOXSELECT( void );	/* ��`�͈͑I���J�n */
	int Command_UP_BOX( BOOL );				/* (��`�I��)�J�[�\����ړ� */
//	int Command_DOWN( int, BOOL );			/* �J�[�\�����ړ� */
//	int  Command_LEFT( int, BOOL );			/* �J�[�\�����ړ� */
//	void Command_RIGHT( int, int, BOOL );	/* �J�[�\���E�ړ� */
//	void Command_UP2( int );				/* �J�[�\����ړ��i�Q�s�Âj */
//	void Command_DOWN2( int );				/* �J�[�\�����ړ��i�Q�s�Âj */
//	void Command_WORDLEFT( int );			/* �P��̍��[�Ɉړ� */
//	void Command_WORDRIGHT( int );			/* �P��̉E�[�Ɉړ� */
//	void Command_GOLINETOP( int, BOOL );	/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
//	void Command_GOLINEEND( int, int );		/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
//	void Command_HalfPageUp( int );			//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	void Command_HalfPageDown( int );		//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	void Command_1PageUp( int );			//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
//	void Command_1PageDown( int );			//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
//	void Command_GOFILETOP( int );			/* �t�@�C���̐擪�Ɉړ� */
//	void Command_GOFILEEND( int );			/* �t�@�C���̍Ō�Ɉړ� */

	/* �N���b�v�{�[�h�n */
	void Command_CUT( void );						/* �؂���i�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜�j*/
	void Command_COPY( int, enumEOLType neweol = EOL_UNKNOWN );/* �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[) */
	void Command_PASTE( void );						/* �\��t���i�N���b�v�{�[�h����\��t���j*/
	void Command_PASTEBOX( void );					/* ��`�\��t���i�N���b�v�{�[�h�����`�\��t���j*/
//	void Command_INSTEXT( BOOL, const char*, int );	/* �e�L�X�g��\��t�� ver0 */
	void Command_INSTEXT( BOOL, const char*, BOOL );/* �e�L�X�g��\��t�� ver1 */
	void Command_ADDTAIL( const char*, int );		/* �Ō�Ƀe�L�X�g��ǉ� */
	void Command_COPYPATH( void );					/* ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
	void Command_COPYTAG( void );					/* ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[ */
	void Command_COPYLINES( void );					/* �I��͈͓��S�s�R�s�[ */
	void Command_COPYLINESASPASSAGE( void );		/* �I��͈͓��S�s���p���t���R�s�[ */
	void Command_COPYLINESWITHLINENUMBER( void );	/* �I��͈͓��S�s�s�ԍ��t���R�s�[ */
	void Command_CREATEKEYBINDLIST( void );			// �L�[���蓖�Ĉꗗ���R�s�[ //Sept. 15, 2000 JEPRO	Command_�̍������킩��Ȃ��̂ŎE���Ă���


	/* �f�[�^�u�� �폜&�}���ɂ��g���� */
void ReplaceData_CEditView(
	int			nDelLineFrom,		/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
	int			nDelColmFrom,		/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
	int			nDelLineTo,			/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
	int			nDelColmTo,			/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
	CMemory*	pcmemCopyOfDeleted,	/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
	const char*	pInsData,			/* �}������f�[�^ */
	int			nInsDataLen,		/* �}������f�[�^�̒��� */
//	int*		pnAddLineNum,		/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
//	int*		pnModLineFrom,		/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	int*		pnModLineTo,		/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	BOOL		bDispSSTRING,		/* �V���O���N�H�[�e�[�V�����������\������ */
//	BOOL		bDispWSTRING,		/* �_�u���N�H�[�e�[�V�����������\������ */
//	BOOL		bUndo				/* Undo���삩�ǂ��� */
//	int			nX,
//	int			nY,
//	const char*	pData,
//	int			nDataLen,
//	int*		pnNewLine,	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int*		pnNewPos,	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//	COpe*		pcOpe,		/* �ҏW����v�f COpe */
	BOOL		bRedraw
//	BOOL		bUndo		/* Undo���삩�ǂ��� */
);

	/* �}���n */
	void Command_INS_DATE( void );	//���t�}��
	void Command_INS_TIME( void );	//�����}��

	/* �ϊ��n */
	void Command_TOLOWER( void );				/* �p�啶�����p������ */
	void Command_TOUPPER( void );				/* �p���������p�啶�� */
	void Command_TOZENKAKUKATA( void );			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	void Command_TOZENKAKUHIRA( void );			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	void Command_TOHANKAKU( void );				/* �S�p�����p */
	void Command_HANKATATOZENKAKUKATA( void );	/* ���p�J�^�J�i���S�p�J�^�J�i */
	void Command_HANKATATOZENKAKUHIRA( void );	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	void Command_TABTOSPACE( void );			/* TAB���� */
	void Command_SPACETOTAB( void );			/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* �������ʁ�SJIS�R�[�h�ϊ� */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS��JIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS��EUC�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS��UTF-8�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS��UTF-7�R�[�h�ϊ� */
	void Command_BASE64DECODE( void );			/* Base64�f�R�[�h���ĕۑ� */
	void Command_UUDECODE( void );				/* uudecode���ĕۑ�	*/	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
	void Command_SEARCH_DIALOG( void );					/* ����(�P�ꌟ���_�C�A���O) */
	void Command_SEARCH_NEXT( BOOL, HWND, const char* );/* �������� */
	void Command_SEARCH_PREV( BOOL, HWND );				/* �O������ */
	void Command_REPLACE( void );						/* �u��(�u���_�C�A���O) */
	void Command_SEARCH_CLEARMARK( void );				/* �����}�[�N�̃N���A */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP( void );							/* �w��s�w�W�����v */
	BOOL Command_FUNCLIST( BOOL );						/* �A�E�g���C����� */
	void /*BOOL*/ Command_TAGJUMP( void/*BOOL*/ );		/* �^�O�W�����v�@�\ */
	void/*BOOL*/ Command_TAGJUMPBACK( void/*BOOL*/ );	/* �^�O�W�����v�o�b�N�@�\ */
	void Command_COMPARE( void );						/* �t�@�C�����e��r */
	void Command_BRACKETPAIR( void );					/* �Ί��ʂ̌��� */

	/* ���[�h�؂�ւ��n */
	void Command_CHGMOD_INS( void );	/* �}���^�㏑�����[�h�؂�ւ� */
	void Command_CANCEL_MODE( void );	/* �e�탂�[�h�̎����� */

	/* �ݒ�n */
	void Command_SHOWTOOLBAR( void );		/* �c�[���o�[�̕\��/��\�� */
	void Command_SHOWFUNCKEY( void );		/* �t�@���N�V�����L�[�̕\��/��\�� */
	void Command_SHOWSTATUSBAR( void );		/* �X�e�[�^�X�o�[�̕\��/��\�� */
	void Command_TYPE_LIST( void );			/* �^�C�v�ʐݒ�ꗗ */
	void Command_OPTION_TYPE( void );		/* �^�C�v�ʐݒ� */
	void Command_OPTION( void );			/* ���ʐݒ� */
	void Command_FONT( void );				/* �t�H���g�ݒ� */
	void Command_WRAPWINDOWWIDTH( void );	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX

	/* �}�N���n */
	void Command_RECKEYMACRO( void );	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	void Command_SAVEKEYMACRO( void );	/* �L�[�}�N���̕ۑ� */
	void Command_LOADKEYMACRO( void );	/* �L�[�}�N���̓ǂݍ��� */
	void Command_EXECKEYMACRO( void );	/* �L�[�}�N���̎��s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void Command_EXECCMMAND( void );	/* �O���R�}���h���s */
	void Command_EXECCOMMAND( void );	/* �O���R�}���h���s */
//	To Here Sept. 20, 2000

	/* �J�X�^�����j���[ */
	void Command_MENU_RBUTTON( void );	/* �E�N���b�N���j���[ */
	int	CreatePopUpMenu_R( void );		/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
	int Command_CUSTMENU( int );		/* �J�X�^�����j���[�\�� */

	/* �E�B���h�E�n */
	void Command_SPLIT_V( void );		/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	void Command_SPLIT_H( void );		/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	void Command_SPLIT_VH( void );		/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	void Command_WINCLOSE( void );		/* �E�B���h�E����� */
	void Command_FILECLOSEALL( void );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	void Command_CASCADE( void );		/* �d�˂ĕ\�� */
	void Command_TILE_V( void );		/* �㉺�ɕ��ׂĕ\�� */
	void Command_TILE_H( void );		/* ���E�ɕ��ׂĕ\�� */
	void Command_MAXIMIZE_V( void );	/* �c�����ɍő剻 */
	void Command_MAXIMIZE_H( void );	/* �������ɍő剻 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* ���ׂčŏ��� */
	void Command_REDRAW( void );		/* �ĕ`�� */
	void Command_WIN_OUTPUT( void );	//�A�E�g�v�b�g�E�B���h�E�\��

	/* �x�� */
	void Command_HOKAN( void );			/* ���͕⊮	*/
	void Command_HELP_CONTENTS( void );	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* �R�}���h�ꗗ */
	void Command_EXTHELP1( void );		/* �O���w���v�P */
	void Command_EXTHTMLHELP( void );	/* �O��HTML�w���v */
	void Command_ABOUT( void );			/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
	void Command_SENDMAIL( void );		/* ���[�����M */

	//	May 23, 2000 genta
	//	��ʕ`��⏕�֐�
	void DrawEOL(HDC hdc, HANDLE hPen, int nPosX, int nPosY, int nWidth, int nHeight,
		CEOL cEol, int bBold );

	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//���ݍs�𗚗��ɒǉ�����

};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
