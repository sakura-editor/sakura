/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/06/23 �V�K�쐬
	@date 1998/12/04 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, hor
	Copyright (C) 2002, aroka, hor, YAZAKI, frozen
	Copyright (C) 2003, little YOSHI
	Copyright (C) 2005, genta
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGFUNCLIST_H_
#define _CDLGFUNCLIST_H_

#include <Windows.h>
#include "dlg/CDialog.h"
#include "doc/CEditDoc.h"

class CFuncInfo;
class CFuncInfoArr; // 2002/2/10 aroka

#define OUTLINE_LAYOUT_FOREGROUND (0)
#define OUTLINE_LAYOUT_BACKGROUND (1)
#define OUTLINE_LAYOUT_FILECHANGED (2)

//!	�A�E�g���C����̓_�C�A���O�{�b�N�X
class CDlgFuncList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFuncList();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );	// 2007.11.07 ryoji �W���ȊO�̃��b�Z�[�W��ߑ�����
	HWND DoModeless( HINSTANCE, HWND, LPARAM, CFuncInfoArr*, CLayoutInt, CLayoutInt, int, int, bool );/* ���[�h���X�_�C�A���O�̕\�� */
	void ChangeView( LPARAM );	/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
	bool IsDocking() { return m_eDockSide > DOCKSIDE_FLOAT; }
	EDockSide GetDockSide() { return m_eDockSide; }

	CommonSetting_OutLine& CommonSet(void){ return m_pShareData->m_Common.m_sOutline; }
	STypeConfig& TypeSet(void){ return CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute(); }
	int& ProfDockSet() { return CommonSet().m_nOutlineDockSet; }
	BOOL& ProfDockSync() { return CommonSet().m_bOutlineDockSync; }
	BOOL& ProfDockDisp() { return (ProfDockSet() == 0)? CommonSet().m_bOutlineDockDisp: TypeSet().m_bOutlineDockDisp; }
	EDockSide& ProfDockSide() { return (ProfDockSet() == 0)? CommonSet().m_eOutlineDockSide: TypeSet().m_eOutlineDockSide; }
	int& ProfDockLeft() { return (ProfDockSet() == 0)? CommonSet().m_cxOutlineDockLeft: TypeSet().m_cxOutlineDockLeft; }
	int& ProfDockTop() { return (ProfDockSet() == 0)? CommonSet().m_cyOutlineDockTop: TypeSet().m_cyOutlineDockTop; }
	int& ProfDockRight() { return (ProfDockSet() == 0)? CommonSet().m_cxOutlineDockRight: TypeSet().m_cxOutlineDockRight; }
	int& ProfDockBottom() { return (ProfDockSet() == 0)? CommonSet().m_cyOutlineDockBottom: TypeSet().m_cyOutlineDockBottom; }

	/*! ���݂̎�ʂƓ����Ȃ�
	*/
	bool CheckListType( int nOutLineType ) const { return nOutLineType == m_nOutlineType; }
	void Redraw( int nOutLineType, int nListType, CFuncInfoArr*, CLayoutInt nCurLine, CLayoutInt nCurCol );
	void Refresh( void );
	bool ChangeLayout( int nId );
	void OnOutlineNotify( WPARAM wParam, LPARAM lParam );
	void SyncColor( void );
	void SetWindowText( const TCHAR* szTitle );		//�_�C�A���O�^�C�g���̐ݒ�
	bool m_bInChangeLayout;

	CFuncInfoArr*	m_pcFuncInfoArr;	/* �֐����z�� */
	CLayoutInt		m_nCurLine;			/* ���ݍs */
	CLayoutInt		m_nCurCol;			/* ���݌� */
	int				m_nSortCol;			/* �\�[�g�����ԍ� */
	int				m_nDocType;			/* �h�L�������g�̎�� */
	int				m_nOutlineType;		/* �A�E�g���C����͂̎�� */
	int				m_nListType;		/* �ꗗ�̎�� */
	CNativeW		m_cmemClipText;		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
	int				m_bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	bool			m_bEditWndReady;	/* �G�f�B�^��ʂ̏������� */
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM, LPARAM );
	BOOL OnSize( WPARAM wParam, LPARAM lParam );
	BOOL OnMinMaxInfo( LPARAM lParam );
	BOOL OnDestroy(void); // 20060201 aroka
	BOOL OnCbnSelChange( HWND hwndCtl, int wID ); // 2002/11/1 frozen
	BOOL OnContextMenu( WPARAM, LPARAM );
	void SetData();	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

	/*
	||  �����w���p�֐�
	*/
	BOOL OnJump( bool bCheckAutoClose = true );	//	bCheckAutoClose�F�u���̃_�C�A���O�������I�ɕ���v���`�F�b�N���邩�ǂ���
	void SetTreeCpp( HWND );	/* �c���[�R���g���[���̏������FC++���\�b�h�c���[ */
	void SetTreeJava( HWND, BOOL );	/* �c���[�R���g���[���̏������FJava���\�b�h�c���[ */
	void SetTree(bool tagjump = false);		/* �c���[�R���g���[���̏������F�ėp�i */
	void SetListVB( void );			/* ���X�g�r���[�R���g���[���̏������FVisualBasic */		// Jul 10, 2003  little YOSHI

	// 2002/11/1 frozen 
	void SortTree(HWND hWndTree,HTREEITEM htiParent);//!< �c���[�r���[�̍��ڂ��\�[�g����i�\�[�g���m_nSortType���g�p�j
#if 0
2002.04.01 YAZAKI SetTreeTxt()�ASetTreeTxtNest()�͔p�~�BGetTreeTextNext�͂��Ƃ��Ǝg�p����Ă��Ȃ������B
	void SetTreeTxt( HWND );	/* �c���[�R���g���[���̏������F�e�L�X�g�g�s�b�N�c���[ */
	int SetTreeTxtNest( HWND, HTREEITEM, int, int, HTREEITEM*, int );
	void GetTreeTextNext( HWND, HTREEITEM, int );
#endif

	//	Apr. 23, 2005 genta ���X�g�r���[�̃\�[�g���֐��Ƃ��ēƗ�������
	void SortListView(HWND hwndList, int sortcol);

	// 2001.12.03 hor
//	void SetTreeBookMark( HWND );		/* �c���[�R���g���[���̏������F�u�b�N�}�[�N */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void Key2Command( WORD );		//	�L�[���쁨�R�}���h�ϊ�
	bool HitTestSplitter( int xPos, int yPos );
	int HitTestCaptionButton( int xPos, int yPos );
	INT_PTR OnNcCalcSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcHitTest( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void GetDockSpaceRect( LPRECT pRect );
	void GetCaptionRect( LPRECT pRect );
	bool GetCaptionButtonRect( int nButton, LPRECT pRect );
	void DoMenu( POINT pt, HWND hwndFrom );
	BOOL PostOutlineNotifyToAllEditors( WPARAM wParam, LPARAM lParam );
	EDockSide GetDropRect( POINT ptDrag, POINT ptDrop, LPRECT pRect, bool bForceFloat );
	BOOL Track( POINT ptDrag );

private:
	//	May 18, 2001 genta
	/*!
		@brief �A�E�g���C����͎��

		0: List, 1: Tree
	*/
	int	m_nViewType;

	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 1/4
	// (������Ȃ̂łǂȂ����C�����肢���܂�)
	bool m_bWaitTreeProcess;

	// 2002/11/1 frozen
	//! �c���[�r���[���\�[�g����
	// 0 �f�t�H���g(�m�[�h�Ɋ֘A�Â����ꂽ�l��)
	// 1 �A���t�@�x�b�g��
	int m_nSortType;

	// �I�𒆂̊֐����
	CFuncInfo* m_cFuncInfo;

	EDockSide	m_eDockSide;	// ���݂̉�ʂ̕\���ʒu
	HWND		m_hwndToolTip;	/*!< �c�[���`�b�v�i�{�^���p�j */
	bool		m_bStretching;
	bool		m_bHovering;
	int			m_nHilightedBtn;
	int			m_nCapturingBtn;

	static LPDLGTEMPLATE m_pDlgTemplate;
	static DWORD m_dwDlgTmpSize;

	POINT				m_ptDefaultSize;
	POINT				m_ptDefaultSizeClient;
	RECT				m_rcItems[12];
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */



