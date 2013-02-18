/*!	@file
	@brief Dialog Box���N���X�w�b�_�t�@�C��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#ifndef _CDIALOG_H_
#define _CDIALOG_H_

class CDialog;

struct DLLSHAREDATA;

enum EAnchorStyle
{
	ANCHOR_NONE              = 0,
	ANCHOR_LEFT              = 1,
	ANCHOR_RIGHT             = 2,
	ANCHOR_LEFT_RIGHT        = 3,
	ANCHOR_TOP               = 4,
	ANCHOR_TOP_LEFT          = 5,
	ANCHOR_TOP_RIGHT         = 6,
	ANCHOR_TOP_LEFT_RIGHT    = 7,
	ANCHOR_BOTTOM            = 8,
	ANCHOR_BOTTOM_LEFT       = 9,
	ANCHOR_BOTTOM_RIGHT      = 10,
	ANCHOR_BOTTOM_LEFT_RIGHT = 11,
	ANCHOR_TOP_BOTTOM        = 12,
	ANCHOR_TOP_BOTTOM_LEFT   = 13,
	ANCHOR_TOP_BOTTOM_RIGHT  = 14,
	ANCHOR_ALL               = 15
};

struct SAnchorList
{
	int id;
	EAnchorStyle anchor;
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �_�C�A���O�E�B���h�E�������N���X

	�_�C�A���O�{�b�N�X�����Ƃ��ɂ͂�������p��������D

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CDialog
{
public:
	/*
	||  Constructors
	*/
	CDialog();
	virtual ~CDialog();
	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */
	INT_PTR DoModal( HINSTANCE, HWND, int, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, int, LPARAM, int );	/* ���[�h���X�_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, LPCDLGTEMPLATE, LPARAM, int );	/* ���[�h���X�_�C�A���O�̕\�� */
	void CloseDialog( int );

	virtual BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	virtual BOOL OnDestroy( void );
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam ){return FALSE;}
	BOOL OnSize();
	virtual BOOL OnSize( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnMove( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnDrawItem( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnTimer( WPARAM wParam ){return TRUE;}
	virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnDeviceChange( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual int GetData( void ){return 1;}/* �_�C�A���O�f�[�^�̎擾 */
	virtual void SetData( void ){return;}/* �_�C�A���O�f�[�^�̐ݒ� */
	virtual BOOL OnBnClicked( int );
	virtual BOOL OnStnClicked( int ){return FALSE;}
	virtual BOOL OnEnChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnLbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnLbnDblclk( int wID ){return FALSE;}
	virtual BOOL OnCbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnCbnEditChange( HWND hwndCtl, int wID ){return FALSE;} // @@2005.03.31 MIK �^�O�W�����vDialog
	virtual BOOL OnCbnDropDown( HWND hwndCtl, int wID );
//	virtual BOOL OnCbnCloseUp( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnCbnSelEndOk( HWND hwndCtl, int wID );

	virtual BOOL OnKillFocus( WPARAM wParam, LPARAM lParam ){return FALSE;}
	virtual BOOL OnActivate( WPARAM wParam, LPARAM lParam ){return FALSE;}	//@@@ 2003.04.08 MIK
	virtual int OnVKeyToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
	virtual LRESULT OnCharToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
	virtual BOOL OnPopupHelp( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void ResizeItem( HWND hTarget, const POINT& ptDlgDefalut, const POINT& ptDlgNew, const RECT& rcItemDefault, EAnchorStyle anchor, bool bUpdate = true);
	void GetItemClientRect( int wID, RECT& rc );
public:
	// �ݒ�t�H���_���΃t�@�C���I��(���L�f�[�^,ini�ʒu�ˑ�)
	static BOOL SelectFile(HWND parent, HWND hwndCtl, const TCHAR* filter, bool resolvePath);

public:
	HWND GetHwnd() const{ return m_hWnd; }
	//����C���^�[�t�F�[�X (�g�p�͍D�܂����Ȃ�)
	void _SetHwnd(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
private:
	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */
public:
	HWND			m_hwndSizeBox;
	LPARAM			m_lParam;
	BOOL			m_bModal;		/* ���[�_�� �_�C�A���O�� */
	int				m_nShowCmd;		//	�ő剻/�ŏ���
	int				m_nWidth;
	int				m_nHeight;
	int				m_xPos;
	int				m_yPos;
//	void*			m_pcEditView;
	LPCTSTR			m_pszHelpFile;
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bInited;

protected:
	void CreateSizeBox( void );
	BOOL OnCommand( WPARAM, LPARAM );

	HWND GetItemHwnd(int nID){ return ::GetDlgItem( GetHwnd(), nID ); }

	// �R���g���[���ɉ�ʂ̃t�H���g��ݒ�	2012/11/27 Uchi
	void SetMainFont( HWND hTarget );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDIALOG_H_ */



