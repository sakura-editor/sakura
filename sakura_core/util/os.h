#pragma once

//�V�X�e������
SAKURA_CORE_API BOOL GetSystemResources( int*, int*, int* );	/* �V�X�e�����\�[�X�𒲂ׂ� */
SAKURA_CORE_API BOOL CheckSystemResources( const TCHAR* );	/* �V�X�e�����\�[�X�̃`�F�b�N */

//�N���b�v�{�[�h
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength );    //!< �N���[�v�{�[�h��Text�`���ŃR�s�[����BANSI�ŁBnLength�͕����P�ʁB
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength ); //!< �N���[�v�{�[�h��Text�`���ŃR�s�[����BUNICODE�ŁBnLength�͕����P�ʁB
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );

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


//�J�����g�f�B���N�g�����[�e�B���e�B�B
//�R���X�g���N�^�ŃJ�����g�f�B���N�g����ۑ����A�f�X�g���N�^�ŃJ�����g�f�B���N�g���𕜌����郂�m�B
//2008.03.01 kobake �쐬
class CCurrentDirectoryBackupPoint{
public:
	CCurrentDirectoryBackupPoint();
	~CCurrentDirectoryBackupPoint();
private:
	TCHAR m_szCurDir[_MAX_PATH];
};



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���b�Z�[�W�萔                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- �}�E�X -- -- //

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif
// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif


// -- -- �e�[�} -- -- //

// 2006.06.17 ryoji WM_THEMECHANGED
#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif


// -- -- IME (imm.h) -- -- //

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

/* 2002.04.09 minfu �ĕϊ����� */
#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING

