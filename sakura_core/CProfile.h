//	$Id$
/*!	@file
	@brief INI�t�@�C�����o��

	@author D.S.Koba
	$Revision$
	@date 2003-10-21 D.S.Koba �����o�֐��̖��O�ƈ��������̂܂܂ɂ��ă����o�ϐ��C�֐��̒��g����������
	@date 2004-01-10 D.S.Koba �Ԓl��BOOL����bool�֕ύX�BIOProfileData���^�ʂ̊֐��ɕ����C���������炷
	@date 2006-02-11 D.S.Koba �ǂݍ���/�����o���������łȂ��C�����o�Ŕ���
	@date 2006-02-12 D.S.Koba IOProfileData�̒��g�̓ǂݍ��݂Ə����o�����֐��ɕ�����
*/
/*
	Copyright (C) 2003-2006, D.S.Koba

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

class CProfile;
#ifndef _CPROFILE_H_
#define _CPROFILE_H_

#pragma warning( disable : 4786 )

#include <windows.h>
#include <string>
#include <vector>
#include <map>

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief INI�t�@�C�����o��
*/
class CProfile
{
	typedef std::pair< std::basic_string< TCHAR >, std::basic_string< TCHAR > > PAIR_STR_STR;
	typedef std::map< std::basic_string< TCHAR >, std::basic_string< TCHAR > > MAP_STR_STR;
	struct Section
	{
		std::basic_string< TCHAR > strSectionName;
		MAP_STR_STR mapEntries;
	};

public:
	CProfile() {}
	~CProfile() {};
	void Init( void );
	bool IsReadingMode( void ) { return m_bRead; }
	void SetReadingMode( void ) { m_bRead = true; }
	void SetWritingMode( void ) { m_bRead = false; }
	bool ReadProfile( const TCHAR* );
	bool WriteProfile( const TCHAR*, const TCHAR* );

	bool IOProfileData( const TCHAR*, const TCHAR*, bool& );
	bool IOProfileData( const TCHAR*, const TCHAR*, int& );
	bool IOProfileData( const TCHAR*, const TCHAR*, WORD& );
	bool IOProfileData( const TCHAR*, const TCHAR*, TCHAR& );
	bool IOProfileData( const TCHAR*, const TCHAR*, TCHAR*, const int& );
	bool IOProfileData( const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, std::basic_string< TCHAR >& );

	void DUMP( void );

protected:
	void ReadOneline( const std::basic_string< TCHAR >& line );
	bool WriteFile( const std::basic_string< TCHAR >&, std::vector< std::basic_string< TCHAR > >& );

	bool GetProfileData( const TCHAR*, const TCHAR*, bool& );
	bool GetProfileData( const TCHAR*, const TCHAR*, int& );
	bool GetProfileData( const TCHAR*, const TCHAR*, WORD& );
	bool GetProfileData( const TCHAR*, const TCHAR*, TCHAR& );
	bool GetProfileData( const TCHAR*, const TCHAR*, TCHAR*, const int& );
	bool GetProfileData( const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, std::basic_string< TCHAR >& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const bool& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const int& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const WORD& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const TCHAR& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const TCHAR*, const int& );
	bool SetProfileData( const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >& );

	// �����o�ϐ�
	std::basic_string< TCHAR > m_strProfileName;//!< �Ō�ɓǂݏ��������t�@�C����
	std::vector< Section > m_ProfileData;
	bool m_bRead;//!< ���[�h(true=�ǂݍ���/false=�����o��)
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */

/*[EOF]*/
