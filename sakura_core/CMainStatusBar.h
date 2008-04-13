#pragma once

#include "doc/CDocListener.h"

class CEditWnd;

class CMainStatusBar : public CDocListenerEx{
public:
	//�쐬�E�j��
	CMainStatusBar(CEditWnd* pOwner);
	void CreateStatusBar();		// �X�e�[�^�X�o�[�쐬
	void DestroyStatusBar();		/* �X�e�[�^�X�o�[�j�� */
	void SendStatusMessage2( const TCHAR* msg );	//	Jul. 9, 2005 genta ���j���[�o�[�E�[�ɂ͏o�������Ȃ����߂̃��b�Z�[�W���o��
	/*!	SendStatusMessage2()�������ڂ����邩��\�߃`�F�b�N
		@date 2005.07.09 genta
		@note ����SendStatusMessage2()�ŃX�e�[�^�X�o�[�\���ȊO�̏�����ǉ�
		����ꍇ�ɂ͂�����ύX���Ȃ��ƐV�����ꏊ�ւ̏o�͂��s���Ȃ��D
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective() const
	{
		return NULL != m_hwndStatusBar;
	}

	//�擾
	HWND GetStatusHwnd() const{ return m_hwndStatusBar; }
	HWND GetProgressHwnd() const{ return m_hwndProgressBar; }

	//�ݒ�
	void SetStatusText(int nIndex, int nOption, const TCHAR* pszText);
private:
	CEditWnd*	m_pOwner;
	HWND		m_hwndStatusBar;
	HWND		m_hwndProgressBar;
};

