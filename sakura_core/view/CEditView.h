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
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_


#include <windows.h>
#include <objidl.h>  // LPDATAOBJECT
#include <shellapi.h>  // HDROP
#include "window/CTipWnd.h"
#include "CDicMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEol.h"
#include "CTextMetrics.h"
#include "CTextDrawer.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CCaret.h"
#include "CRuler.h"
#include "CViewCalc.h" // parent
#include "CViewCommander.h"
#include "CViewParser.h"
#include "CViewSelect.h"
#include "basis/SakuraBasis.h"
#include "CEditView_Paint.h"
#include "mfclike/CMyWnd.h"
#include "doc/CDocListener.h"

class CEditView;

class CDropTarget; /// 2002/2/3 aroka �w�b�_�y�ʉ�
class COpeBlk;///
class CSplitBoxWnd;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka �w�b�_�y�ʉ� to here
class CEditDoc;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CLayout;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CMigemo;	// 2004.09.14 isearch
struct SColorStrategyInfo;

// struct DispPos; //	�N����include���Ă܂�
// class CColorStrategy;	// �N����include���Ă܂�
// enum EColorIndexType;	// �N����include���Ă܂�


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
, public CMyWnd
, public CDocListenerEx
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
public:
	//! �w�i�Ƀr�b�g�}�b�v���g�p���邩�ǂ���(������͏��FALSE�BTRUE�̏ꍇ�w�i��悵�Ȃ�)
	bool IsBkBitmap() const{ return false; }

public:
	CEditView* GetEditView()
	{
		return this;
	}
	const CEditView* GetEditView() const
	{
		return this;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����Ɣj��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* Constructors */
	CEditView(CEditWnd* pcEditWnd);
	~CEditView();
	/* �������n�����o�֐� */
	BOOL Create(
		HWND		hwndParent,	//!< �e
		CEditDoc*	pcEditDoc,	//!< �Q�Ƃ���h�L�������g
		int			nMyIndex,	//!< �r���[�̃C���f�b�N�X
		BOOL		bShow		//!< �쐬���ɕ\�����邩�ǂ���
	);
	void CopyViewStatus( CEditView* ) const;					/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �N���b�v�{�[�h                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//�擾
	bool MyGetClipboardData( CNativeW&, bool*, bool* = NULL );			/* �N���b�v�{�[�h����f�[�^���擾 */

	//�ݒ�
	bool MySetClipboardData( const ACHAR*, int, bool bColmnSelect, bool = false );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	bool MySetClipboardData( const WCHAR*, int, bool bColmnSelect, bool = false );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */

	//���p
	void CopyCurLine( bool bAddCRLFWhenCopy, enumEOLType neweol, bool bEnableLineModePaste );	/* �J�[�\���s���N���b�v�{�[�h�ɃR�s�[���� */	// 2007.10.08 ryoji
	void CopySelectedAllLines( const wchar_t*, BOOL );			/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//�h�L�������g�C�x���g
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	//
	void OnChangeSetting();										/* �ݒ�ύX�𔽉f������ */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* �ʏ�̕`�揈�� */
	void OnTimer( HWND, UINT, UINT, DWORD );
	//�E�B���h�E
	void OnSize( int, int );							/* �E�B���h�E�T�C�Y�̕ύX���� */
	void OnMove( int, int, int, int );
	//�t�H�[�J�X
	void OnSetFocus( void );
	void OnKillFocus( void );
	//�X�N���[��
	CLayoutInt  OnVScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	CLayoutInt  OnHScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	//�}�E�X
	void OnLBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
	void OnMOUSEMOVE( WPARAM, int, int );				/* �}�E�X�ړ��̃��b�Z�[�W���� */
	void OnLBUTTONUP( WPARAM, int, int );				/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
	void OnLBUTTONDBLCLK( WPARAM, int , int );			/* �}�E�X���{�^���_�u���N���b�N */
	void OnRBUTTONDOWN( WPARAM, int, int );				/* �}�E�X�E�{�^������ */
	void OnRBUTTONUP( WPARAM, int, int );				/* �}�E�X�E�{�^���J�� */
	void OnMBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
	void OnMBUTTONUP( WPARAM, int, int );				/* �}�E�X���{�^���J�� */
	void OnXLBUTTONDOWN( WPARAM, int, int );			/* �}�E�X�T�C�h�{�^��1���� */
	void OnXLBUTTONUP( WPARAM, int, int );				/* �}�E�X�T�C�h�{�^��1�J�� */		// 2009.01.17 nasukoji
	void OnXRBUTTONDOWN( WPARAM, int, int );			/* �}�E�X�T�C�h�{�^��2���� */
	void OnXRBUTTONUP( WPARAM, int, int );				/* �}�E�X�T�C�h�{�^��2�J�� */		// 2009.01.17 nasukoji
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );				/* �}�E�X�z�C�[���̃��b�Z�[�W���� */
	int  IsSpecialScrollMode( int );					/* �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷�� */		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2006.05.14 Moca  �݊�BMP�ɂ���ʃo�b�t�@
	// 2007.09.30 genta CompatibleDC����֐�
protected:
	//! ���W�b�N�s��1�s�`��
	bool DrawLogicLine(
		HDC				hdc,			//!< [in]     ���Ώ�
		DispPos*		pDispPos,		//!< [in/out] �`�悷��ӏ��A�`�挳�\�[�X
		CLayoutInt		nLineTo			//!< [in]     ���I�����郌�C�A�E�g�s�ԍ�
	);

	//! ���C�A�E�g�s��1�s�`��
	bool DrawLayoutLine(SColorStrategyInfo* pInfo);

	//�F����
public:
	EColorIndexType GetColorIndex( const CLayout*, int, bool bPrev = false, CColorStrategy** ppStrategy = NULL, CColorStrategy** ppStrategyFound = NULL );				/* �w��ʒu��ColorIndex�̎擾 02/12/13 ai */
	void SetCurrentColor( CGraphics& gr, EColorIndexType );							/* ���݂̐F���w�� */

	//��ʃo�b�t�@
protected:
	bool CreateOrUpdateCompatibleBitmap( int cx, int cy );	//!< ������BMP���쐬�܂��͍X�V
	void UseCompatibleDC(BOOL fCache);
public:
	void DeleteCompatibleBitmap();							//!< ������BMP���폜

public:
	void DispTextSelected( HDC hdc, CLayoutInt nLineNum, const CMyPoint& ptXY, CLayoutInt nX_Layout );	/* �e�L�X�g���] */
	void RedrawAll();											/* �t�H�[�J�X�ړ����̍ĕ`�� */
	void Redraw();										// 2001/06/21 asa-o �ĕ`��
	void CaretUnderLineON( bool );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( bool );								/* �J�[�\���s�A���_�[���C����OFF */
	bool GetDrawSwitch() const
	{
		return m_bDrawSWITCH;
	}
	void SetDrawSwitch(bool b)
	{
		m_bDrawSWITCH = b;
	}



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AdjustScrollBars();											/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	BOOL CreateScrollBar();												/* �X�N���[���o�[�쐬 */	// 2006.12.19 ryoji
	void DestroyScrollBar();											/* �X�N���[���o�[�j�� */	// 2006.12.19 ryoji
	CLayoutInt GetWrapOverhang( void ) const;							/* �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z */	// 2008.06.08 ryoji
	CLayoutInt ViewColNumToWrapColNum( CLayoutInt nViewColNum ) const;	/* �u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z���� */	// 2008.06.08 ryoji
	CLayoutInt GetRightEdgeForScrollBar( void );								/* �X�N���[���o�[����p�ɉE�[���W���擾���� */		// 2009.08.28 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON( void );	// IME ON��	// 2006.12.04 ryoji
	

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt  ScrollAtV( CLayoutInt );										/* �w���[�s�ʒu�փX�N���[�� */
	CLayoutInt  ScrollAtH( CLayoutInt );										/* �w�荶�[���ʒu�փX�N���[�� */
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	CLayoutInt  ScrollByV( CLayoutInt vl ){	return ScrollAtV( GetTextArea().GetViewTopLine() + vl );}	/* �w��s�X�N���[��*/
	CLayoutInt  ScrollByH( CLayoutInt hl ){	return ScrollAtH( GetTextArea().GetViewLeftCol() + hl );}	/* �w�茅�X�N���[�� */
public:
	void SyncScrollV( CLayoutInt );									/* ���������X�N���[�� */
	void SyncScrollH( CLayoutInt );									/* ���������X�N���[�� */

	void SetBracketPairPos( bool );								/* �Ί��ʂ̋����\���ʒu�ݒ� 03/02/18 ai */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �ߋ��̈�Y                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void SetIMECompFormPos( void );								/* IME�ҏW�G���A�̈ʒu��ύX */
	void SetIMECompFormFont( void );							/* IME�ҏW�G���A�̕\���t�H���g��ύX */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �e�L�X�g�I��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2002/01/19 novice public�����ɕύX
	bool GetSelectedData( CNativeW*, BOOL, const wchar_t*, BOOL, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN);/* �I��͈͂̃f�[�^���擾 */
	int IsCurrentPositionSelected( CLayoutPoint ptCaretPos );					/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	int IsCurrentPositionSelectedTEST( const CLayoutPoint& ptCaretPos, const CLayoutRange& sSelect ) const;/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	// 2006.07.09 genta �s���w��ɂ��J�[�\���ړ�(�I��̈���l��)
	void MoveCursorSelecting( CLayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE );
	void ConvSelectedArea( EFunctionCode );								/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	//!�w��ʒu�܂��͎w��͈͂��e�L�X�g�̑��݂��Ȃ��G���A���`�F�b�N����		// 2008.08.03 nasukoji
	bool IsEmptyArea( CLayoutPoint ptFrom, CLayoutPoint ptTo = CLayoutPoint( CLayoutInt(-1), CLayoutInt(-1) ), bool bSelect = false, bool bBoxSelect = false ) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�픻��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL( const CLayoutPoint& ptCaretPos, CLogicRange* pUrlRange, std::wstring* pwstrURL );/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	BOOL CheckTripleClick( CMyPoint ptMouse );							/* �g���v���N���b�N���`�F�b�N���� */	// 2007.10.02 nasukoji



	void ExecCmd(const TCHAR*, int ) ;							// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	void AddToCmdArr( const TCHAR* );
	BOOL ChangeCurRegexp(void);									// 2002.01.16 hor ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�)
	void SendStatusMessage( const TCHAR* msg );					// 2002.01.26 hor �����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������
	void SendStatusMessage2( const TCHAR* msg );					// Jul. 9, 2005 genta
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* �ĕϊ��p�\���̂�ݒ肷�� 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				/* �ĕϊ��p�\���̂̏������ɑI��͈͂�ύX���� 2002.04.09 minfu */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           D&D                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* IDropTarget���� */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP PostMyDropFiles( LPDATAOBJECT pDataObject );		/* �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g���� */	// 2008.06.20 ryoji
	void OnMyDropFiles( HDROP hDrop );								/* �Ǝ��h���b�v�t�@�C�����b�Z�[�W���� */	// 2008.06.20 ryoji
	CLIPFORMAT GetAvailableClipFormat( LPDATAOBJECT pDataObject );
	DWORD TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect );
	bool IsDragSource( void );

	void _SetDragMode(BOOL b)
	{
		m_bDragMode = b;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ҏW                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* �w��ʒu�̎w�蒷�f�[�^�폜 */
	void DeleteData2(
		const CLayoutPoint&	ptCaretPos,
		CLogicInt			nDelLen,
		CNativeW*			pcMem
	);

	/* ���݈ʒu�̃f�[�^�폜 */
	void DeleteData( bool bRedraw );

	/* ���݈ʒu�Ƀf�[�^��}�� */
	void InsertData_CEditView(
		CLayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		CLayoutPoint*	pptNewPos,	//�}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		bool			bRedraw
	);

	/* �f�[�^�u�� �폜&�}���ɂ��g���� */
	void ReplaceData_CEditView(
		CLayoutRange	sDelRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		CNativeW*		pcmemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
		const wchar_t*	pInsData,			// �}������f�[�^
		CLogicInt		nInsDataLen,		// �}������f�[�^�̒���
		bool			bRedraw,
		COpeBlk*		pcOpeBlk
	);
	void ReplaceData_CEditView2(
		const CLogicRange&	sDelRange,			// �폜�͈́B���W�b�N�P�ʁB
		CNativeW*			pcmemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
		const wchar_t*		pInsData,			// �}������f�[�^
		CLogicInt			nInsDataLen,		// �}������f�[�^�̒���
		bool				bRedraw,
		COpeBlk*			pcOpeBlk
	);
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */

	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�E�擾
	bool IsInsMode() const;
	void SetInsMode(bool);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2004.10.13 �C���N�������^���T�[�`�֌W
	void TranslateCommand_isearch( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, bool, LPARAM, LPARAM, LPARAM, LPARAM );

	//	Jan. 10, 2005 genta HandleCommand����grep�֘A�����𕪗�
	void TranslateCommand_grep( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );

	//	Jan. 10, 2005 �C���N�������^���T�[�`
	bool IsISearchEnabled(int nCommand) const;

	BOOL KeySearchCore( const CNativeW* pcmemCurText );	// 2006.04.10 fon

	/*!	CEditView::KeyWordHelpSearchDict�̃R�[�����w��p���[�J��ID
		@date 2006.04.10 fon �V�K�쐬
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon

	bool IsSearchString( const CStringRef& cStr, CLogicInt, CLogicInt*, CLogicInt* ) const;	/* ���݈ʒu������������ɊY�����邩 */	//2002.02.08 hor �����ǉ�

	void GetCurrentTextForSearch( CNativeW& );			/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	void GetCurrentTextForSearchDlg( CNativeW& );		/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j 2006.08.23 ryoji */

private:
	/* �C���N�������^���T�[�` */ 
	//2004.10.24 isearch migemo
	void ISearchEnter( int mode, ESearchDirection direction);
	void ISearchExit();
	void ISearchExec(DWORD wChar);
	void ISearchExec(LPCWSTR pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CNativeT* msg) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
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
	void DrawBracketPair( bool );								/* �Ί��ʂ̋����\�� 02/09/18 ai */
	bool IsBracket( const wchar_t*, CLogicInt, CLogicInt );					/* ���ʔ��� 03/01/09 ai */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �⊮                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* �x�� */
	//	Jan. 10, 2005 genta HandleCommand����⊮�֘A�����𕪗�
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);

	// �⊮�E�B���h�E��\������BCtrl+Space��A�����̓���/�폜���ɌĂяo����܂��B YAZAKI 2002/03/11
	void ShowHokanMgr( CNativeW& cmemData, BOOL bAutoDecided );

	int HokanSearchByFile( const wchar_t*, BOOL, CNativeW**, int, int ); // 2003.06.25 Moca


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �W�����v                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose�ǉ�
	//	Feb. 17, 2007 genta ���΃p�X�̊�f�B���N�g���w����ǉ�
	bool TagJumpSub( const TCHAR* pszJumpToFile, CMyPoint ptJumpTo, bool bClose = false, bool bRelFromIni = false );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���j���[                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int	CreatePopUpMenu_R( void );		/* �|�b�v�A�b�v���j���[(�E�N���b�N) */



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           DIFF                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AnalyzeDiffInfo( const char*, int );	/* DIFF���̉�� */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( TCHAR*, HWND );	/* DIFF�ꎞ�t�@�C���쐬 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const TCHAR*, const TCHAR*, int );		/* DIFF�����\�� */		//2005.10.29 maru

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//���ݍs�𗚗��ɒǉ�����


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          ���̑�                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const TCHAR* [], const TCHAR* [], int, int, const TCHAR* ); // �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐� // 2003.08.12 Moca
	//	Jan.  8, 2006 genta �܂�Ԃ��g�O�����씻��
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( CLayoutInt* newKetas );
	void SmartIndent_CPP( wchar_t );	/* C/C++�X�}�[�g�C���f���g���� */
	/* �R�}���h���� */
	void SetFont( void );										/* �t�H���g�̕ύX */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �A�N�Z�T                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
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
	CEditWnd*		m_pcEditWnd;	//!< �E�B���h�E
	CEditDoc*		m_pcEditDoc;	//!< �h�L�������g

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
	CViewParser		m_cParser;
	CTextDrawer		m_cTextDrawer;
	CViewCommander	m_cCommander;

public:
	//�E�B���h�E
	HWND			m_hwndParent;		/* �e�E�B���h�E�n���h�� */
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	int				m_nVScrollRate;		/* �����X�N���[���o�[�̏k�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* ���������{�b�N�X */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* ���������{�b�N�X */

public:
	//�`��
	bool			m_bDrawSWITCH;
	COLORREF		m_crBack;				/* �e�L�X�g�̔w�i�F */			// 2006.12.07 ryoji
	int				m_nOldUnderLineY;		// �O���悵���J�[�\���A���_�[���C���̈ʒu 0����=��\��
	int				m_nOldCursorLineX;		/* �O���悵���J�[�\���ʒu�c���̈ʒu */ // 2007.09.09 Moca
	bool			m_bUnderLineON;

public:
	//��ʃo�b�t�@
	HDC				m_hdcCompatDC;		/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */
	int				m_nCompatBMPWidth;  /* �č��p�������a�l�o�̕� */	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	int				m_nCompatBMPHeight; /* �č��p�������a�l�o�̍��� */	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@

public:
	//D&D
	CDropTarget*	m_pcDropTarget;
	BOOL			m_bDragMode;	/* �I���e�L�X�g�̃h���b�O���� */
	CLIPFORMAT		m_cfDragData;	/* �h���b�O�f�[�^�̃N���b�v�`�� */	// 2008.06.20 ryoji
	BOOL			m_bDragBoxData;	/* �h���b�O�f�[�^�͋�`�� */
	CLayoutPoint	m_ptCaretPos_DragEnter;			/* �h���b�O�J�n���̃J�[�\���ʒu */	// 2007.12.09 ryoji
	CLayoutInt		m_nCaretPosX_Prev_DragEnter;	/* �h���b�O�J�n����X���W�L�� */	// 2007.12.09 ryoji

	//����
	CLogicPoint		m_ptBracketCaretPos_PHY;	// �O�J�[�\���ʒu�̊��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	CLogicPoint		m_ptBracketPairPos_PHY;		// �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	BOOL			m_bDrawBracketPairFlag;		/* �Ί��ʂ̋����\�����s�Ȃ��� */						// 03/02/18 ai

	//�}�E�X
	bool			m_bActivateByMouse;		//!< �}�E�X�ɂ��A�N�e�B�x�[�g	//2007.10.02 nasukoji
	DWORD			m_dwTripleClickCheck;	//!< �g���v���N���b�N�`�F�b�N�p����	//2007.10.02 nasukoji

	//����
	mutable CBregexp	m_CurRegexp;				/*!< �R���p�C���f�[�^ */
	bool				m_bCurSrchKeyMark;			/* ����������̃}�[�N */
	wchar_t				m_szCurSrchKey[_MAX_PATH];	/* ���������� */
	SSearchOption		m_sCurSearchOption;			// �����^�u��  �I�v�V����
	CLogicPoint			m_ptSrchStartPos_PHY;		// ����/�u���J�n���̃J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	BOOL				m_bSearch;					/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	ESearchDirection	m_nISearchDirection;
	int					m_nISearchMode;
	bool				m_bISearchWrap;
	bool				m_bISearchFlagHistory[256];
	int					m_nISearchHistoryCount;
	bool				m_bISearchFirst;
	CLayoutRange		m_sISearchHistory[256];

	//�}�N��
	bool			m_bExecutingKeyMacro;		/* �L�[�{�[�h�}�N���̎��s�� */
	BOOL			m_bCommandRunning;	/* �R�}���h�̎��s�� */

	// ���͕⊮
	BOOL			m_bHokan;			//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH

	//�ҏW
	COpeBlk*		m_pcOpeBlk;			/* ����u���b�N */
	bool			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */

	// ����Tip�֘A
	DWORD			m_dwTipTimer;			/* Tip�N���^�C�}�[ */
	CTipWnd			m_cTipWnd;				/* Tip�\���E�B���h�E */
	POINT			m_poTipCurPos;			/* Tip�N�����̃}�E�X�J�[�\���ʒu */
	BOOL			m_bInMenuLoop;			/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
	CDicMgr			m_cDicMgr;				/* �����}�l�[�W�� */

	// IME
private:
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;
public:
	UINT			m_uWM_MSIME_RECONVERTREQUEST;
private:
	int				m_nLastReconvLine;             //2002.04.09 minfu �ĕϊ����ۑ��p;
	int				m_nLastReconvIndex;            //2002.04.09 minfu �ĕϊ����ۑ��p;

public:
	//ATOK��p�ĕϊ���API
	typedef BOOL (WINAPI *FP_ATOK_RECONV)( HIMC , int ,PRECONVERTSTRING , DWORD  );
	HMODULE			m_hAtokModule;
	FP_ATOK_RECONV	m_AT_ImmSetReconvertString;

	// ���̑�
	CAutoMarkMgr*	m_cHistory;	//	Jump����
	CRegexKeyword*	m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	int				m_nMyIndex;	/* ������� */
	CMigemo*		m_pcmigemo;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */




