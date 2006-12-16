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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_

#define _CARETMARGINRATE 20

#include <windows.h>
#include "CShareData.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEOL.h"

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
class SAKURA_CORE_API CCaretUnderLine
{
public:
	CCaretUnderLine(){
		m_nLockCounter = 0;
	};
	void Lock(){	//	�\����\����؂�ւ����Ȃ��悤�ɂ���
		m_nLockCounter++;
	}
	void UnLock(){	//	�\����\����؂�ւ�����悤�ɂ���
		m_nLockCounter--;
		if (m_nLockCounter < 0){
			m_nLockCounter = 0;
		};
	}
	void CaretUnderLineON( BOOL );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( BOOL );								/* �J�[�\���s�A���_�[���C����OFF */
	void SetView( CEditView* pcEditView ){
		m_pcEditView = pcEditView;
	};
protected:
	/* ���b�N�J�E���^�B0�̂Ƃ��́A���b�N����Ă��Ȃ��BUnLock���Ă΂ꂷ���Ă����ɂ͂Ȃ�Ȃ� */
	int m_nLockCounter;
	CEditView* m_pcEditView;
};

/*!
	@brief �����E�B���h�E�̊Ǘ�
	
	1�̕����E�B���h�E�ɂ�1��CEditDoc�I�u�W�F�N�g�����蓖�Ă��A
	1��CEditDoc�I�u�W�F�N�g�ɂ��A4��CEditViwe�I�u�W�F�N�g�����蓖�Ă���B
	�E�B���h�E���b�Z�[�W�̏����A�R�}���h���b�Z�[�W�̏����A
	��ʕ\���Ȃǂ��s���B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CEditView/* : public CDropTarget*/
{
public:
	/* Constructors */
	CEditView();
	~CEditView();
	/* �������n�����o�֐� */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	/* ��� */
	//<< 2002/03/29 Azumaiya �C�����C���֐���
	BOOL IsTextSelected( void )		/* �e�L�X�g���I������Ă��邩 */
	{
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		// ���ׂĂ� or ���Z������ɁA�r�b�g���]���čŏ�ʃr�b�g(�����t���O)��Ԃ��B
		/* ���Ȃ킿�A�����ꂩ���ЂƂł�-1(0xFFFF)�Ȃ�AFALSE��Ԃ��H */
		return ~((DWORD)(m_nSelectLineFrom|m_nSelectLineTo|m_nSelectColmFrom|m_nSelectColmTo)) >> 31;
	};
	BOOL IsTextSelecting( void )	/* �e�L�X�g�̑I�𒆂� */
	{
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		return m_bSelectingLock|IsTextSelected();
	};
	//>> 2002/03/29 Azumaiya
	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�E�擾
	bool IsInsMode() const;
	void SetInsMode(bool);

	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	void OnTimer( HWND, UINT, UINT, DWORD );
	BOOL HandleCommand( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );
	/* �R�}���h���� */
	void CaretUnderLineON( BOOL );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( BOOL );								/* �J�[�\���s�A���_�[���C����OFF */
	void AdjustScrollBars( void );								/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	int  MoveCursor( int, int, BOOL, int = _CARETMARGINRATE );	/* �s���w��ɂ��J�[�\���ړ� */
	// 2006.07.09 genta �s���w��ɂ��J�[�\���ړ�(�I��̈���l��)
	void MoveCursorSelecting( int, int, BOOL, int = _CARETMARGINRATE );
	BOOL GetAdjustCursorPos( int *, int *);	// �������J�[�\���ʒu���Z�o����
	BOOL DetectWidthOfLineNumberArea( BOOL );					/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	int DetectWidthOfLineNumberArea_calculate( void );			/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
	void DisableSelectArea( BOOL );								/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	void OnChangeSetting( void );								/* �ݒ�ύX�𔽉f������ */
	void SetFont( void );										/* �t�H���g�̕ύX */
	void RedrawAll( void );										/* �t�H�[�J�X�ړ����̍ĕ`�� */
	void Redraw( void );										// 2001/06/21 asa-o �ĕ`��
	void CopyViewStatus( CEditView* );							/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
	DWORD DoGrep( const CMemory*, const CMemory*, const CMemory*, BOOL, BOOL, BOOL, int, BOOL, BOOL, int );	/* Grep���s */
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int DoGrepTree( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, int, BOOL, BOOL, int, CBregexp*, int, int* );
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	Mar. 28, 2004 genta �s�v�Ȉ������폜
	int DoGrepFile( CDlgCancel*, HWND, const char*, int*, const char*, BOOL, BOOL, int, BOOL, BOOL, int, CBregexp*, int*, const char*, CMemory& );
	/* Grep���ʂ�pszWork�Ɋi�[ */
	void SetGrepResult(
		/* �f�[�^�i�[�� */
		char*		pWork,
		int*		pnWorkLen,
		/* �}�b�`�����t�@�C���̏�� */
		const char*		pszFullPath,	//	�t���p�X
		const char*		pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		/* �}�b�`�����s�̏�� */
		int			nLine,			//	�}�b�`�����s�ԍ�
		int			nColm,			//	�}�b�`�������ԍ�
		const char*		pCompareData,	//	�s�̕�����
		int			nLineLen,		//	�s�̕�����̒���
		int			nEolCodeLen,	//	EOL�̒���
		/* �}�b�`����������̏�� */
		const char*		pMatchData,		//	�}�b�`����������
		int			nMatchLen,		//	�}�b�`����������̒���
		/* �I�v�V���� */
		BOOL		bGrepOutputLine,
		int			nGrepOutputStyle
	);
	
	//	Jun. 16, 2000 genta
	bool  SearchBracket( int PosX, int PosY, int* NewX, int* NewY, int* mode );	// �Ί��ʂ̌���		// mode�̒ǉ� 02/09/18 ai
	bool  SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
						char* upChar, char* dnChar, int* mode );	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
						char* dnChar, char* upChar, int* mode );	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai
//@@@ 2001.02.03 Start by MIK: �S�p�̑Ί���
	bool  SearchBracketForward2( int PosX, int PosY, int* NewX, int* NewY,
								 char* upChar, char* dnChar, int* mode );	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward2( int PosX, int PosY, int* NewX, int* NewY,
								  char* dnChar, char* upChar, int* mode );	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��

// 2002/01/19 novice public�����ɕύX
	BOOL GetSelectedData( CMemory&, BOOL, const char*, BOOL, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN);/* �I��͈͂̃f�[�^���擾 */
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON( void );	// IME ON��	// 2006.12.04 ryoji
	void CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight );		/* �L�����b�g�̍쐬 */	// 2006.12.07 ryoji
	void ShowEditCaret( void );									/* �L�����b�g�̕\���E�X�V */
	int HokanSearchByFile( const char*, BOOL, CMemory**, int, int ); // 2003.06.25 Moca

	/*!	CEditView::KeyWordHelpSearchDict�̃R�[�����w��p���[�J��ID
		@date 2006.04.10 fon �V�K�쐬
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon
	BOOL KeySearchCore( const CMemory* pcmemCurText );	// 2006.04.10 fon
	
	//	Jan. 10, 2005 �C���N�������^���T�[�`
	bool IsISearchEnabled(int nCommand) const;

	//	Jan.  8, 2006 genta �܂�Ԃ��g�O�����씻��
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( int& );

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
//	HANDLE	m_hThreadGrep;	@@@ 2002.01.03 YAZAKI �g�p���Ă��Ȃ�����
	HWND	m_hWnd;				/* �ҏW�E�B���h�E�n���h�� */
	int		m_nViewTopLine;		/* �\����̈�ԏ�̍s(0�J�n) */
	int		m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */
	/*!	@brief ���O��X���W�L���p

		�t���[�J�[�\�����[�h�łȂ��ꍇ�ɃJ�[�\�����㉺�Ɉړ��������ꍇ
		�J�[�\���ʒu���Z���s�ł͍s���ɃJ�[�\�����ړ����邪�C
		����Ɉړ��𑱂����ꍇ�ɒ����s�ňړ��N�_��X�ʒu�𕜌��ł���悤��
		���邽�߂̕ϐ��D
		
		@par �g����
		�ǂݏo����CEditView::Cursor_UPDOWN()�݂̂ōs���D
		�J�[�\���㉺�ړ��ȊO�ŃJ�[�\���ړ����s�����ꍇ�ɂ�
		������m_nCaretPosX�̒l��ݒ肷��D�������Ȃ���
		���̒���̃J�[�\���㉺�ړ��ňړ��O��X���W�ɖ߂��Ă��܂��D
	
		�r���[���[����̃J�[�\�����ʒu(�O�J�n)
		
		@date 2004.04.09 genta �������ǉ�
	*/
	int		m_nCaretPosX_Prev;
	int		m_nCaretPosX;		/* �r���[���[����̃J�[�\�����ʒu�i�O�J�n�j*/
	int		m_nCaretPosY;		/* �r���[��[����̃J�[�\���s�ʒu�i�O�J�n�j*/
	int		m_nCaretPosX_PHY;	/* �J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	int		m_nCaretPosY_PHY;	/* �J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/

	int		m_nSrchStartPosX_PHY;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/06/26 ai
	int		m_nSrchStartPosY_PHY;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ�(0�J�n) */				// 02/06/26 ai
	BOOL	m_bSearch;				/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	int		m_nCharSize;			/* �Ί��ʂ̕����T�C�Y */	// 02/09/18 ai 
	int		m_nBracketCaretPosX_PHY;/* �O�J�[�\���ʒu�̊��ʂ̈ʒu ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 03/02/18 ai
	int		m_nBracketCaretPosY_PHY;/* �O�J�[�\���ʒu�̊��ʂ̈ʒu ���s�P�ʍs�̍s�ԍ�(0�J�n) */				// 03/02/18 ai
	int		m_nBracketPairPosX_PHY;	/* �Ί��ʂ̈ʒu ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/12/13 ai
	int		m_nBracketPairPosY_PHY;	/* �Ί��ʂ̈ʒu ���s�P�ʍs�̍s�ԍ�(0�J�n) */			// 02/12/13 ai
	BOOL	m_bDrawBracketPairFlag;	/* �Ί��ʂ̋����\�����s�Ȃ��� */						// 03/02/18 ai
	BOOL	m_bDrawSelectArea;		/* �I��͈͂�`�悵���� */	// 02/12/13 ai

	/*
	||  �����o�ϐ�
	*/
	char*			m_pszAppName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HINSTANCE		m_hInstance;	/* �C���X�^���X�n���h�� */
	HWND			m_hwndParent;	/* �e�E�B���h�E�n���h�� */

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
	//	Dec. 4, 2002 genta
	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��CEditWnd�ֈڊ�
	//HFONT			m_hFontCaretPosInfo;/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	//int			m_nCaretPosInfoCharWidth;
	//int			m_nCaretPosInfoCharHeight;
	//int			m_pnCaretPosInfoDx[64];	/* ������`��p�������z�� */
	HDC				m_hdcCompatDC;		/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */
//@@@2002.01.14 YAZAKI static�ɂ��ă������̐ߖ�i(10240+10) * 3 �o�C�g�j
	static int		m_pnDx[MAXLINESIZE + 10];	/* ������`��p�������z�� */
	HFONT			m_hFont_HAN;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT;	/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_UL;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT_UL;	/* ���݂̃t�H���g�n���h�� */
//	2004.02.14 Moca m_hFont_ZEN�́A���g�p�ɂ��폜
//	HFONT			m_hFont_ZEN;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* �R�}���h�̎��s�� */
	/* ������� */
	int				m_nMyIndex;
	/* �L�[���s�[�g��� */
	int				m_bPrevCommand;
	/* ���͏�� */
	int		m_nCaretWidth;			/* �L�����b�g�̕� */
	int		m_nCaretHeight;			/* �L�����b�g�̍��� */
	COLORREF	m_crCaret;			/* �L�����b�g�̐F */			// 2006.12.07 ryoji
	COLORREF	m_crBack;			/* �e�L�X�g�̔w�i�F */			// 2006.12.07 ryoji
	HBITMAP	m_hbmpCaret;			/* �L�����b�g�̃r�b�g�}�b�v */	// 2006.11.28 ryoji
	CCaretUnderLine m_cUnderLine;	/* �A���_�[���C�� */
	int		m_nOldUnderLineY;

	int		m_nOldCaretPosX;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	int		m_nOldCaretWidth;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
	bool	m_bRedrawRuler;		// ���[���[�S�̂�`�������� = true      2002.02.25 Add By KK

	/* �I����� */
	int		m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	int		m_bBeginSelect;			/* �͈͑I�� */
	int		m_bBeginBoxSelect;		/* ��`�͈͑I�� */
	int		m_bBeginLineSelect;		/* �s�P�ʑI�� */
	int		m_bBeginWordSelect;		/* �P��P�ʑI�� */
	/* �I��͈͂�ێ����邽�߂̕ϐ��Q
		�����͂��ׂĐ܂�Ԃ��s�ƁA�܂�Ԃ�����ێ����Ă���B
	*/
	int		m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
	int		m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
	int		m_nSelectLineBgnTo;		/* �͈͑I���J�n�s(���_) */
	int		m_nSelectColmBgnTo;		/* �͈͑I���J�n��(���_) */
	int		m_nSelectLineFrom;		/* �͈͑I���J�n�s */
	int		m_nSelectColmFrom;		/* �͈͑I���J�n�� */
	int		m_nSelectLineTo;		/* �͈͑I���I���s */
	int		m_nSelectColmTo;		/* �͈͑I���I���� */
	/* DrawSelectArea()�Ɍ��݂̑I��͈͂������č����̂ݕ`�悷�邽�߂̂���
	   ���݂̑I��͈͂�Old�փR�s�[������ŐV�����I��͈͂�Select�ɐݒ肵��
	   DrawSelectArea()���Ăт������ƂŐV�����͈͂��`�����D
	*/
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
	BOOL		m_bHokan;			//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH
	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump����
	CRegexKeyword	*m_cRegexKeyword;	//@@@ 2001.11.17 add MIK

	/*
	||  �����w���p�֐�
	*/
	void GetCurrentTextForSearch( CMemory& );			/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	void DestroyCaret( void ){	/* �L�����b�g��j������i�����I�ɂ��j���j*/
		::DestroyCaret();
		m_nCaretWidth = 0;
	}
protected:
//	CEOL GetCurrentInsertEOL( void );					/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */

	BOOL MyGetClipboardData( CMemory&, BOOL* );			/* �N���b�v�{�[�h����f�[�^���擾 */
	BOOL MySetClipboardData( const char*, int, BOOL );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	int GetLeftWord( CMemory*, int );					/* �J�[�\�����O�̒P����擾 */
	BOOL GetCurrentWord( CMemory* );					/* �L�����b�g�ʒu�̒P����擾 */	// 2006.03.24 fon
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
	void DrawRulerCaret( HDC hdc );								// ���[���[�̃L�����b�g�̂ݕ`�� 2002.02.25 Add By KK
	int	DispText( HDC, int, int, const char*, int );	/* �e�L�X�g�\�� */	//@@@ 2002.09.22 YAZAKI
	void DispTextSelected( HDC, int, int, int, int );			/* �e�L�X�g���] */
//	int	DispCtrlCode( HDC, int, int, const unsigned char*, int );	/* �R���g���[���R�[�h�\�� */
	void TwoPointToRect( RECT*, int, int, int, int );			/* 2�_��Ίp�Ƃ����`�����߂� */
	void DrawSelectArea( void );								/* �w��s�̑I��̈�̕`�� */
	void DrawSelectAreaLine( HDC, int, int, int, int, int );	/* �w��s�̑I��̈�̕`�� */
	int  LineColmnToIndex( const CLayout* pcLayout, int nColumn );		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColmnToIndex( const CDocLine* pcDocLine, int nColumn );		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColmnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen );	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */		// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColmn( const CLayout* pcLayout, int nIndex );		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */	// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColmn( const CDocLine* pcLayout, int nIndex );		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */	// @@@ 2002.09.28 YAZAKI
	void CopySelectedAllLines( const char*, BOOL );				/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	void ConvSelectedArea( int );								/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	void ConvMemory( CMemory*, int );							/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
	void OnSize( int, int );									/* �E�B���h�E�T�C�Y�̕ύX���� */
	int  OnVScroll( int, int );								/* �����X�N���[���o�[���b�Z�[�W���� */
	int  OnHScroll( int, int );								/* �����X�N���[���o�[���b�Z�[�W���� */
	void OnLBUTTONDOWN( WPARAM, int, int );						/* �}�E�X���{�^������ */
	void OnMOUSEMOVE( WPARAM, int, int );						/* �}�E�X�ړ��̃��b�Z�[�W���� */
	void OnLBUTTONUP( WPARAM, int, int );						/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
	void OnLBUTTONDBLCLK( WPARAM, int , int );					/* �}�E�X���{�^���_�u���N���b�N */
	void OnRBUTTONDOWN( WPARAM, int, int );						/* �}�E�X�E�{�^������ */
	void OnRBUTTONUP( WPARAM, int, int );						/* �}�E�X�E�{�^���J�� */
// novice 2004/10/11 �}�E�X���{�^���Ή�
	void OnMBUTTONDOWN( WPARAM, int, int );						/* �}�E�X���{�^������ */
// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
	void OnXLBUTTONDOWN( WPARAM, int, int );					/* �}�E�X���T�C�h�{�^������ */
	void OnXRBUTTONDOWN( WPARAM, int, int );					/* �}�E�X�E�T�C�h�{�^������ */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );						/* �}�E�X�z�C�[���̃��b�Z�[�W���� */
	/*! �I��͈͂��w�肷��(���_���I��)

		@date 2005.06.24 Moca
	*/
	void SetSelectArea( int nLineFrom, int nColmFrom, int nLineTo, int nColmTo ){
		m_nSelectLineBgnFrom = nLineFrom;
		m_nSelectColmBgnFrom = nColmFrom;
		m_nSelectLineBgnTo = nLineFrom;
		m_nSelectColmBgnTo = nColmFrom;
		m_nSelectLineFrom = nLineFrom;
		m_nSelectColmFrom = nColmFrom;
		m_nSelectLineTo = nLineTo;
		m_nSelectColmTo = nColmTo;
	}
	void BeginSelectArea( void );								/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	void ChangeSelectAreaByCurrentCursor( int, int );			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
	void ChangeSelectAreaByCurrentCursorTEST( int, int, int&, int&, int&, int& );/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
	void PrintSelectionInfoMsg(void);		// �I�����f�[�^�̍쐬	2005.07.09 genta
	int  MoveCursorToPoint( int, int );							/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ� */
	int  ScrollAtV( int );										/* �w���[�s�ʒu�փX�N���[�� */
	int  ScrollAtH( int );										/* �w�荶�[���ʒu�փX�N���[�� */
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	int  ScrollByV( int vl ){	return ScrollAtV( m_nViewTopLine + vl );}			/* �w��s�X�N���[��*/
	int  ScrollByH( int hl ){	return ScrollAtH( m_nViewLeftCol + hl );}					/* �w�茅�X�N���[�� */
	void SyncScrollV( int );									/* ���������X�N���[�� */
	void SyncScrollH( int );									/* ���������X�N���[�� */
	//	To Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	int Cursor_UPDOWN( int, int );								/* �J�[�\���㉺�ړ����� */
	void DrawBracketPair( bool );								/* �Ί��ʂ̋����\�� 02/09/18 ai */
	void SetBracketPairPos( bool );								/* �Ί��ʂ̋����\���ʒu�ݒ� 03/02/18 ai */
	int GetColorIndex( HDC, const CLayout*, int );				/* �w��ʒu��ColorIndex�̎擾 02/12/13 ai */
	bool IsBracket( const char*, int, int );					/* ���ʔ��� 03/01/09 ai */
public:
	void SetIMECompFormPos( void );								/* IME�ҏW�G���A�̈ʒu��ύX */
protected:
	void SetIMECompFormFont( void );							/* IME�ҏW�G���A�̕\���t�H���g��ύX */
	void SetParentCaption( BOOL = FALSE );						/* �e�E�B���h�E�̃^�C�g�����X�V */
public:
	void DrawCaretPosInfo( void );								/* �L�����b�g�̍s���ʒu��\������ */
protected:
	BOOL IsCurrentPositionURL( int, int, int*, int*, int*, char** );/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	int IsCurrentPositionSelected( int, int );					/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	int IsCurrentPositionSelectedTEST( int, int, int, int, int, int );/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	BOOL IsSearchString( const char*, int, int, int*, int* );	/* ���݈ʒu������������ɊY�����邩 */	//2002.02.08 hor �����ǉ�
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine );		/* �t�H���g��I�� */
	void ExecCmd(const char*, BOOL ) ;							// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	void AddToCmdArr( const char* );
	BOOL ChangeCurRegexp(void);									// 2002.01.16 hor ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�)
	void SendStatusMessage( const char* msg );					// 2002.01.26 hor �����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������
	void SendStatusMessage2( const char* msg );					// Jul. 9, 2005 genta
//  �ȉ��̓�͂���Ȃ��Ȃ�܂����B minfu 2002.04.10
//	LRESULT RequestedReconversion( PRECONVERTSTRING pReconv);	/*  IME����̍ĕϊ��v���ɓ����� minfu 2002.03.27 */
//	LRESULT RequestedReconversionW( PRECONVERTSTRING pReconv);	/*  IME����̍ĕϊ��v���ɓ����� for 95/NT 20020331 aroka */
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* �ĕϊ��p�\���̂�ݒ肷�� 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(PRECONVERTSTRING pReconv, bool bUnicode);				/* �ĕϊ��p�\���̂̏������ɑI��͈͂�ύX���� 2002.04.09 minfu */

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
	/* �t�@�C�����J�� */
	// Oct. 2, 2001 genta �}�N���p�ɋ@�\�g��
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILEOPEN( const char *filename = NULL,
		int nCharCode = CODE_AUTODETECT, BOOL bReadOnly = FALSE );
	
	/* �㏑���ۑ� */ // Feb. 28, 2004 genta �����ǉ�, Jan. 24, 2005 genta �����ǉ�
	BOOL Command_FILESAVE( bool warnbeep = true, bool askname = true );	
	BOOL Command_FILESAVEAS_DIALOG();			/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEAS( const char *filename );			/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEALL( void );	/* �S�ď㏑���ۑ� */ // Jan. 23, 2005 genta
	void Command_FILECLOSE( void );				/* �J����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	/* ���ĊJ��*/
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILECLOSE_OPEN( const char *filename = NULL,
		int nCharCode = CODE_AUTODETECT, BOOL bReadOnly = FALSE );
	
	void Command_FILE_REOPEN( int, int );			/* �ăI�[�v�� */	//Dec. 4, 2002 genta �����ǉ�
#if 0
	2002/04/19 YAZAKI
	void Command_FILE_REOPEN_SJIS( void );		/* SJIS�ŊJ������ */
	void Command_FILE_REOPEN_JIS( void );		/* JIS�ŊJ������ */
	void Command_FILE_REOPEN_EUC( void );		/* EUC�ŊJ������ */
	void Command_FILE_REOPEN_UNICODE( void );	/* Unicode�ŊJ������ */
	void Command_FILE_REOPEN_UTF8( void );		/* UTF-8�ŊJ������ */
	void Command_FILE_REOPEN_UTF7( void );		/* UTF-7�ŊJ������ */
#endif
	void Command_PRINT( void );					/* ���*/
	void Command_PRINT_PREVIEW( void );			/* ����v���r���[*/
	void Command_PRINT_PAGESETUP( void );		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	BOOL Command_OPEN_HfromtoC( BOOL );			/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
	BOOL Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	BOOL Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plus�Ŏ��s */
	void Command_BROWSE( void );				/* �u���E�Y */
	void Command_READONLY( void );				/* �ǂݎ���p */
	void Command_PROPERTY_FILE( void );			/* �t�@�C���̃v���p�e�B */
	void Command_EXITALL( void );				/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	void Command_CHAR( char );				/* �������� */
	void Command_IME_CHAR( WORD );			/* �S�p�������� */
	void Command_UNDO( void );				/* ���ɖ߂�(Undo) */
	void Command_REDO( void );				/* ��蒼��(Redo) */
	void Command_DELETE( void );			/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	void Command_DELETE_BACK( void );		/* �J�[�\���O���폜 */
	void Command_WordDeleteToStart( void );	/* �P��̍��[�܂ō폜 */
	void Command_WordDeleteToEnd( void );	/* �P��̉E�[�܂ō폜 */
	void Command_WordCut( void );			/* �P��؂��� */
	void Command_WordDelete( void );		/* �P��폜 */
	void Command_LineCutToStart( void );	//�s���܂Ő؂���(���s�P��)
	void Command_LineCutToEnd( void );		//�s���܂Ő؂���(���s�P��)
	void Command_LineDeleteToStart( void );	/* �s���܂ō폜(���s�P��) */
	void Command_LineDeleteToEnd( void );  	//�s���܂ō폜(���s�P��)
	void Command_CUT_LINE( void );			/* �s�؂���(�܂�Ԃ��P��) */
	void Command_DELETE_LINE( void );		/* �s�폜(�܂�Ԃ��P��) */
	void Command_DUPLICATELINE( void );		/* �s�̓�d��(�܂�Ԃ��P��) */
	void Command_INDENT( char cChar );		/* �C���f���g ver 1 */
// From Here 2001.12.03 hor
//	void Command_INDENT( const char*, int );/* �C���f���g ver0 */
	void Command_INDENT( const char*, int ,BOOL=FALSE );/* �C���f���g ver0 */
	void Command_INDENT_TAB( void );		/* �C���f���g ver 2 */
// To Here 2001.12.03 hor
	void Command_UNINDENT( char cChar );	/* �t�C���f���g */
//	void Command_WORDSREFERENCE( void );	/* �P�ꃊ�t�@�����X */
	void Command_TRIM(BOOL);				// 2001.12.03 hor
	void Command_TRIM2(CMemory*,BOOL);		// 2001.12.03 hor
	void Command_SORT(BOOL);				// 2001.12.06 hor
	void Command_MERGE(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
	void Command_CtrlCode_Dialog(void);		/* �R���g���[���R�[�h�̓���(�_�C�A���O) */	//@@@ 2002.06.02 MIK

	/* �w��ʒu�̎w�蒷�f�[�^�폜 */
	void CEditView::DeleteData2(
		int			nCaretX,
		int			nCaretY,
		int			nDelLen,
		CMemory*	pcMem,
		COpe*		pcOpe		/* �ҏW����v�f COpe */
//		BOOL		bRedraw,
//		BOOL		bRedraw2
	);
	void DeleteData( BOOL bRedraw );/* ���݈ʒu�̃f�[�^�폜 */
	/* ���݈ʒu�Ƀf�[�^��}�� */
	void InsertData_CEditView(
		int			nX,
		int			nY,
		const char*	pData,
		int			nDataLen,
		int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		int*		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		COpe*		pcOpe,				/* �ҏW����v�f COpe */
		BOOL		bRedraw
	);
	void SmartIndent_CPP( char );	/* C/C++�X�}�[�g�C���f���g���� */
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */

	/* �J�[�\���ړ��n */
	//	Oct. 24, 2001 genta �@�\�g���̂��߈����ǉ�
	int Command_UP( int, BOOL, int line = 0 );			/* �J�[�\����ړ� */
	int Command_DOWN( int, BOOL );			/* �J�[�\�����ړ� */
	int  Command_LEFT( int, BOOL );			/* �J�[�\�����ړ� */
	void Command_RIGHT( int, int, BOOL );	/* �J�[�\���E�ړ� */
	void Command_UP2( int );				/* �J�[�\����ړ��i�Q�s�Âj */
	void Command_DOWN2( int );				/* �J�[�\�����ړ��i�Q�s�Âj */
	void Command_WORDLEFT( int );			/* �P��̍��[�Ɉړ� */
	void Command_WORDRIGHT( int );			/* �P��̉E�[�Ɉړ� */
	//	Oct. 29, 2001 genta �}�N�������@�\�g��
	void Command_GOLINETOP( int bSelect, int lparam );	/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
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
	void Command_JUMPHIST_PREV(void);		// �ړ�����: �O��
	void Command_JUMPHIST_NEXT(void);		// �ړ�����: ����
	void Command_JUMPHIST_SET(void);		// ���݈ʒu���ړ������ɓo�^
	void Command_WndScrollDown(void);		// �e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// �e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	void Command_GONEXTPARAGRAPH( int bSelect );	// ���̒i���֐i��
	void Command_GOPREVPARAGRAPH( int bSelect );	// �O�̒i���֖߂�

	/* �I���n */
	bool Command_SELECTWORD( void );		/* ���݈ʒu�̒P��I�� */
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
	void Command_COPY( int, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN );/* �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[) */
	void Command_PASTE( void );						/* �\��t���i�N���b�v�{�[�h����\��t���j*/
	void Command_PASTEBOX( void );					/* ��`�\��t���i�N���b�v�{�[�h�����`�\��t���j*/
	//<< 2002/03/29 Azumaiya
	/* ��`�\��t���i�����n���ł̒���t���j*/
	void Command_PASTEBOX( const char *szPaste, int nPasteSize );
	//>> 2002/03/29 Azumaiya
//	void Command_INSTEXT( BOOL, const char*, int );	/* �e�L�X�g��\��t�� ver0 */
	//void Command_INSTEXT( BOOL, const char*, BOOL );/* �e�L�X�g��\��t�� ver1 */
	void Command_INSTEXT( BOOL, const char*, int, BOOL ); // 2004.05.14 Moca �e�L�X�g��\��t�� '\0'�Ή�
	void Command_ADDTAIL( const char*, int );		/* �Ō�Ƀe�L�X�g��ǉ� */
	void Command_COPYFILENAME( void );				/* ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ */ //2002/2/3 aroka
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
//	int			nX,
//	int			nY,
//	const char*	pData,
//	int			nDataLen,
//	int*		pnNewLine,	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int*		pnNewPos,	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//	COpe*		pcOpe,		/* �ҏW����v�f COpe */
	BOOL		bRedraw
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
	void Command_TOHANKATA( void );				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	void Command_TOZENEI( void );				/* ���p�p�����S�p�p�� */ //July. 30, 2001 Misaka
	void Command_TOHANEI( void );				/* �S�p�p�������p�p�� */ //@@@ 2002.2.11 YAZAKI
	void Command_HANKATATOZENKAKUKATA( void );	/* ���p�J�^�J�i���S�p�J�^�J�i */
	void Command_HANKATATOZENKAKUHIRA( void );	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	void Command_TABTOSPACE( void );			/* TAB���� */
	void Command_SPACETOTAB( void );			/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* �������ʁ�SJIS�R�[�h�ϊ� */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODEBE2SJIS( void );	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS��JIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS��EUC�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS��UTF-8�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS��UTF-7�R�[�h�ϊ� */
	void Command_BASE64DECODE( void );			/* Base64�f�R�[�h���ĕۑ� */
	void Command_UUDECODE( void );				/* uudecode���ĕۑ� */	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
	void Command_SEARCH_BOX( void );					/* ����(�{�b�N�X) */	// 2006.06.04 yukihane
	void Command_SEARCH_DIALOG( void );					/* ����(�P�ꌟ���_�C�A���O) */
	void Command_SEARCH_NEXT( bool, BOOL, HWND, const char* );/* �������� */
	void Command_SEARCH_PREV( BOOL, HWND );				/* �O������ */
	void Command_REPLACE_DIALOG( void );				/* �u��(�u���_�C�A���O) */
	void Command_REPLACE( HWND hwndParent );			/* �u��(���s) 2002/04/08 YAZAKI �e�E�B���h�E���w�肷��悤�ɕύX */
	enum REPLACE_ALL_MODE { REP_NORMAL = 0, REP_LINE = 1 };
	void Command_REPLACE_ALL( int nMode = REP_NORMAL );	/* ���ׂĒu��(���s) */
	void Command_SEARCH_CLEARMARK( void );				/* �����}�[�N�̃N���A */
	void Command_JUMP_SRCHSTARTPOS( void );				/* �����J�n�ʒu�֖߂� */	// 02/06/26 ai

	//	Jan. 10, 2005 genta HandleCommand����grep�֘A�����𕪗�
	void TranslateCommand_grep( int&, BOOL&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	void Command_GREP_DIALOG( void );					/* Grep�_�C�A���O�̕\�� */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP_DIALOG( void );					/* �w��s�w�W�����v�_�C�A���O�̕\�� */
	void Command_JUMP( void );							/* �w��s�w�W�����v */
// From Here 2001.12.03 hor
//	BOOL Command_FUNCLIST( BOOL );						/* �A�E�g���C����� */
//	BOOL Command_FUNCLIST( BOOL ,int=OUTLINE_DEFAULT );	/* �A�E�g���C����� */
	BOOL Command_FUNCLIST( int ,int=OUTLINE_DEFAULT );	/* �A�E�g���C����� */ // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta �����ǉ�
	bool Command_TAGJUMP( bool bClose = false );		/* �^�O�W�����v�@�\ */
	void Command_TAGJUMPBACK( void );					/* �^�O�W�����v�o�b�N�@�\ */
	bool Command_TagJumpByTagsFile( void );				//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.13 MIK
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose�ǉ�
	bool TagJumpSub( const char *pszJumpToFile, int nLine, int nColumn, bool bClose = false );
	bool Command_TagsMake( void );						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword( const char* keyword );	//	@@ 2005.03.31 MIK
	void Command_COMPARE( void );						/* �t�@�C�����e��r */
	void Command_Diff_Dialog( void );					/* DIFF�����\���_�C�A���O */	//@@@ 2002.05.25 MIK
	void Command_Diff( const char*, int );	/* DIFF�����\�� */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next( void );						/* ���̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev( void );						/* �O�̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset( void );					/* �����̑S���� */	//@@@ 2002.05.25 MIK
	void Command_BRACKETPAIR( void );					/* �Ί��ʂ̌��� */
// From Here 2001.12.03 hor
	void Command_BOOKMARK_SET( void );					/* �u�b�N�}�[�N�ݒ�E���� */
	void Command_BOOKMARK_NEXT( void );					/* ���̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_PREV( void );					/* �O�̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_RESET( void );				/* �u�b�N�}�[�N�̑S���� */
// To Here 2001.12.03 hor
	void Command_BOOKMARK_PATTERN( void );				// 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N

//2004.10.13 �C���N�������^���T�[�`�֌W
	void TranslateCommand_isearch( int&, BOOL&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );

	/* ���[�h�؂�ւ��n */
	void Command_CHGMOD_INS( void );	/* �}���^�㏑�����[�h�؂�ւ� */
	void Command_CHGMOD_EOL( enumEOLType );	/* ���͂�����s�R�[�h��ݒ� 2003.06.23 moca */
	void Command_CANCEL_MODE( void );	/* �e�탂�[�h�̎����� */

	/* �ݒ�n */
	void Command_SHOWTOOLBAR( void );		/* �c�[���o�[�̕\��/��\�� */
	void Command_SHOWFUNCKEY( void );		/* �t�@���N�V�����L�[�̕\��/��\�� */
	void Command_SHOWTAB( void );			/* �^�u�̕\��/��\�� */	//@@@ 2003.06.10 MIK
	void Command_SHOWSTATUSBAR( void );		/* �X�e�[�^�X�o�[�̕\��/��\�� */
	void Command_TYPE_LIST( void );			/* �^�C�v�ʐݒ�ꗗ */
	void Command_OPTION_TYPE( void );		/* �^�C�v�ʐݒ� */
	void Command_OPTION( void );			/* ���ʐݒ� */
	void Command_FONT( void );				/* �t�H���g�ݒ� */
	void Command_WRAPWINDOWWIDTH( void );	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	void Command_Favorite( void );	//���C�ɓ���	//@@@ 2003.04.08 MIK
	void Command_SET_QUOTESTRING( const char* );	//	Jan. 29, 2005 genta ���p���̐ݒ�

	/* �}�N���n */
	void Command_RECKEYMACRO( void );	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	void Command_SAVEKEYMACRO( void );	/* �L�[�}�N���̕ۑ� */
	void Command_LOADKEYMACRO( void );	/* �L�[�}�N���̓ǂݍ��� */
	void Command_EXECKEYMACRO( void );	/* �L�[�}�N���̎��s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void Command_EXECCMMAND( void );	/* �O���R�}���h���s */
	//	Oct. 9, 2001 genta �}�N���Ή��̂��ߋ@�\�g��
	void Command_EXECCOMMAND_DIALOG( const char* cmd );	/* �O���R�}���h���s�_�C�A���O�\�� */
	void Command_EXECCOMMAND( const char* cmd );	/* �O���R�}���h���s */
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
	void Command_BIND_WINDOW( void );	/* �������ĕ\�� */	//2004.07.14 Kazika �V�K�ǉ�
	void Command_CASCADE( void );		/* �d�˂ĕ\�� */
	void Command_TILE_V( void );		/* �㉺�ɕ��ׂĕ\�� */
	void Command_TILE_H( void );		/* ���E�ɕ��ׂĕ\�� */
	void Command_MAXIMIZE_V( void );	/* �c�����ɍő剻 */
	void Command_MAXIMIZE_H( void );	/* �������ɍő剻 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* ���ׂčŏ��� */
	void Command_REDRAW( void );		/* �ĕ`�� */
	void Command_WIN_OUTPUT( void );	//�A�E�g�v�b�g�E�B���h�E�\��
	void Command_TRACEOUT( const char* outputstr , int );	//�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	void Command_WINTOPMOST( LPARAM );		// ��Ɏ�O�ɕ\�� 2004.09.21 Moca
	void Command_WINLIST( int nCommandFrom );		/* �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�

	/* �x�� */
	//	Jan. 10, 2005 genta HandleCommand����⊮�֘A�����𕪗�
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);
	void Command_ToggleKeySearch( void );	/* �L�����b�g�ʒu�̒P���������������@�\ON-OFF */	// 2006.03.24 fon

	void ShowHokanMgr( CMemory& cmemData, BOOL bAutoDecided );	//	�⊮�E�B���h�E��\������BCtrl+Space��A�����̓���/�폜���ɌĂяo����܂��B YAZAKI 2002/03/11
	void Command_HOKAN( void );			/* ���͕⊮ */
	void Command_HELP_CONTENTS( void );	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* �R�}���h�ꗗ */
	void Command_EXTHELP1( void );		/* �O���w���v�P */
	//	Jul. 5, 2002 genta
	void Command_EXTHTMLHELP( const char* helpfile = NULL, const char* kwd = NULL );	/* �O��HTML�w���v */
	void Command_ABOUT( void );			/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//@@@ 2002.01.14 YAZAKI �s�g�p�̂���
//	void Command_SENDMAIL( void );		/* ���[�����M */

	//	May 23, 2000 genta
	//	��ʕ`��⏕�֐�
//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
//	void DrawEOL(HDC hdc, HANDLE hPen, int nPosX, int nPosY, int nWidth, int nHeight,
//		CEOL cEol, int bBold );
	void DrawEOL(HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight,
		CEOL cEol, int bBold, COLORREF pColor );
	//�^�u���`��֐�	//@@@ 2003.03.26 MIK
	void DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor );
	//	EOF�`��֐�	//	2004.05.29 genta
	int  DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo );
	//!	�w�茅�c���`��֐�	// 2005.11.08 Moca
	void DispVerticalLines( HDC, int, int, int, int );

	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//���ݍs�𗚗��ɒǉ�����

	void AnalyzeDiffInfo( const char*, int );	/* DIFF���̉�� */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( char*, HWND );	/* DIFF�ꎞ�t�@�C���쐬 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const char*, const char*, int );		/* DIFF�����\�� */		//2005.10.29 maru
	
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const char* [], const char* [], int, int, const char* ); // �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐� // 2003.08.12 Moca

// 2002/07/22 novice
	void ShowCaret_( HWND hwnd );
	void HideCaret_( HWND hwnd );

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;
	UINT	m_uWM_MSIME_RECONVERTREQUEST;
	
	int		m_nLastReconvLine;             //2002.04.09 minfu �ĕϊ����ۑ��p;
	int		m_nLastReconvIndex;            //2002.04.09 minfu �ĕϊ����ۑ��p;

	//ATOK��p�ĕϊ���API
	HMODULE m_hAtokModule;
	BOOL (WINAPI *AT_ImmSetReconvertString)( HIMC , int ,PRECONVERTSTRING , DWORD  );
	
	bool	m_bUnderLineON;
	bool	m_bCaretShowFlag;

	/* �C���N�������^���T�[�` */ 
	//2004.10.24 isearch migemo
	CMigemo* m_pcmigemo;
	void ISearchEnter( int mode  ,int direction);
	void ISearchExit();
	void ISearchExec(WORD wChar);
	void ISearchExec(const char* pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CMemory* msg) const;
	char* m_pszMigemoWord;
	int m_nISearchDirection;
	int m_nISearchMode;
	bool m_bISearchWrap;
	int m_nISearchX1History[256];
	int m_nISearchY1History[256];
	int m_nISearchX2History[256];
	int m_nISearchY2History[256];
	bool m_bISearchFlagHistory[256];
	int m_nISearchHistoryCount;
	bool m_bISearchFirst;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
