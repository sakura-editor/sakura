#pragma once

SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* �A�N�e�B�u�ɂ��� */

/*
||	�������̃��[�U�[������\�ɂ���
||	�u���b�L���O�t�b�N(?)(���b�Z�[�W�z��)
*/
SAKURA_CORE_API BOOL BlockingHook( HWND hwndDlgCancel );


#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100


HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// �w�肵���E�B���h�E�̑c��̃n���h�����擾����	// 2007.07.01 ryoji
