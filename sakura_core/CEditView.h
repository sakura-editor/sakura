/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_


#include <windows.h>
#include <objidl.h>  // LPDATAOBJECT
#include "CShareData.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEOL.h"
#include "CTextMetrics.h"
#include "CTextDrawer.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CCaret.h"
#include "CRuler.h"
#include "CViewCalc.h"
#include "CViewCommander.h"
#include "CViewParser.h"
#include "CViewSelect.h"
#include "basis/SakuraBasis.h"
#include "CEditView_Paint.h"

class CDropTarget; /// 2002/2/3 aroka �w�b�_�y�ʉ�
class CMemory;///
class COpe;///
class COpeBlk;///
class CSplitBoxWnd;///
class CDlgCancel;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka �w�b�_�y�ʉ� to here
class CEditDoc;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CLayout;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CDocLine;
class CMigemo;	// 2004.09.14 isearch

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

#if !defined(RECONVERTSTRING) && (WINVER < 0x040A)
typedef struct tagRECONVERTSTRING {
    DWORD dwSize;
    DWORD dwVersion;
    DWORD dwStrLen;
    DWORD dwStrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwTargetStrLen;
    DWORD dwTargetStrOffset;
} RECONVERTSTRING, *PRECONVERTSTRING;
#endif // RECONVERTSTRING

///	�}�E�X����R�}���h�����s���ꂽ�ꍇ�̏�ʃr�b�g
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;

class CEditView;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �����E�B���h�E�̊Ǘ�
	
	1�̕����E�B���h�E�ɂ�1��CEditDoc�I�u�W�F�N�g�����蓖�Ă��A
	1��CEditDoc�I�u�W�F�N�g�ɂ��A4��CEditViwe�I�u�W�F�N�g�����蓖�Ă���B
	�E�B���h�E���b�Z�[�W�̏����A�R�}���h���b�Z�[�W�̏����A
	��ʕ\���Ȃǂ��s���B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
//2007.08.25 kobake �����Ԋu�z��̋@�\��CTextMetrics�Ɉړ�
//2007.10.02 kobake Command_TRIM2��CConvert�Ɉړ�

class SAKURA_CORE_API CEditView
: public CViewCalc //$$ ���ꂪ�e�N���X�ł���K�v�͖������A���̃N���X�̃��\�b�h�Ăяo���������̂ŁA�b��I�ɐe�N���X�Ƃ���B
, public CEditView_Paint
{
public:
	const CEditDoc* GetDocument() const
	{
		return m_pcEditDoc;
	}
	CEditDoc* GetDocument()
	{
		return m_pcEditDoc;
	}
	HWND GetHwnd() const
	{
		return m_hWnd;
	}
public:
	//! �w�i�Ƀr�b�g�}�b�v���g�p���邩�ǂ���(������͏��FALSE�BTRUE�̏ꍇ�w�i��悵�Ȃ�)
	bool IsBkBitmap() const{ return false; }

public:
	bool GetDrawSwitch() const
	{
		return m_bDrawSWITCH;
	}
public:
	void SetDrawSwitch(bool b)
	{
		m_bDrawSWITCH = b;
	}
	CEditView* GetEditView()
	{
		return this;
	}
	const CEditView* GetEditView() const
	{
		return this;
	}


public:
	/* Constructors */
	CEditView(CEditWnd* pcEditWnd);
	~CEditView();
	/* �������n�����o�֐� */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	BOOL CreateScrollBar( void );		/* �X�N���[���o�[�쐬 */	// 2006.12.19 ryoji
	void DestroyScrollBar( void );		/* �X�N���[���o�[�j�� */	// 2006.12.19 ryoji

	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�E�擾
	bool IsInsMode() const;
	void SetInsMode(bool);

	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	void OnTimer( HWND, UINT, UINT, DWORD );
	/* �R�}���h���� */
	void CaretUnderLineON( bool );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( bool );								/* �J�[�\���s�A���_�[���C����OFF */
	void AdjustScrollBars( void );								/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	// 2006.07.09 genta �s���w��ɂ��J�[�\���ړ�(�I��̈���l��)
	void MoveCursorSelecting( CLayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE );
	void OnChangeSetting( void );								/* �ݒ�ύX�𔽉f������ */
	void SetFont( void );										/* �t�H���g�̕ύX */
	void RedrawAll( void );										/* �t�H�[�J�X�ړ����̍ĕ`�� */
	void Redraw( void );										// 2001/06/21 asa-o �ĕ`��
	void CopyViewStatus( CEditView* );							/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */

	DWORD DoGrep( const CNativeW*, const CNativeT*, const CNativeT*, BOOL, const SSearchOption&, ECodeType, BOOL, int );	/* Grep���s */
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int DoGrepTree( CDlgCancel*, HWND, const wchar_t*, int*, const TCHAR*, const TCHAR*, BOOL, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int, int* );
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	Mar. 28, 2004 genta �s�v�Ȉ������폜
	int DoGrepFile( CDlgCancel*, HWND, const wchar_t*, int*, const TCHAR*, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int*, const TCHAR*, CNativeW& );
	/* Grep���ʂ�pszWork�Ɋi�[ */
	void SetGrepResult(
		/* �f�[�^�i�[�� */
		wchar_t*	pWork,
		int*		pnWorkLen,
		/* �}�b�`�����t�@�C���̏�� */
		const TCHAR*		pszFullPath,	//	�t���p�X
		const TCHAR*		pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		/* �}�b�`�����s�̏�� */
		int			nLine,			//	�}�b�`�����s�ԍ�
		int			nColm,			//	�}�b�`�������ԍ�
		const wchar_t*	pCompareData,	//	�s�̕�����
		int			nLineLen,		//	�s�̕�����̒���
		int			nEolCodeLen,	//	EOL�̒���
		/* �}�b�`����������̏�� */
		const wchar_t*	pMatchData,		//	�}�b�`����������
		int			nMatchLen,		//	�}�b�`����������̒���
		/* �I�v�V���� */
		BOOL		bGrepOutputLine,
		int			nGrepOutputStyle
	);
	
	//	Jun. 16, 2000 genta
	bool  SearchBracket( const CLayoutPoint& ptPos, CLayoutPoint* pptLayoutNew, int* mode );	// �Ί��ʂ̌���		// mode�̒ǉ� 02/09/18 ai
	bool  SearchBracketForward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int* mode );	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int* mode );	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai
//@@@ 2001.02.03 Start by MIK: �S�p�̑Ί���
	bool  SearchBracketForward2( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
								 const wchar_t* upChar, const wchar_t* dnChar, int* mode );	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward2( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
								  const wchar_t* dnChar, const wchar_t* upChar, int* mode );	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��

// 2002/01/19 novice public�����ɕύX
	BOOL GetSelectedData( CNativeW*, BOOL, const wchar_t*, BOOL, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN);/* �I��͈͂̃f�[�^���擾 */
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON( void );	// IME ON��	// 2006.12.04 ryoji
	int HokanSearchByFile( const wchar_t*, BOOL, CNativeW**, int, int ); // 2003.06.25 Moca

	/*!	CEditView::KeyWordHelpSearchDict�̃R�[�����w��p���[�J��ID
		@date 2006.04.10 fon �V�K�쐬
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon
	BOOL KeySearchCore( const CNativeW* pcmemCurText );	// 2006.04.10 fon
	
	//	Jan. 10, 2005 �C���N�������^���T�[�`
	bool IsISearchEnabled(int nCommand) const;

	//	Jan.  8, 2006 genta �܂�Ԃ��g�O�����씻��
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( CLayoutInt* newKetas );

	void _SetDragMode(BOOL b)
	{
		m_bDragMode = b;
	}
public:
	/*
	||  �����w���p�֐�
	*/
	void GetCurrentTextForSearch( CNativeW& );			/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	void GetCurrentTextForSearchDlg( CNativeW& );		/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j 2006.08.23 ryoji */
public:
//	CEOL GetCurrentInsertEOL( void );					/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �N���b�v�{�[�h                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�擾
	BOOL MyGetClipboardData( CNativeW&, BOOL*, BOOL* = NULL );			/* �N���b�v�{�[�h����f�[�^���擾 */

	//�ݒ�
	BOOL MySetClipboardData( const ACHAR*, int, bool bColmnSelect, bool = FALSE );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	BOOL MySetClipboardData( const WCHAR*, int, bool bColmnSelect, bool = FALSE );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */

	void CopyCurLine( bool bAddCRLFWhenCopy, enumEOLType neweol, bool bEnableLineModePaste );	/* �J�[�\���s���N���b�v�{�[�h�ɃR�s�[���� */	// 2007.10.08 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	void OnPaintOld( HDC, PAINTSTRUCT *, BOOL );	/* �ʏ�̕`�揈�� */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* �ʏ�̕`�揈�� */
	void OnSize( int, int );							/* �E�B���h�E�T�C�Y�̕ύX���� */
	CLayoutInt  OnVScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	CLayoutInt  OnHScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	void OnLBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
	void OnMOUSEMOVE( WPARAM, int, int );				/* �}�E�X�ړ��̃��b�Z�[�W���� */
	void OnLBUTTONUP( WPARAM, int, int );				/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
	void OnLBUTTONDBLCLK( WPARAM, int , int );			/* �}�E�X���{�^���_�u���N���b�N */
	void OnRBUTTONDOWN( WPARAM, int, int );				/* �}�E�X�E�{�^������ */
	void OnRBUTTONUP( WPARAM, int, int );				/* �}�E�X�E�{�^���J�� */
// novice 2004/10/11 �}�E�X���{�^���Ή�
	void OnMBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
	void OnXLBUTTONDOWN( WPARAM, int, int );			/* �}�E�X���T�C�h�{�^������ */
	void OnXRBUTTONDOWN( WPARAM, int, int );			/* �}�E�X�E�T�C�h�{�^������ */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );				/* �}�E�X�z�C�[���̃��b�Z�[�W���� */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetCurrentColor( HDC, int );							/* ���݂̐F���w�� */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// �s�̃e�L�X�g�^�I����Ԃ̕`��
	//	int DispLine( HDC, int, int, int, const unsigned char*, int, BOOL );
	//2007.08.25 kobake �߂�l��int����bool�ɕύX
	bool DispLineNew(
		HDC				hdc,			//!< [in]     ���Ώ�
		const CLayout*	pcLayout,		//!< [in]     �\�����J�n���郌�C�A�E�g
		DispPos*		pDispPos,		//!< [in/out] �`�悷��ӏ��A�`�挳�\�[�X
		CLayoutInt		nLineTo,		//!< [in]     ���I�����郌�C�A�E�g�s�ԍ�
		bool			bSelected		//!< [in]     �I�𒆂�
	);

	void DispTextSelected( HDC hdc, CLayoutInt nLineNum, const CMyPoint& ptXY, CLayoutInt nX_Layout );	/* �e�L�X�g���] */
//	int	DispCtrlCode( HDC, int, int, const unsigned char*, int );	/* �R���g���[���R�[�h�\�� */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �v�Z                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void CopySelectedAllLines( const wchar_t*, BOOL );			/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	void ConvSelectedArea( EFunctionCode );								/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */

public:
	

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt  ScrollAtV( CLayoutInt );										/* �w���[�s�ʒu�փX�N���[�� */
	CLayoutInt  ScrollAtH( CLayoutInt );										/* �w�荶�[���ʒu�փX�N���[�� */
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	CLayoutInt  ScrollByV( CLayoutInt vl ){	return ScrollAtV( GetTextArea().GetViewTopLine() + vl );}			/* �w��s�X�N���[��*/
	CLayoutInt  ScrollByH( CLayoutInt hl ){	return ScrollAtH( GetTextArea().GetViewLeftCol() + hl );}					/* �w�茅�X�N���[�� */
public:
	void SyncScrollV( CLayoutInt );									/* ���������X�N���[�� */
	void SyncScrollH( CLayoutInt );									/* ���������X�N���[�� */

	void DrawBracketPair( bool );								/* �Ί��ʂ̋����\�� 02/09/18 ai */
	void SetBracketPairPos( bool );								/* �Ί��ʂ̋����\���ʒu�ݒ� 03/02/18 ai */
protected:
	int GetColorIndex( HDC, const CLayout*, int );				/* �w��ʒu��ColorIndex�̎擾 02/12/13 ai */
	bool IsBracket( const wchar_t*, CLogicInt, CLogicInt );					/* ���ʔ��� 03/01/09 ai */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �ߋ��̈�Y                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

public:
	void SetIMECompFormPos( void );								/* IME�ҏW�G���A�̈ʒu��ύX */
public:
	void SetIMECompFormFont( void );							/* IME�ҏW�G���A�̕\���t�H���g��ύX */
	void SetParentCaption( void );								/* �e�E�B���h�E�̃^�C�g�����X�V */	// 2007.03.08 ryoji bKillFocus�p�����[�^������
public:

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�픻��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsCurrentPositionURL( const CLayoutPoint& ptCaretPos, CLogicRange* pUrlRange, std::wstring* pwstrURL );/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	BOOL CheckTripleClick( CMyPoint ptMouse );							/* �g���v���N���b�N���`�F�b�N���� */	// 2007.10.02 nasukoji
	int IsCurrentPositionSelected( CLayoutPoint ptCaretPos );					/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	int IsCurrentPositionSelectedTEST( const CLayoutPoint& ptCaretPos, const CLayoutRange& sSelect ) const;/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	bool IsSearchString( const wchar_t*, CLogicInt, CLogicInt, CLogicInt*, CLogicInt* );	/* ���݈ʒu������������ɊY�����邩 */	//2002.02.08 hor �����ǉ�


	void ExecCmd(const TCHAR*, int ) ;							// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	void AddToCmdArr( const TCHAR* );
	BOOL ChangeCurRegexp(void);									// 2002.01.16 hor ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�)
	void SendStatusMessage( const TCHAR* msg );					// 2002.01.26 hor �����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������
	void SendStatusMessage2( const TCHAR* msg );					// Jul. 9, 2005 genta
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* �ĕϊ��p�\���̂�ݒ肷�� 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				/* �ĕϊ��p�\���̂̏������ɑI��͈͂�ύX���� 2002.04.09 minfu */

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* IDropTarget���� */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );


public:


	/* �w��ʒu�̎w�蒷�f�[�^�폜 */
	void CEditView::DeleteData2(
		const CLayoutPoint&	ptCaretPos,
		CLogicInt			nDelLen,
		CNativeW*			pcMem
	);

	void DeleteData( bool bRedraw );/* ���݈ʒu�̃f�[�^�폜 */

	/* ���݈ʒu�Ƀf�[�^��}�� */
	void InsertData_CEditView(
		CLayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		CLayoutPoint*	pptNewPos,	//�}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		bool			bRedraw
	);
	void SmartIndent_CPP( wchar_t );	/* C/C++�X�}�[�g�C���f���g���� */
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */


	/* �f�[�^�u�� �폜&�}���ɂ��g���� */
	void ReplaceData_CEditView(
		CLayoutRange	sDelRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		CNativeW*		pcmemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
		const wchar_t*	pInsData,			// �}������f�[�^
		CLogicInt		nInsDataLen,		// �}������f�[�^�̒���
		bool			bRedraw
	);

	//	Jan. 10, 2005 genta HandleCommand����grep�֘A�����𕪗�
	void TranslateCommand_grep( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );


	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose�ǉ�
	//	Feb. 17, 2007 genta ���΃p�X�̊�f�B���N�g���w����ǉ�
	bool TagJumpSub( const TCHAR* pszJumpToFile, CMyPoint ptJumpTo, bool bClose = false, bool bRelFromIni = false );


//2004.10.13 �C���N�������^���T�[�`�֌W
	void TranslateCommand_isearch( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, bool, LPARAM, LPARAM, LPARAM, LPARAM );

	int	CreatePopUpMenu_R( void );		/* �|�b�v�A�b�v���j���[(�E�N���b�N) */

	/* �x�� */
	//	Jan. 10, 2005 genta HandleCommand����⊮�֘A�����𕪗�
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);

	void ShowHokanMgr( CNativeW& cmemData, BOOL bAutoDecided );	//	�⊮�E�B���h�E��\������BCtrl+Space��A�����̓���/�폜���ɌĂяo����܂��B YAZAKI 2002/03/11


	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//���ݍs�𗚗��ɒǉ�����

	void AnalyzeDiffInfo( const char*, int );	/* DIFF���̉�� */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( TCHAR*, HWND );	/* DIFF�ꎞ�t�@�C���쐬 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const TCHAR*, const TCHAR*, int );		/* DIFF�����\�� */		//2005.10.29 maru
	
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const TCHAR* [], const TCHAR* [], int, int, const TCHAR* ); // �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐� // 2003.08.12 Moca


public:
	//ATOK��p�ĕϊ���API
	HMODULE m_hAtokModule;
	BOOL (WINAPI *AT_ImmSetReconvertString)( HIMC , int ,PRECONVERTSTRING , DWORD  );
private:

	/* �C���N�������^���T�[�` */ 
	//2004.10.24 isearch migemo
	void ISearchEnter( int mode, ESearchDirection direction);
	void ISearchExit();
	void ISearchExec(WORD wChar);
	void ISearchExec(const char* pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CNativeT* msg) const;



public:
	//��v�\�����i�A�N�Z�X
	CTextArea& GetTextArea(){ assert(m_pcTextArea); return *m_pcTextArea; }
	const CTextArea& GetTextArea() const{ assert(m_pcTextArea); return *m_pcTextArea; }
	CCaret& GetCaret(){ assert(m_pcCaret); return *m_pcCaret; }
	const CCaret& GetCaret() const{ assert(m_pcCaret); return *m_pcCaret; }
	CRuler& GetRuler(){ assert(m_pcRuler); return *m_pcRuler; }
	const CRuler& GetRuler() const{ assert(m_pcRuler); return *m_pcRuler; }

	//��v�����A�N�Z�X
	CTextMetrics& GetTextMetrics(){ return m_cTextMetrics; }
	const CTextMetrics& GetTextMetrics() const{ return m_cTextMetrics; }
	CViewSelect& GetSelectionInfo(){ return m_cViewSelect; }
	const CViewSelect& GetSelectionInfo() const{ return m_cViewSelect; }

	//��v�I�u�W�F�N�g�A�N�Z�X
	CViewFont& GetFontset(){ assert(m_pcFontset); return *m_pcFontset; }
	const CViewFont& GetFontset() const{ assert(m_pcFontset); return *m_pcFontset; }

	//��v�w���p�A�N�Z�X
	const CViewParser& GetParser() const{ return m_cParser; }
	const CTextDrawer& GetTextDrawer() const{ return m_cTextDrawer; }
	CViewCommander& GetCommander(){ return m_cCommander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//�Q��
	CEditWnd*	m_pcEditWnd;	//!< �E�B���h�E
	CEditDoc*	m_pcEditDoc;	//!< �h�L�������g

	//��v�\�����i
	CTextArea*		m_pcTextArea;
	CCaret*			m_pcCaret;
	CRuler*			m_pcRuler;

	//��v����
	CTextMetrics	m_cTextMetrics;
	CViewSelect		m_cViewSelect;

	//��v�I�u�W�F�N�g
	CViewFont*		m_pcFontset;

	//��v�w���p
	CViewParser	m_cParser;
	CTextDrawer		m_cTextDrawer;
	CViewCommander	m_cCommander;


public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	CDropTarget*	m_pcDropTarget;
private:
	bool			m_bDrawSWITCH;
public:
	BOOL			m_bDragSource;	/* �I���e�L�X�g�̃h���b�O���� */
	BOOL			m_bDragMode;	/* �I���e�L�X�g�̃h���b�O���� */

	/* �P�ꌟ���̏�� */

	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	m_CurRegexp;			/*!< �R���p�C���f�[�^ */
	bool		m_bCurSrchKeyMark;			/* ����������̃}�[�N */
	wchar_t		m_szCurSrchKey[_MAX_PATH];	/* ���������� */
	
	SSearchOption m_sCurSearchOption;	// �����^�u��  �I�v�V����

	bool	m_bExecutingKeyMacro;		/* �L�[�{�[�h�}�N���̎��s�� */
	HWND	m_hWnd;				/* �ҏW�E�B���h�E�n���h�� */

	CLogicPoint	m_ptSrchStartPos_PHY;	// ����/�u���J�n���̃J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))

	BOOL		m_bSearch;				/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
//	CLogicInt	m_nCharSize;			/* �Ί��ʂ̕����T�C�Y */	// 02/09/18 ai 
	
	CLogicPoint	m_ptBracketCaretPos_PHY;// �O�J�[�\���ʒu�̊��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	CLogicPoint	m_ptBracketPairPos_PHY;	// �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	BOOL	m_bDrawBracketPairFlag;	/* �Ί��ʂ̋����\�����s�Ȃ��� */						// 03/02/18 ai

	TCHAR*			m_pszAppName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HINSTANCE		m_hInstance;	/* �C���X�^���X�n���h�� */
	HWND			m_hwndParent;	/* �e�E�B���h�E�n���h�� */

	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	COpeBlk*		m_pcOpeBlk;			/* ����u���b�N */
	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	int				m_nVScrollRate;		/* �����X�N���[���o�[�̏k�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* ���������{�b�N�X */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* ���������{�b�N�X */

	HDC				m_hdcCompatDC;		/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */
public:
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* �R�}���h�̎��s�� */

	/* ������� */
	int				m_nMyIndex;

	/* ���͏�� */
	COLORREF	m_crBack;			/* �e�L�X�g�̔w�i�F */			// 2006.12.07 ryoji
	int		m_nOldUnderLineY;




	/* �\�����@ */
public:
	/* ����Tip�֘A */
	DWORD	m_dwTipTimer;			/* Tip�N���^�C�}�[ */
	CTipWnd	m_cTipWnd;				/* Tip�\���E�B���h�E */
	POINT	m_poTipCurPos;			/* Tip�N�����̃}�E�X�J�[�\���ʒu */
	BOOL	m_bInMenuLoop;			/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
	CDicMgr	m_cDicMgr;				/* �����}�l�[�W�� */

	/* ���͕⊮ */
	BOOL		m_bHokan;			//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH

	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump����
	CRegexKeyword	*m_cRegexKeyword;	//@@@ 2001.11.17 add MIK

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;
public:
	UINT	m_uWM_MSIME_RECONVERTREQUEST;
private:
	
	int		m_nLastReconvLine;             //2002.04.09 minfu �ĕϊ����ۑ��p;
	int		m_nLastReconvIndex;            //2002.04.09 minfu �ĕϊ����ۑ��p;

	bool	m_bUnderLineON;

	CMigemo* m_pcmigemo;

	ESearchDirection m_nISearchDirection;
	int m_nISearchMode;
	bool m_bISearchWrap;

	CLayoutRange m_sISearchHistory[256];

	bool m_bISearchFlagHistory[256];
	int m_nISearchHistoryCount;
	bool m_bISearchFirst;

	// 2007.10.02 nasukoji
	bool	m_bActivateByMouse;		//!< �}�E�X�ɂ��A�N�e�B�x�[�g

	// 2007.10.02 nasukoji
	DWORD	m_dwTripleClickCheck;	//!< �g���v���N���b�N�`�F�b�N�p����
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
