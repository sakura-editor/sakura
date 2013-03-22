/*!	@file
	@brief COsVersionInfo

	OSVERSIONINFO�����b�s���O

	@author YAZAKI
	@date 2002�N3��3��
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

class COsVersionInfo {
public:
	// ���������s��(�����̓_�~�[)
	// �ďo�͊�{1��̂�
	COsVersionInfo( bool pbStart ) {
		memset_raw( &m_cOsVersionInfo, 0, sizeof( m_cOsVersionInfo ) );
		m_cOsVersionInfo.dwOSVersionInfoSize = sizeof( m_cOsVersionInfo );
		m_bSuccess = ::GetVersionEx( &m_cOsVersionInfo );
	}

	// �ʏ�̃R���X�g���N�^
	// �������Ȃ�
	COsVersionInfo() {}

	/* OsVersion���擾�ł������H */
	BOOL GetVersion(){
		return m_bSuccess;
	}

	/* �g�p���Ă���OS�iWindows�j���A����Ώۂ��m�F���� */
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
	/*! NT�v���b�g�t�H�[�����ǂ������ׂ�

		@retval true NT platform
		@retval false non-NT platform
	*/
	bool _IsWin32NT(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	// 2005.10.31 ryoji
	/*! Windows�v���b�g�t�H�[�����ǂ������ׂ�

		@retval true Windows platform
		@retval false non-Windows platform
	*/
	bool IsWin32Windows(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

	/*	::WinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );
		���g�p�ł��Ȃ��o�[�W�����Ȃ�Atrue
		�g�p�ł���o�[�W�����Ȃ�Afalse
	*/
	bool _HasWinHelpContentsProblem(){
		return ( _IsWin32NT() && (m_cOsVersionInfo.dwMajorVersion <= 4));
	}

	/*	�ĕϊ���OS�W���Œ񋟂���Ă��Ȃ����B
		�񋟂���Ă��Ȃ��Ȃ�Afalse�B
		�񋟂���Ă���Ȃ�Atrue�B

		Windows95 or WindowsNT�Ȃ�AFASLE�i�񋟂���Ă��Ȃ��j
		����ȊO��OS�Ȃ�Atrue�i�񋟂���Ă���j
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

	/*! Windows Vista�ȏォ���ׂ�

		@retval true Windows Vista or later

		@date 2007.05.19 ryoji
	*/
	bool _IsWinVista_or_later()
	{
		return ( 6 <= m_cOsVersionInfo.dwMajorVersion );
	}

	/*! Windows XP�ȏォ���ׂ�

		@retval true Windows XP or later

		@date 2003.09.06 genta
	*/
	bool _IsWinXP_or_later(){
		return ( m_cOsVersionInfo.dwMajorVersion >= 6 ||	// 2006.06.17 ryoji Ver 6.0, 7.0,...���܂߂�
			(m_cOsVersionInfo.dwMajorVersion >= 5 && m_cOsVersionInfo.dwMinorVersion >= 1) );
	}

	/*! Windows 2000�ȏォ���ׂ�

		@retval true Windows 2000 or later

		@date 2005.10.26 ryoji
	*/
	bool _IsWin2000_or_later(){
		return ( _IsWin32NT() && (5 <= m_cOsVersionInfo.dwMajorVersion) );
	}

	/*! Windows Me�����ׂ�

		@retval true Windows Me

		@date 2005.10.26 ryoji
	*/
	bool _IsWinMe(){
		return ( IsWin32Windows() && (4 == m_cOsVersionInfo.dwMajorVersion) && ( 90 == m_cOsVersionInfo.dwMinorVersion ) );
	}

protected:
	// Class��static(�S�N���X���L)�ϐ��ȊO�����Ȃ�
	static BOOL m_bSuccess;
	static OSVERSIONINFO m_cOsVersionInfo;
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

#endif


