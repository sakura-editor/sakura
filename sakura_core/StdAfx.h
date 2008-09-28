// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���A
//				�܂��͎Q�Ɖ񐔂������A�����܂�ύX����Ȃ�
//				�v���W�F�N�g��p�̃C���N���[�h �t�@�C�����L�q���܂��B
//

#if !defined(AFX_STDAFX_H__11490042_E569_11D3_BCE2_444553540001__INCLUDED_)
#define AFX_STDAFX_H__11490042_E569_11D3_BCE2_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ���̈ʒu�Ƀw�b�_�[��}�����Ă�������
#define WIN32_LEAN_AND_MEAN		// Windows �w�b�_�[����w�ǎg�p����Ȃ��X�^�b�t�����O���܂�

#if _MSC_VER >= 1400
#pragma warning( disable : 4996 )
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


//�r���h�I�v�V�����I�ȃw�b�_
#include "config/build_config.h"

//�萔
#include "config/system_constants.h"	//�V�X�e���萔
#include "config/app_constants.h"		//�A�v���P�[�V�����萔

//�O���[�o��
#include "global.h"

//���p�xAPI��
#include <windows.h>
#include <stdlib.h>  // _MAX_PATH
#include <tchar.h>

//�V���v���ł悭�g������
#include "basis/primitive.h"
#include "util/std_macro.h"

//MFC�݊�
#include "basis/CMyString.h"
#include "basis/CMyRect.h"
#include "basis/CMyPoint.h"
#include "basis/CMySize.h"

//�T�N���G�f�B�^�ŗL�^
#include "basis/SakuraBasis.h"

//�f�o�b�O
#include "debug/Debug2.h"
#include "debug/Debug3.h"

//�悭�g���w�b�_
#include "debug/Debug.h"
#include "mem/CNativeW.h"
#include "mem/CNativeA.h"
#include "mem/CNativeT.h"
#include <wchar.h>

#include "util/string_ex.h"

//���̑�
#define malloc_char (char*)malloc
#define GlobalLockChar  (char*)::GlobalLock
#define GlobalLockUChar (unsigned char*)::GlobalLock
#define GlobalLockWChar (wchar_t*)::GlobalLock
#define GlobalLockBYTE  (BYTE*)::GlobalLock

//API���b�v
#include "apiwrap/StdControl.h"
#include "apiwrap/CommonControl.h"
#include "apiwrap/StdApi.h"

//TCHAR���[�e�B���e�B
#include "util/tchar_convert.h"


//###########����
#include "CGraphics.h"
#include "CNormalProcess.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"
#include "docplus/CModifyManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CBookmarkManager.h"
#include "CReadManager.h"
#include "CWriteManager.h"
#include "CSearchAgent.h"
#include "doc/CDocReader.h"
#include "CAppMode.h"
#pragma warning( disable : 4355 ) //warning C4355: 'this' : �x�[�X �����o���������X�g�Ŏg�p����܂����B

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_STDAFX_H__11490042_E569_11D3_BCE2_444553540001__INCLUDED_)



