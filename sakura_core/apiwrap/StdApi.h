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
#ifndef SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_
#define SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_

#include <ImageHlp.h> //MakeSureDirectoryPathExists

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          W�n�����݂��Ȃ�API�̂��߂́A�V�����֐���`         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//W�ł������̂ŁA����
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR wszDirPath);
#ifdef _UNICODE
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExistsW
#else
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExists
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W�n�`��API (ANSI�łł����p�\)                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	BOOL ExtTextOutW_AnyBuild(
		HDC				hdc,
		int				x,
		int				y,
		UINT			fuOptions,
		const RECT*		lprc,
		LPCWSTR			lpwString,
		UINT			cbCount,
		const int*		lpDx
	);

	BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpString,
		int		cbString
	);

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	);

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	);

#ifdef _UNICODE
	#define GetTextExtentPoint32W_AnyBuild GetTextExtentPoint32
#else
	BOOL GetTextExtentPoint32W_AnyBuild(
		HDC		hdc, 
		LPCWSTR	lpString, 
		int		cbString, 
		LPSIZE	lpSize
	);
#endif



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���̑�W�nAPI (ANSI�łł����p�\)               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< �o�b�t�@�̃T�C�Y�B�����P�ʁB
	);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    �`��API �s����b�v                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//Vista��SetPixel�������Ȃ����߁A��֊֐���p�ӁB
	void SetPixelSurely(HDC hdc,int x,int y,COLORREF c);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �悭�g�������l                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! �悭�g��ExtTextOutW_AnyBuild�̃I�v�V����
	inline UINT ExtTextOutOption()
	{
		return ETO_CLIPPED | ETO_OPAQUE;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �悭�g���p�@                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! SHIFT�������Ă��邩�ǂ���
	inline bool GetKeyState_Shift()
	{
		return (::GetKeyState(VK_SHIFT)&0x8000)!=0;
	}

	//! CTRL�������Ă��邩�ǂ���
	inline bool GetKeyState_Control()
	{
		return (::GetKeyState(VK_CONTROL)&0x8000)!=0;
	}

	//! ALT�������Ă��邩�ǂ���
	inline bool GetKeyState_Alt()
	{
		return (::GetKeyState(VK_MENU)&0x8000)!=0;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �萔                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const size_t sizeof_old_PROPSHEETHEADER = sizeof(PROPSHEETHEADER);
	#else
		static const size_t sizeof_old_PROPSHEETHEADER = 40;
	#endif

	//	Jan. 29, 2002 genta
	//	Win95/NT���[������sizeof( MENUITEMINFO )
	//	����ȊO�̒l��^����ƌÂ�OS�ł����Ɠ����Ă���Ȃ��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const int SIZEOF_MENUITEMINFO = sizeof(MENUITEMINFO);
	#else
		static const int SIZEOF_MENUITEMINFO = 44;
	#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             SendMessage,PostMessage�Ӗ��t��                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �ߋ���UNICODE���̖��c�ł��B
	// ���݂ƂȂ��ẮA���ɈӖ��͂���܂���B

	//�����R�[�h�Ɋ֌W�̂Ȃ������� SendMessage �� SendMessageAny �ɍ����ւ��Ă����B
	#define SendMessageAny SendMessage

	//WM_COMMAND�n�� SendMessage �� SendMessageCmd �ɍ����ւ��Ă����B
	#define SendMessageCmd SendMessage

	//�����R�[�h�Ɋ֌W�̂Ȃ������� PostMessage �� PostMessageAny �ɍ����ւ��Ă����B
	#define PostMessageAny PostMessage

	//WM_COMMAND�n�� PostMessage �� PostMessageCmd �ɍ����ւ��Ă����B
	#define PostMessageCmd PostMessage

}
using namespace ApiWrap;




//	Sep. 22, 2003 MIK
//	�Â�SDK�΍�D�V����SDK�ł͕s�v
#ifndef _WIN64
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif
#ifndef ULONG_PTR
#define ULONG_PTR ULONG
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef UINT_PTR
#define UINT_PTR UINT
#endif
#ifndef INT_PTR
#define INT_PTR INT
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE GWL_HINSTANCE
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT DWL_MSGRESULT
#endif
#endif  //_WIN64

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif

#endif /* SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_ */
/*[EOF]*/
