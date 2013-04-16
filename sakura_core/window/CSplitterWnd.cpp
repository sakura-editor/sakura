/*!	@file
	@brief �������E�B���h�E�N���X

	@author Norio Nakatani
	@date 1998/07/07 �V�K�쐬
	@date 2002/2/3 aroka ���g�p�R�[�h����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CSplitterWnd.h"
#include "window/CSplitBoxWnd.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "env/DLLSHAREDATA.h"


//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CSplitterWnd::CSplitterWnd()
: CWnd(_T("::CSplitterWnd"))
, m_pszClassName(_T("SplitterWndClass"))	/* �N���X�� */
, m_nActivePane(0)					/* �A�N�e�B�u�ȃy�C�� 0-3 */
, m_nAllSplitRows(1)					/* �����s�� */
, m_nAllSplitCols(1)					/* �������� */
, m_nVSplitPos(0)					/* ���������ʒu */
, m_nHSplitPos(0)					/* ���������ʒu */
, m_bDragging(0)						/* �����o�[���h���b�O���� */
, m_nDragPosX(0)						/* �h���b�O�ʒu�w */
, m_nDragPosY(0)						/* �h���b�O�ʒu�x */
, m_nChildWndCount(0)
, m_pCEditWnd(NULL)
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = &GetDllShareData();

	m_hcurOld = NULL;						/* ���Ƃ̃}�E�X�J�[�\�� */

	for( int v=0; v < MAXCOUNTOFVIEW; v++ ){
		m_ChildWndArr[v] = NULL;				/* �q�E�B���h�E�z�� */
	}
	return;
}




CSplitterWnd::~CSplitterWnd()
{
}




/* ������ */
HWND CSplitterWnd::Create( HINSTANCE hInstance, HWND hwndParent, void* pCEditWnd )
{
	/* ������ */
	m_pCEditWnd	= pCEditWnd;

	/* �E�B���h�E�N���X�쐬 */
	ATOM atWork;
	atWork = RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		NULL,// Handle to the class cursor.
		(HBRUSH)NULL,// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated 
				//character string that specifies the resource name of the class menu,
				//as the name appears in the resource file.
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);
	if( 0 == atWork ){
		ErrorMessage( NULL, _T("SplitterWnd�N���X�̓o�^�Ɏ��s���܂����B") );
	}

	/* ���N���X�����o�Ăяo�� */
	return CWnd::Create(
		hwndParent,
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
}





/* �q�E�B���h�E�̐ݒ�
	@param hwndEditViewArr [in] HWND�z�� NULL�I�[
*/
void CSplitterWnd::SetChildWndArr( HWND* hwndEditViewArr )
{
	int v=0;
	for( ; v < MAXCOUNTOFVIEW && hwndEditViewArr[v]; v++ ){
		m_ChildWndArr[v] = hwndEditViewArr[v];				/* �q�E�B���h�E�z�� */
	}
	m_nChildWndCount = v;
	// �c���NULL�Ŗ��߂�
	for( ; v < MAXCOUNTOFVIEW; v++ ){
		m_ChildWndArr[v] = NULL;
	}

	// 2002/05/11 YAZAKI �s�v�ȏ����Ǝv����
	/* �E�B���h�E�̕��� */
//	DoSplit( m_nHSplitPos, m_nVSplitPos );
//	DoSplit( 0, 0 );
	return;
}




/* �����t���[���`�� */
void CSplitterWnd::DrawFrame( HDC hdc, RECT* prc )
{
	CSplitBoxWnd::Draw3dRect( hdc, prc->left, prc->top, prc->right, prc->bottom,
		::GetSysColor( COLOR_3DSHADOW ),
		::GetSysColor( COLOR_3DHILIGHT )
	);
	CSplitBoxWnd::Draw3dRect( hdc, prc->left + 1, prc->top + 1, prc->right - 2, prc->bottom - 2,
		RGB( 0, 0, 0 ),
		::GetSysColor( COLOR_3DFACE )
	);
	return;
}




/* �����g���b�J�[�̕\�� */
void CSplitterWnd::DrawSplitter( int xPos, int yPos, int bEraseOld )
{
	HDC			hdc;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	RECT		rc;
	RECT		rc2;
	int			nTrackerWidth = 6;

	hdc = ::GetDC( GetHwnd() );
	hBrush = ::CreateSolidBrush( RGB(255,255,255) );
	hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	::SetROP2( hdc, R2_XORPEN );
	::SetBkMode( hdc, TRANSPARENT );
	::GetClientRect( GetHwnd(), &rc );

	if( bEraseOld ){
		if( m_bDragging & 1 ){	/* �����o�[���h���b�O���� */
			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + nTrackerWidth;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
		if( m_bDragging & 2 ){	/* �����o�[���h���b�O���� */
			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + nTrackerWidth;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
	}

	m_nDragPosX = xPos;
	m_nDragPosY = yPos;
	if( m_bDragging & 1 ){	/* �����o�[���h���b�O���� */
		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + nTrackerWidth;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}
	if( m_bDragging & 2 ){	/* �����o�[���h���b�O���� */
		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + nTrackerWidth;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}

	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( GetHwnd(), hdc );
	return;
}




/* �����o�[�ւ̃q�b�g�e�X�g */
int CSplitterWnd::HitTestSplitter( int xPos, int yPos )
{
	int			nFrameWidth = 3;
	int			nMargin = 2;

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		return 0;
	}else
	if( m_nAllSplitRows == 2 && m_nAllSplitCols == 1 ){
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else{
			return 0;
		}
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 2 ){
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}else{
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin &&
			m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 3;
		}else
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}
}

/*! �E�B���h�E�̕���
	@param nHorizontal �����N���C�A���g���W 1�ȏ�ŕ��� 0:�������Ȃ�  -1: �O�̐ݒ��ێ�
	@param nVertical   �����N���C�A���g���W 1�ȏ�ŕ��� 0:�������Ȃ�  -1: �O�̐ݒ��ێ�
*/
void CSplitterWnd::DoSplit( int nHorizontal, int nVertical )
{
	int					nActivePane;
	int					nLimit = 32;
	RECT				rc;
	int					nAllSplitRowsOld = m_nAllSplitRows;	/* �����s�� */
	int					nAllSplitColsOld = m_nAllSplitCols;	/* �������� */
	CEditView*			pcViewArr[MAXCOUNTOFVIEW];
//	int					i;
	BOOL				bVUp;
	BOOL				bHUp;
	BOOL				bSizeBox;
	CEditWnd*			pCEditWnd = (CEditWnd*)m_pCEditWnd;
	bVUp = FALSE;
	bHUp = FALSE;

	if( -1 == nHorizontal && -1 == nVertical ){
		nVertical = m_nVSplitPos;		/* ���������ʒu */
		nHorizontal = m_nHSplitPos;		/* ���������ʒu */
	}

	if( 0 != nVertical || 0 != nHorizontal ){
		// �����w���B�܂����쐬�Ȃ�2�ڈȍ~�̃r���[���쐬���܂�
		// ���̂Ƃ���͕������Ɋ֌W�Ȃ�4�܂ň�x�ɍ��܂��B
		pCEditWnd->CreateEditViewBySplit(2*2);
	}
	/*
	|| �t�@���N�V�����L�[�����ɕ\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	|| �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	*/
	if( NULL == pCEditWnd
	 ||( NULL != pCEditWnd->m_CFuncKeyWnd.GetHwnd()
	  && 1 == m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	  )
	){
		bSizeBox = FALSE;
	}else{
		bSizeBox = TRUE;
		/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
		if( NULL != pCEditWnd->m_cStatusBar.GetStatusHwnd() ){
			bSizeBox = FALSE;
		}
	}
	/* ���C���E�B���h�E���ő剻����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
	WINDOWPLACEMENT	wp;
	wp.length = sizeof( wp );
	::GetWindowPlacement( GetParentHwnd(), &wp );
	if( SW_SHOWMAXIMIZED == wp.showCmd ){
		bSizeBox = FALSE;
	}

	int v;
	for( v=0; v < m_nChildWndCount; v++ ){
		pcViewArr[v] = ( CEditView* )::GetWindowLongPtr( m_ChildWndArr[v], 0 );
	}
	::GetClientRect( GetHwnd(), &rc );
	if( nHorizontal < nLimit ){
		if( nHorizontal > 0 ){
			bHUp = TRUE;
		}
		nHorizontal = 0;
	}
	if( nHorizontal > rc.right - nLimit * 2 ){
		nHorizontal = 0;
	}
	if( nVertical < nLimit ){
		if( nVertical > 0 ){
			bVUp = TRUE;
		}
		nVertical = 0;
	}
	if( nVertical > rc.bottom - nLimit * 2 ){
		nVertical = 0;
	}
	m_nVSplitPos = nVertical;		/* ���������ʒu */
	m_nHSplitPos = nHorizontal;		/* ���������ʒu */

	if( nVertical == 0 && nHorizontal == 0 ){
		m_nAllSplitRows = 1;	/* �����s�� */
		m_nAllSplitCols = 1;	/* �������� */
		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_HIDE );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_HIDE );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );

		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( TRUE, TRUE, bSizeBox );		/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
			if( bVUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
			if( bHUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}else{
			if( !bVUp && !bHUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}else
			if( bVUp && !bHUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
			}else
			if( !bVUp && bHUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[0] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}
		nActivePane = 0;
	}else
	if( nVertical > 0 &&  nHorizontal == 0 ){
		m_nAllSplitRows = 2;	/* �����s�� */
		m_nAllSplitCols = 1;	/* �������� */

		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_HIDE );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_SHOW );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );
		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, TRUE, bSizeBox );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* �㉺�ɕ��������Ƃ� */
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			// YAZAKI
			pcViewArr[2]->GetTextArea().SetViewTopLine( pcViewArr[0]->GetTextArea().GetViewTopLine() + pcViewArr[0]->GetTextArea().m_nViewRowNum );
		}
		else if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
		}
		else if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
		}
		else{
			if( bHUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[2] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[2] );
				}
			}else{
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( m_nActivePane != 0 &&
					m_nActivePane != 2 &&
					NULL != pcViewArr[0] &&
					NULL != pcViewArr[1] &&
					NULL != pcViewArr[2] &&
					NULL != pcViewArr[3]
				){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
					pcViewArr[3]->CopyViewStatus( pcViewArr[2] );
				}
			}
		}
		if( m_nActivePane == 0 || m_nActivePane == 1 ){
			// 2007.10.01 ryoji
			// ������������̐ؑ֎��̂ݏ]���R�[�h�����s���ăA�N�e�B�u�y�C�������߂�B
			// ����ȊO�̏ꍇ�̓y�C��0���A�N�e�B�u�ɂ���B
			// �]���́A�㉺�ɕ������Ă����āA
			// �E�㉺�����o�[�𓮂���
			// �E�X�e�[�^�X�o�[�ȂǊe��o�[�̕\���^��\����؂�ւ���
			// �E�ݒ��ʂ�OK�ŕ���
			// �E���E���������č��E��������������
			// �Ƃ�������������邾���ŉ��̃y�C�����A�N�e�B�u������邱�Ƃ��������B
			// �i�V���v����0�Œ�ɂ��Ă��܂��Ă��ǂ��C�͂��邯��ǁD�D�D�j
			nActivePane = 0;
			if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
				if ( pcViewArr[2]->GetTextArea().GetViewTopLine() < pcViewArr[2]->GetCaret().GetCaretLayoutPos().y ){
					nActivePane = 2;
				}
				else{
					nActivePane = 0;
				}
			}
		}
		else{
			nActivePane = 2;
		}
	}
	else if( nVertical == 0 &&  nHorizontal > 0 ){
		m_nAllSplitRows = 1;	/* �����s�� */
		m_nAllSplitCols = 2;	/* �������� */

		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_SHOW );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_HIDE );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );
		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( TRUE, FALSE, bSizeBox );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
		}else{
			if( bVUp ){
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[1] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[1] );
				}
			}else{
				/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
				if( m_nActivePane != 0 &&
					m_nActivePane != 1 &&
					NULL != pcViewArr[0] &&
					NULL != pcViewArr[1] &&
					NULL != pcViewArr[2] &&
					NULL != pcViewArr[3]
				){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
					pcViewArr[3]->CopyViewStatus( pcViewArr[1] );
				}
			}
		}
		if( m_nActivePane == 0 || m_nActivePane == 2 ){
			nActivePane = 0;
		}else{
			nActivePane = 1;
		}
	}else{
		m_nAllSplitRows = 2;	/* �����s�� */
		m_nAllSplitCols = 2;	/* �������� */
		if( m_ChildWndArr[0] != NULL ){ ::ShowWindow( m_ChildWndArr[0], SW_SHOW );}
		if( m_ChildWndArr[1] != NULL ){ ::ShowWindow( m_ChildWndArr[1], SW_SHOW );}
		if( m_ChildWndArr[2] != NULL ){ ::ShowWindow( m_ChildWndArr[2], SW_SHOW );}
		if( m_ChildWndArr[3] != NULL ){ ::ShowWindow( m_ChildWndArr[3], SW_SHOW );}
		if( NULL != pcViewArr[0] ){ pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		if( NULL != pcViewArr[1] ){ pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		if( NULL != pcViewArr[2] ){ pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		if( NULL != pcViewArr[3] ){ pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, bSizeBox );}	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[3] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[3] );
			}
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[2] && NULL != pcViewArr[3] ){
				pcViewArr[2]->CopyViewStatus( pcViewArr[3] );
			}
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			/* �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
			if( NULL != pcViewArr[1] && NULL != pcViewArr[3] ){
				pcViewArr[1]->CopyViewStatus( pcViewArr[3] );
			}
		}else{
		}
		nActivePane = m_nActivePane;
	}
	OnSize( 0, 0, 0, 0 );

	/* �A�N�e�B�u�ɂȂ������Ƃ��y�C���ɒʒm */
	if( m_ChildWndArr[nActivePane] != NULL ){
		::PostMessageAny( m_ChildWndArr[nActivePane], MYWM_SETACTIVEPANE, 0, 0 );
	}

	return;
}

/* �A�N�e�B�u�y�C���̐ݒ� */
void CSplitterWnd::SetActivePane( int nIndex )
{
	assert( nIndex < MAXCOUNTOFVIEW );
	m_nActivePane = nIndex;
	return;
}


/* �c�����n�m�^�n�e�e */
void CSplitterWnd::VSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( m_nHSplitPos, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, 0 );
	}else{
		DoSplit( m_nHSplitPos, 0 );
	}
	return;
}



/* �������n�m�^�n�e�e */
void CSplitterWnd::HSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2, 0 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( 0, 0 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2 , m_nVSplitPos );
	}else{
		DoSplit( 0, m_nVSplitPos );
	}
	return;
}



/* �c�������n�m�^�n�e�e */
void CSplitterWnd::VHSplitOnOff( void )
{
	int		nX;
	int		nY;
	RECT	rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows > 1 && m_nAllSplitCols > 1 ){
		nX = 0;
		nY = 0;
	}else{
		if( m_nAllSplitRows == 1){
			nY = rc.bottom / 2;
		}else{
			nY = m_nVSplitPos;
		}
		if( m_nAllSplitCols == 1 ){
			nX = rc.right / 2;
		}else{
			nX = m_nHSplitPos;
		}
	}
	DoSplit( nX, nY );

	return;
}


/* �O�̃y�C����Ԃ� */
int CSplitterWnd::GetPrevPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 2:
			nPane = 0;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		case 2:
			nPane = 1;
			break;
		case 3:
			nPane = 2;
			break;
		}
	}
	return nPane;
}




/* ���̃y�C����Ԃ� */
int CSplitterWnd::GetNextPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 2;
			break;
		case 2:
			nPane = -1;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = -1;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = 2;
			break;
		case 2:
			nPane = 3;
			break;
		case 3:
			nPane = -1;
			break;
		}
	}
	return nPane;
}


/* �ŏ��̃y�C����Ԃ� */
int CSplitterWnd::GetFirstPane( void )
{
	return 0;
}



/* �Ō�̃y�C����Ԃ� */
int CSplitterWnd::GetLastPane( void )
{
	int		nPane;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = 0;
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		nPane = 1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		nPane = 2;
	}else{
		nPane = 3;
	}
	return nPane;
}




/* �`�揈�� */
LRESULT CSplitterWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	RECT		rcFrame;
	int			nFrameWidth = 3;
	HBRUSH		hBrush;
	hdc = ::BeginPaint( hwnd, &ps );
	::GetClientRect( GetHwnd(), &rc );
	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
	if( m_nAllSplitRows > 1 ){
		::SetRect( &rcFrame, rc.left, m_nVSplitPos, rc.right, m_nVSplitPos + nFrameWidth );
		::FillRect( hdc, &rcFrame, hBrush );
	}
	if( m_nAllSplitCols > 1 ){
		::SetRect( &rcFrame, m_nHSplitPos, rc.top, m_nHSplitPos + nFrameWidth, rc.bottom );
		::FillRect( hdc, &rcFrame, hBrush );
	}
	::DeleteObject( hBrush );
	::EndPaint(hwnd, &ps);
	return 0L;
}




/* �E�B���h�E�T�C�Y�̕ύX���� */
LRESULT CSplitterWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CEditWnd*	pCEditWnd = (CEditWnd*)m_pCEditWnd;
	CEditView*	pcViewArr[MAXCOUNTOFVIEW];
	int					i;
	RECT		rcClient;
	int			nFrameWidth = 3;
	BOOL		bSizeBox;
	for( i = 0; i < m_nChildWndCount; ++i ){
		pcViewArr[i] = ( CEditView* )::GetWindowLongPtr( m_ChildWndArr[i], 0 );
	}

	/*
	|| �t�@���N�V�����L�[�����ɕ\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	|| �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	*/
	if( NULL == pCEditWnd
	 ||( NULL != pCEditWnd->m_CFuncKeyWnd.GetHwnd()
	  && 1 == m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	  )
	){
		bSizeBox = FALSE;
	}else{
		bSizeBox = TRUE;
		/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
		if( NULL != pCEditWnd->m_cStatusBar.GetStatusHwnd() ){
			bSizeBox = FALSE;
		}
	}
	/* ���C���E�B���h�E���ő剻����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
	WINDOWPLACEMENT	wp;
	wp.length = sizeof( wp );
	::GetWindowPlacement( GetParentHwnd(), &wp );
	if( SW_SHOWMAXIMIZED == wp.showCmd ){
		bSizeBox = FALSE;
	}

	::GetClientRect( GetHwnd(), &rcClient );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, rcClient.right,  rcClient.bottom, TRUE );		/* �q�E�B���h�E�z�� */

			pcViewArr[0]->SplitBoxOnOff( TRUE, TRUE, bSizeBox );		/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
	}else
	if( m_nAllSplitRows == 2 && m_nAllSplitCols == 1 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, rcClient.right,  m_nVSplitPos, TRUE );		/* �q�E�B���h�E�z�� */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
		if( m_ChildWndArr[2] != NULL ){
			::MoveWindow( m_ChildWndArr[2], 0, m_nVSplitPos + nFrameWidth, rcClient.right, rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[2]->SplitBoxOnOff( FALSE, TRUE, bSizeBox );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 2 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, m_nHSplitPos, rcClient.bottom, TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
		if( m_ChildWndArr[1] != NULL ){
			::MoveWindow( m_ChildWndArr[1], m_nHSplitPos + nFrameWidth, 0, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  rcClient.bottom, TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[1]->SplitBoxOnOff( TRUE, FALSE, bSizeBox );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
	}else{
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, m_nHSplitPos,  m_nVSplitPos, TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
		if( m_ChildWndArr[1] != NULL ){
			::MoveWindow( m_ChildWndArr[1], m_nHSplitPos + nFrameWidth, 0, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  m_nVSplitPos, TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
		if( m_ChildWndArr[2] != NULL ){
			::MoveWindow( m_ChildWndArr[2], 0, m_nVSplitPos + nFrameWidth , m_nHSplitPos,  rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
		if( m_ChildWndArr[3] != NULL ){
			::MoveWindow( m_ChildWndArr[3], m_nHSplitPos + nFrameWidth, m_nVSplitPos + nFrameWidth, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* �q�E�B���h�E�z�� */
			pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, bSizeBox );	/* �c�E���̕����{�b�N�X�̂n�m�^�n�e�e */
		}
	}
	//�f�X�N�g�b�v��������̂ł���!
	//::InvalidateRect( GetHwnd(), NULL, TRUE );	//�ĕ`�悵�ĂˁB	//@@@ 2003.06.11 MIK
	return 0L;
}



/* �}�E�X�ړ����̏��� */
LRESULT CSplitterWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	int		nHit;
	HCURSOR	hcurOld;	/* ���Ƃ̃}�E�X�J�[�\�� */
	RECT	rc;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	switch( nHit ){
	case 1:
		hcurOld = ::SetCursor( ::LoadCursor( NULL, IDC_SIZENS ) );
		break;
	case 2:
		hcurOld = ::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) );
		break;
	case 3:
		hcurOld = ::SetCursor( ::LoadCursor( NULL, IDC_SIZEALL ) );
		break;
	}
	if( 0 != m_bDragging ){		/* �����o�[���h���b�O���� */
		::GetClientRect( GetHwnd(), &rc );
		if( xPos < 1 ){
			xPos = 1;
		}
		if( xPos > rc.right - 6 ){
			xPos = rc.right - 6;
		}
		if( yPos < 1 ){
			yPos = 1;
		}
		if( yPos > rc.bottom - 6 ){
			yPos = rc.bottom - 6;
		}
		/* �����g���b�J�[�̕\�� */
		DrawSplitter( xPos, yPos, TRUE );
//		MYTRACE( _T("xPos=%d yPos=%d \n"), xPos, yPos );
	}
	return 0L;
}



/* �}�E�X���{�^���������̏��� */
LRESULT CSplitterWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int		nHit;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	::SetFocus( GetParentHwnd() );
	/* �����o�[�ւ̃q�b�g�e�X�g */
	nHit = HitTestSplitter( xPos, yPos );
	if( 0 != nHit ){
		m_bDragging = nHit;	/* �����o�[���h���b�O���� */
		::SetCapture( GetHwnd() );
	}
	/* �����g���b�J�[�̕\�� */
	DrawSplitter( xPos, yPos, FALSE );

	return 0L;
}




/* �}�E�X���{�^��������̏��� */
LRESULT CSplitterWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	int bDraggingOld;
	int nX;
	int nY;

	if( m_bDragging ){
		/* �����g���b�J�[�̕\�� */
		DrawSplitter( m_nDragPosX, m_nDragPosY, FALSE );
		bDraggingOld = m_bDragging;
		m_bDragging = 0;
		::ReleaseCapture();
		if( NULL != m_hcurOld ){
			::SetCursor( m_hcurOld );
		}
		/* �E�B���h�E�̕��� */
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nDragPosY;
		}
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nDragPosX;
		}
		if( bDraggingOld == 1 ){
			DoSplit( m_nHSplitPos, nY );
		}else
		if( bDraggingOld == 2 ){
			DoSplit( nX, m_nVSplitPos );
		}else
		if( bDraggingOld == 3 ){
			DoSplit( nX, nY );
		}
	}
	return 0L;
}




/* �}�E�X���{�^���_�u���N���b�N���̏��� */
LRESULT CSplitterWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nX;
	int nY;
	int	nHit;
	int	xPos;
	int	yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	if( nHit == 1 ){
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nHSplitPos;
		}
		DoSplit( nX , 0 );
	}else
	if( nHit == 2 ){
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nVSplitPos;
		}
		DoSplit( 0 , nY );
	}else
	if( nHit == 3 ){
		DoSplit( 0 , 0 );
	}
	OnMouseMove( GetHwnd(), 0, 0, MAKELONG( xPos, yPos ) );
	return 0L;
}




/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */
LRESULT CSplitterWnd::DispatchEvent_WM_APP( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nPosX;
	int nPosY;
	switch( uMsg ){
	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( _T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY );

		/* �E�B���h�E�̕��� */
		if( 0 != m_nHSplitPos ){
			nPosX = m_nHSplitPos;
		}
		if( 0 != m_nVSplitPos ){
			nPosY = m_nVSplitPos;
		}
		DoSplit( nPosX , nPosY );
		break;
	case MYWM_SETACTIVEPANE:
		SetActivePane( (int)wParam );
		break;
	}
	return 0L;
}



