#pragma once

//�V�X�e������
SAKURA_CORE_API BOOL GetSystemResources( int*, int*, int* );	/* �V�X�e�����\�[�X�𒲂ׂ� */
SAKURA_CORE_API BOOL CheckSystemResources( const TCHAR* );	/* �V�X�e�����\�[�X�̃`�F�b�N */

//�N���b�v�{�[�h
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength );    //!< �N���[�v�{�[�h��Text�`���ŃR�s�[����BANSI�ŁBnLength�͕����P�ʁB
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength ); //!< �N���[�v�{�[�h��Text�`���ŃR�s�[����BUNICODE�ŁBnLength�͕����P�ʁB

//	Sep. 10, 2002 genta CWSH.cpp����̈ړ��ɔ����ǉ�
bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount);

//	May 01, 2004 genta �}���`���j�^�Ή��̃f�X�N�g�b�v�̈�擾
bool GetMonitorWorkRect(HWND     hWnd, LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji �p�����[�^ prcMonitor ��ǉ�
bool GetMonitorWorkRect(LPCRECT  prc,  LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(POINT    pt,   LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji


// 2006.06.17 ryoji
#define PACKVERSION( major, minor ) MAKELONG( minor, major )
DWORD GetComctl32Version();					// Comctl32.dll �̃o�[�W�����ԍ����擾						// 2006.06.17 ryoji
BOOL IsVisualStyle();						// ���������݃r�W���A���X�^�C���\����Ԃ��ǂ���������		// 2006.06.17 ryoji
void PreventVisualStyle( HWND hWnd );		// �w��E�B���h�E�Ńr�W���A���X�^�C�����g��Ȃ��悤�ɂ���	// 2006.06.23 ryoji
void MyInitCommonControls();				// �R�����R���g���[��������������							// 2006.06.21 ryoji
