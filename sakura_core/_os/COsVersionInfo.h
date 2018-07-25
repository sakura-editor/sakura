/*!	@file
	@brief COsVersionInfo

	OSVERSIONINFOをラッピング

	@author YAZAKI
	@date 2002年3月3日
*/
/*
	Copyright (C) 2001, YAZAKI, shoji masami
	Copyright (C) 2002, YAZAKI, minfu
	Copyright (C) 2003, genta
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

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

#ifndef _COSVERSIONINFO_H_
#define _COSVERSIONINFO_H_

#ifndef _WIN32_WINNT_WIN2K
#define _WIN32_WINNT_WIN2K	0x0500
#endif
#ifndef _WIN32_WINNT_WINXP
#define _WIN32_WINNT_WINXP	0x0501
#endif
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA	0x0600
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7	0x0601
#endif

#ifdef USE_SSE2
#ifdef __MINGW32__
#include <cpuid.h>
#else
#include <intrin.h>
#endif
#endif

class COsVersionInfo {
public:
	// 初期化を行う(引数はダミー)
	// 呼出は基本1回のみ
	COsVersionInfo( bool pbStart );

	// 通常のコンストラクタ
	// 何もしない
	COsVersionInfo() {}

	/* OsVersionが取得できたか？ */
	BOOL GetVersion(){
		return m_bSuccess;
	}

	/* 使用しているOS（Windows）が、動作対象か確認する */
	bool OsIsEnableVersion(){
#if (WINVER >= _WIN32_WINNT_WIN7)
		return ( _IsWin32NT() &&
			(m_cOsVersionInfo.dwMajorVersion >= 7 ||
			(m_cOsVersionInfo.dwMajorVersion == 6 && m_cOsVersionInfo.dwMinorVersion >= 1)) );
#elif (WINVER >= _WIN32_WINNT_VISTA)
		return ( _IsWin32NT() && (m_cOsVersionInfo.dwMajorVersion >= 6) );
#elif (WINVER >= _WIN32_WINNT_WIN2K)
		return ( _IsWin32NT() && (m_cOsVersionInfo.dwMajorVersion >= 5) );
#else
		return ( m_cOsVersionInfo.dwMajorVersion >= 4 );
#endif
	}


	// From Here Jul. 5, 2001 shoji masami
	/*! NTプラットフォームかどうか調べる

		@retval true NT platform
		@retval false non-NT platform
	*/
	bool _IsWin32NT(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	// 2005.10.31 ryoji
	/*! Windowsプラットフォームかどうか調べる

		@retval true Windows platform
		@retval false non-Windows platform
	*/
	bool IsWin32Windows(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

	/*	::WinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );
		が使用できないバージョンなら、true
		使用できるバージョンなら、false
	*/
	bool _HasWinHelpContentsProblem(){
		return ( _IsWin32NT() && (m_cOsVersionInfo.dwMajorVersion <= 4));
	}

	/*	再変換がOS標準で提供されていないか。
		提供されていないなら、false。
		提供されているなら、true。

		Windows95 or WindowsNTなら、FASLE（提供されていない）
		それ以外のOSなら、true（提供されている）
	*/
	bool _OsSupportReconvert(){
		return !((4 == m_cOsVersionInfo.dwMajorVersion) && ( 0 == m_cOsVersionInfo.dwMinorVersion ));
	}

	// 2005.10.29 ryoji
	// Windows 2000 version of OPENFILENAME.
	// The new version has three extra members.
	// See CommDlg.h
	bool _IsWinV5forOfn() {
		return (_IsWin2000_or_later() || _IsWinMe()); 
	}

	/*! Windows Vista以上か調べる

		@retval true Windows Vista or later

		@date 2007.05.19 ryoji
	*/
	bool _IsWinVista_or_later()
	{
		return ( 6 <= m_cOsVersionInfo.dwMajorVersion );
	}

	/*! Windows XP以上か調べる

		@retval true Windows XP or later

		@date 2003.09.06 genta
	*/
	bool _IsWinXP_or_later(){
		return ( m_cOsVersionInfo.dwMajorVersion >= 6 ||	// 2006.06.17 ryoji Ver 6.0, 7.0,...も含める
			(m_cOsVersionInfo.dwMajorVersion >= 5 && m_cOsVersionInfo.dwMinorVersion >= 1) );
	}

	/*! Windows 2000以上か調べる

		@retval true Windows 2000 or later

		@date 2005.10.26 ryoji
	*/
	bool _IsWin2000_or_later(){
		return ( _IsWin32NT() && (5 <= m_cOsVersionInfo.dwMajorVersion) );
	}

	/*! Windows Meか調べる

		@retval true Windows Me

		@date 2005.10.26 ryoji
	*/
	bool _IsWinMe(){
		return ( IsWin32Windows() && (4 == m_cOsVersionInfo.dwMajorVersion) && ( 90 == m_cOsVersionInfo.dwMinorVersion ) );
	}

#ifdef USE_SSE2
	/*! SSE2サポートかを調べる

		@retval true support SSE2
	*/
	bool _SupportSSE2(){
		return m_bSSE2;
	}
#endif

	/*! Wine上で実行されているかを調べる

		@retval true run in Wine

		@date 2013.10.19 novice
	*/
	bool _IsWine(){
		return m_bWine;
	}

protected:
	// Classはstatic(全クラス共有)変数以外持たない
	static BOOL m_bSuccess;
	static OSVERSIONINFO m_cOsVersionInfo;
#ifdef USE_SSE2
	static bool m_bSSE2;
#endif
	static bool m_bWine;
};



inline bool IsWin32NT() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return COsVersionInfo()._IsWin32NT();
#endif
}

inline bool HasWinHelpContentsProblem() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return false;
#else
	return COsVersionInfo()._HasWinHelpContentsProblem();
#endif
}

inline bool OsSupportReconvert() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return COsVersionInfo()._OsSupportReconvert();
#endif
}

inline bool IsWinV5forOfn() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return COsVersionInfo()._IsWinV5forOfn();
#endif
}

inline bool IsWinVista_or_later() {
#if (WINVER >= _WIN32_WINNT_VISTA)
	return true;
#else
	return COsVersionInfo()._IsWinVista_or_later();
#endif
}

inline bool IsWinXP_or_later() {
#if (WINVER >= _WIN32_WINNT_WINXP)
	return true;
#else
	return COsVersionInfo()._IsWinXP_or_later();
#endif
}

inline bool IsWin2000_or_later() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return COsVersionInfo()._IsWin2000_or_later();
#endif
}

inline bool IsWinMe() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return false;
#else
	return COsVersionInfo()._IsWinMe();
#endif
}

inline bool IsWine() {
	return COsVersionInfo()._IsWine();
}

#endif


