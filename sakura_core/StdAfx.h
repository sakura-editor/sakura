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

#include "global.h"
#include <windows.h>
#include <commdlg.h> // PRINTDLG, 
#include <comutil.h> // VT_BSTR,..
#include <objidl.h>  // LPDATAOBJECT
#include <shellapi.h>// HDROP
#include <stdlib.h>  // _MAX_PATH

// TODO: �v���O�����ŕK�v�ȃw�b�_�[�Q�Ƃ�ǉ����Ă��������B

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_STDAFX_H__11490042_E569_11D3_BCE2_444553540001__INCLUDED_)


/*[EOF]*/
