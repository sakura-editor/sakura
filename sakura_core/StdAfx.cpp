// stdafx.cpp : �W���C���N���[�h�t�@�C�����܂ރ\�[�X�t�@�C��
//				sakura_core.pch ���������v���R���p�C���σw�b�_�[
//				stdafx.obj ���������v���R���p�C���σ^�C�v���

#include "stdafx.h"

// TODO: STDAFX.H �Ɋ܂܂�Ă��āA���̃t�@�C���ɋL�q����Ă��Ȃ�
// �w�b�_�[�t�@�C����ǉ����Ă��������B



//########
HWND G_GetProgressBar()
{
	if(CEditWnd::Instance()){
		return CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	}
	else{
		return NULL;
	}
}
