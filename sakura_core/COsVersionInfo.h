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

class COsVersionInfo {
public:
	COsVersionInfo(){
		memset_raw( &m_cOsVersionInfo, 0, sizeof( m_cOsVersionInfo ) );
		m_cOsVersionInfo.dwOSVersionInfoSize = sizeof( m_cOsVersionInfo );
		m_bSuccess = ::GetVersionEx( &m_cOsVersionInfo );
	}
	
	/* OsVersion���擾�ł������H */
	BOOL GetVersion(){
		return m_bSuccess;
	}
	
	/* �g�p���Ă���OS�iWindows�j���A����Ώۂ��m�F���� */
	BOOL OsIsEnableVersion(){
		return !( m_cOsVersionInfo.dwMajorVersion < 4 );
	}
	
	
	// From Here Jul. 5, 2001 shoji masami
	/*! NT�v���b�g�t�H�[�����ǂ������ׂ�

		@retval TRUE NT platform
		@retval FALSE non-NT platform
	*/
	BOOL IsWin32NT(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}
	
	// 2005.10.31 ryoji
	/*! Windows�v���b�g�t�H�[�����ǂ������ׂ�

		@retval TRUE Windows platform
		@retval FALSE non-Windows platform
	*/
	BOOL IsWin32Windows(){
		return (m_cOsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

	/*	::WinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );
		���g�p�ł��Ȃ��o�[�W�����Ȃ�ATRUE
		�g�p�ł���o�[�W�����Ȃ�AFALSE
	*/
	BOOL HasWinHelpContentsProblem(){
		return ( IsWin32NT() && (m_cOsVersionInfo.dwMajorVersion <= 4));
	}
	
	/*	�ĕϊ���OS�W���Œ񋟂���Ă��Ȃ����B
		�񋟂���Ă��Ȃ��Ȃ�ATRUE�B
		�񋟂���Ă���Ȃ�AFALSE�B
	
		Windows95 or WindowsNT�Ȃ�ATRUE�i�񋟂���Ă��Ȃ��j
		����ȊO��OS�Ȃ�AFALSE�i�񋟂���Ă���j
	*/
	BOOL OsDoesNOTSupportReconvert(){
		return ((4 == m_cOsVersionInfo.dwMajorVersion) && ( 0 == m_cOsVersionInfo.dwMinorVersion ));
	}
#if 0
	2002.04.11 YAZAKI �J�v�Z���������B
	// 2002.04.08 minfu OSVERSIONINFO�\���̂ւ̃|�C���^��Ԃ�
	POSVERSIONINFO GetOsVersionInfo(){
		return &m_cOsVersionInfo;
	}
#endif

	/*! Windows XP�ȏォ���ׂ�

		@retval TRUE Windows XP or later

		@date 2003.09.06 genta
	*/
	BOOL IsWinXP_or_later(){
		return ( m_cOsVersionInfo.dwMajorVersion >= 6 ||	// 2006.06.17 ryoji Ver 6.0, 7.0,...���܂߂�
			(m_cOsVersionInfo.dwMajorVersion >= 5 && m_cOsVersionInfo.dwMinorVersion >= 1) );
	}

	/*! Windows 2000�ȏォ���ׂ�

		@retval TRUE Windows 2000 or later

		@date 2005.10.26 ryoji
	*/
	BOOL IsWin2000_or_later(){
		return ( IsWin32NT() && (5 <= m_cOsVersionInfo.dwMajorVersion) );
	}

	/*! Windows Me�����ׂ�

		@retval TRUE Windows Me

		@date 2005.10.26 ryoji
	*/
	BOOL IsWinMe(){
		return ( IsWin32Windows() && (4 == m_cOsVersionInfo.dwMajorVersion) && ( 90 == m_cOsVersionInfo.dwMinorVersion ) );
	}

	/*! Windows Vista�ȏォ���ׂ�

		@retval TRUE Windows Vista or later

		@date 2007.05.19 ryoji
	*/
	bool IsWinVista_or_later()
	{
		return ( 6 <= m_cOsVersionInfo.dwMajorVersion );
	}

protected:
	BOOL m_bSuccess;
	OSVERSIONINFO m_cOsVersionInfo;
};

#endif


