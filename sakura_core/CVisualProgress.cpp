#include "StdAfx.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"

#include "window/CEditWnd.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CVisualProgress::CVisualProgress()
: m_pcWaitCursor(NULL)
{
}

CVisualProgress::~CVisualProgress()
{
	SAFE_DELETE(m_pcWaitCursor);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeLoad(SLoadInfo* sLoadInfo)
{
	_Begin();
}

void CVisualProgress::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	_End();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	_Begin();
}

void CVisualProgress::OnFinalSave(ESaveResult eSaveResult)
{
	_End();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �v���O���X��M                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnProgress(int nPer)
{
	_Doing(nPer);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::_Begin()
{
	//�����v
	if(!m_pcWaitCursor){
		m_pcWaitCursor = new CWaitCursor( CEditWnd::getInstance()->GetHwnd() );
	}

	//�v���O���X�o�[
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
		//�͈͐ݒ�E���Z�b�g
		Progress_SetRange( hwndProgress, 0, 100 );
		Progress_SetPos( hwndProgress, 0);
	}
}

void CVisualProgress::_Doing(int nPer)
{
	//�v���O���X�o�[�X�V
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if(hwndProgress)
		Progress_SetPos( hwndProgress, nPer );
}

void CVisualProgress::_End()
{
	//�v���O���X�o�[
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		Progress_SetPos( hwndProgress, 0);
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	//�����v
	SAFE_DELETE(m_pcWaitCursor);
}

