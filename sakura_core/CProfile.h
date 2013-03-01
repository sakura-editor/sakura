/*!	@file
	@brief INI�t�@�C�����o��

	@author D.S.Koba
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

#ifndef _CPROFILE_H_
#define _CPROFILE_H_

#include <Windows.h>
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
	//������^
	typedef std::wstring wstring;
	typedef std::string string;

	typedef std::pair< wstring, wstring > PAIR_STR_STR;
	typedef std::map< wstring, wstring > MAP_STR_STR;
	struct Section
	{
		wstring     strSectionName;
		MAP_STR_STR mapEntries;
	};

public:
	CProfile() {}
	~CProfile() {}
	void Init( void );
	bool IsReadingMode( void ) { return m_bRead; }
	void SetReadingMode( void ) { m_bRead = true; }
	void SetWritingMode( void ) { m_bRead = false; }
	bool ReadProfile( const TCHAR* );
	bool ReadProfileRes( const TCHAR*, const TCHAR* );				// 200/5/19 Uchi
	bool WriteProfile( const TCHAR*, const WCHAR* pszComment);



	void DUMP( void );

protected:
	void ReadOneline( const wstring& line );
	bool _WriteFile( const tstring& strFilename, const std::vector< wstring >& vecLine);


	bool GetProfileDataImp( const wstring& strSectionName, const wstring& strEntryKey, wstring& strEntryValue);

	bool SetProfileDataImp( const wstring& strSectionName, const wstring& strEntryKey, const wstring& strEntryValue );

protected:
	// �����o�ϐ�
	tstring					m_strProfileName;	//!< �Ō�ɓǂݏ��������t�@�C����
	std::vector< Section >	m_ProfileData;
	bool					m_bRead;			//!< ���[�h(true=�ǂݍ���/false=�����o��)
};

#define _INI_T LTEXT




///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */


