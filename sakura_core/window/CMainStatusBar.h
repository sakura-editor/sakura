/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CMAINSTATUSBAR_2F45F977_E8DD_4787_8EA8_FF15685D114F_H_
#define SAKURA_CMAINSTATUSBAR_2F45F977_E8DD_4787_8EA8_FF15685D114F_H_

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

#endif /* SAKURA_CMAINSTATUSBAR_2F45F977_E8DD_4787_8EA8_FF15685D114F_H_ */
/*[EOF]*/
