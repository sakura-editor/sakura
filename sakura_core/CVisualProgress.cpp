#include "stdafx.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"


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
		m_pcWaitCursor = new CWaitCursor( CEditWnd::Instance()->GetHwnd() );
	}

	//�v���O���X�o�[
	HWND hwndProgress = CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
		//�͈͐ݒ�E���Z�b�g
		::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
	}
}

void CVisualProgress::_Doing(int nPer)
{
	//�v���O���X�o�[�X�V
	HWND hwndProgress = CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	if(hwndProgress)
		::PostMessageAny( hwndProgress, PBM_SETPOS, nPer , 0 );
}

void CVisualProgress::_End()
{
	//�v���O���X�o�[
	HWND hwndProgress = CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	//�����v
	SAFE_DELETE(m_pcWaitCursor);
}

