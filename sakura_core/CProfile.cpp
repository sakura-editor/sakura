//	$Id$
/*!	@file
	@brief INI�t�@�C�����o��

	@author D.S.Koba
	$Revision$
	@date 2003-10-21 D.S.Koba �����o�֐��̖��O�ƈ��������̂܂܂ɂ��ă����o�ϐ��C�֐��̒��g����������
	@date 2004-01-10 D.S.Koba �Ԓl��BOOL����bool�֕ύX�BIOProfileData���^�ʂ̊֐��ɕ����C���������炷
*/
/*
	Copyright (C) 2003, D.S.Koba

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

#include "CProfile.h"
#include "Debug.h"
#include <fstream>
#include <sstream>
#include <algorithm>


/*! Profile��������
	
	@date 2003-10-21 D.S.Koba STL�ŏ�������
*/
void CProfile::Init( void )
{
	m_strProfileName = _T("");
	m_ProfileData.clear();
	return;
}

/*! Profile���t�@�C������ǂݏo��
	
	@param pszProfileName [in] �t�@�C����

	@retval true  ����
	@retval false ���s

	@date 2003-10-21 D.S.Koba STL�ŏ�������
*/
bool CProfile::ReadProfile( const TCHAR* pszProfileName )
{
	m_strProfileName = pszProfileName;
	m_ProfileData.reserve( 34 );//�������̂��߃������m��

	// �t�@�C���ǂݍ���
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( false == ReadFile( m_strProfileName, vecLine ) ) return false;

	unsigned int idx;
	VEC_STR_ITER iter;
	VEC_STR_ITER enditer = vecLine.end();

	// �R�����g�폜
	for( iter=vecLine.begin(); iter!=enditer; iter++ )
	{
		idx = iter->find( _T("//") );
		if( iter->npos != idx )
		{
			iter->erase( idx );
		}
	}

	try
	{
		for( iter=vecLine.begin(); iter!=enditer; iter++ )
		{
			// �Z�N�V�����擾
			if( 0 == iter->find( _T("[") ) && iter->npos == iter->find( _T("=") ) )
			{
				// �f�[�^�𒼐ڕύX����̂ŁC�ȍ~�Z�N�V�������Ƃ͕�����Ȃ��Ȃ邱�Ƃɒ���
				iter->erase( iter->begin() );
				iter->erase( --(iter->end()) );
				Section Buffer;
				Buffer.strSectionName = iter->data();
				m_ProfileData.push_back( Buffer );
			}
			// �G���g���擾
			else if( 0 != m_ProfileData.size() )	//�ŏ��̃Z�N�V�����ȑO�̍s�̃G���g���͖���
			{
				idx = iter->find( _T("=") );
				if( iter->npos != idx )
				{
					m_ProfileData.back().Data.insert( PAIR_STR_STR( iter->substr(0,idx), iter->substr(idx+1) ) );
				}
			}
		}
	} //try
	catch(...)
	{
		//ini�t�@�C��������ǂݎ�莸�s
		return false;
	}
    return true;
}

/*! Profile�̓���̃Z�N�V�������t�@�C������ǂݏo��
	
	@param pszProfileName [in] �t�@�C����
	@param pszSectionName [in] �Z�N�V������

	@retval true  ����
	@retval false ���s

	@date 2003-10-26 D.S.Koba ReadProfile()�����ɍ쐬
*/
bool CProfile::ReadProfileSection(
	const TCHAR* pszProfileName,
	const TCHAR* pszSectionName
)
{
	m_strProfileName = pszProfileName;

	// �t�@�C���ǂݍ���
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( false == ReadFile( m_strProfileName, vecLine ) ) return false;

	unsigned int idx;
	VEC_STR_ITER iter;
	VEC_STR_ITER enditer = vecLine.end();

	// �R�����g�폜
	for( iter=vecLine.begin(); iter!=enditer; iter++ )
	{
		idx = iter->find( _T("//") );
		if( iter->npos != idx )
		{
			iter->erase( idx );
		}
	}

	std::basic_string< TCHAR > strSectionLine = _T("[");
	strSectionLine += pszSectionName;
	strSectionLine += _T("]");

	//�ړI�̃Z�N�V�������擾
	iter = std::find( vecLine.begin(), vecLine.end(), strSectionLine );
	if( vecLine.end() == iter ) return false;
	
	Section Buffer;
	Buffer.strSectionName = pszSectionName;
	m_ProfileData.push_back( Buffer );

	try
	{
		//�G���g�����擾
		for( ++iter; iter!=enditer; iter++ )
		{
			//���̃Z�N�V�����ɓ�������G���g���ǂݍ��ݏI��
			if( 0 == iter->find( _T("[") ) && iter->npos == iter->find( _T("=") ) ) break;
			idx = iter->find( _T("=") );
			if( iter->npos != idx )
			{
				m_ProfileData.back().Data.insert( PAIR_STR_STR( iter->substr(0,idx), iter->substr(idx+1) ) );
			}
		}
	}
	catch(...)
	{
		//ini�t�@�C��������ǂݎ�莸�s
		return false;
	}
	return true;
}

/*! �t�@�C����ǂݍ���
	
	@param strFilename [in] �t�@�C����
	@param vecLine [out] ������i�[��

	@retval true  ����
	@retval false ���s

	@date 2003-10-26 D.S.Koba ReadProfile()���番��
*/
inline bool CProfile::ReadFile(
	const std::basic_string< TCHAR >& strFilename,
	std::vector< std::basic_string< TCHAR > >& vecLine
)
{
	std::basic_ifstream< TCHAR > ifs( strFilename.c_str() );
	if(!ifs.is_open()) return false;
	
	std::basic_string< TCHAR > strBuffer;
	try
	{
		for(;;)
		{
			std::getline( ifs, strBuffer );
			vecLine.push_back( strBuffer );
			if(ifs.eof()) break;
		}
	} //try
	catch(...)
	{
		//�t�@�C���̓ǂݍ��ݎ��s
		ifs.close();
		return false;
	}
	ifs.close();
	return true;
}


/*! Profile���t�@�C���֏����o��
	
	@param pszProfileName [in] �t�@�C����(NULL=�Ō�ɓǂݏ��������t�@�C��)
	@param pszComment [in] �R�����g��(NULL=�R�����g�ȗ�)

	@retval true  ����
	@retval false ���s

	@date 2003.02.12 Mr.Nak fprintf���g���悤��
	@date 2003-10-21 D.S.Koba STL�ŏ�������
*/
bool CProfile::WriteProfile(
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{
	if( NULL != pszProfileName )
	{
		m_strProfileName = pszProfileName;
	}

	std::basic_ofstream< TCHAR > ofs( m_strProfileName.c_str() );
	if(!ofs.is_open()) return false;

	// �R�����g����������
	if( NULL != pszComment )
	{
		ofs << "//" << pszComment << "\n" << std::endl;
	}
    
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		//�Z�N�V����������������
		ofs << "[" << iter->strSectionName << "]\n";
		mapenditer = iter->Data.end();
		for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
		{
			//�G���g������������
			ofs << mapiter->first << "=" << mapiter->second << "\n";
		}
		ofs << std::endl;
	}
	ofs.close();
	return true;
}

/*! Profile�̓���̃Z�N�V�����̃t�@�C���ւ̏����o��
	
	@param pszSectionName [in] �Z�N�V������
	@param pszProfileName [in] �t�@�C����
	@param pszComment [in] �R�����g��(NULL=�R�����g�ȗ�)

	@retval true  ����
	@retval false ���s

	@date 2003-10-21 D.S.Koba WriteProfile()�����ɍ쐬
*/
bool CProfile::WriteProfileSection(
	const TCHAR* pszSectionName,
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{  
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		//�ړI�̃Z�N�V�����̏ꍇ
		if( iter->strSectionName == pszSectionName )
		{
			std::basic_ofstream< TCHAR > ofs( pszProfileName );
			if(!ofs.is_open()) return false;

			// �R�����g����������
			if( NULL != pszComment )
			{
				ofs << "//" << pszComment << "\n" << std::endl;
			}
			//�Z�N�V����������������
			ofs << "[" << iter->strSectionName << "]\n";
			mapenditer = iter->Data.end();
			for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
			{
				//�G���g������������
				ofs << mapiter->first << "=" << mapiter->second << "\n";
			}
			ofs << std::endl;
			ofs.close();
			break;
		}
	}
	if( enditer == iter ) return false;
	return true;
}


/*! �G���g���l(bool�^)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param pszSectionName [in] �Z�N�V������
	@param pszEntryKey [in] �G���g����
	@param pEntryValue [i/o] �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	bool&			pEntryValue
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( strWork != _T("0") )	pEntryValue = true;
		else						pEntryValue = false;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		if( pEntryValue == true )	strNewEntryValue = _T("1");
		else						strNewEntryValue = _T("0");
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! �G���g���l(int�^)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param pszSectionName [in] �Z�N�V������
	@param pszEntryKey [in] �G���g����
	@param pEntryValue [i/o] �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	int&			pEntryValue
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		std::basic_stringstream< TCHAR > stream;
		stream << strWork;
		stream >> pEntryValue;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		std::basic_stringstream< TCHAR > stream;
		stream << pEntryValue;
		stream >> strNewEntryValue;
		//strNewEntryValue = stream.str();
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! �G���g���l(WORD�^)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param pszSectionName [in] �Z�N�V������
	@param pszEntryKey [in] �G���g����
	@param pEntryValue [i/o] �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	WORD&			pEntryValue
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		std::basic_stringstream< TCHAR > stream;
		stream << strWork;
		stream >> pEntryValue;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		std::basic_stringstream< TCHAR > stream;
		stream << pEntryValue;
		stream >> strNewEntryValue;
		//strNewEntryValue = stream.str();
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! �G���g���l(TCHAR�^)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param pszSectionName [in] �Z�N�V������
	@param pszEntryKey [in] �G���g����
	@param pEntryValue [i/o] �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR&			pEntryValue
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( 0 == strWork.length() )	pEntryValue = _T('\0');
		else						pEntryValue = strWork.at(0);
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		if( _T('\0') == pEntryValue )	strNewEntryValue = _T("");
		else							strNewEntryValue = pEntryValue;
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! �G���g���l(NULL�����I�[�̕�����)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param pszSectionName [in] �Z�N�V������
	@param pszEntryKey [in] �G���g����
	@param pEntryValue [i/o] �G���g���l
	@param nEntryValueSize [in] pEntryValue�̊m�ۃ������T�C�Y(�ǂݍ��ݎ��T�C�Y���\���ł��邱�Ƃ����m�Ȃ� 0 �w��B�������ݎ��͎g�p���Ȃ�)

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( nEntryValueSize > static_cast<int>(strWork.length())
			|| nEntryValueSize ==0 )
		{
			strWork.copy( pEntryValue, strWork.length() );
			//copy()��NULL�����I�[���Ȃ��̂�NULL�����ǉ�
			pEntryValue[strWork.length()] = _T('\0');
		}
		else
		{
			strWork.copy( pEntryValue, nEntryValueSize-1 );
			//copy()��NULL�����I�[���Ȃ��̂�NULL�����ǉ�
			pEntryValue[nEntryValueSize-1] = _T('\0');
		}
		return true;
	}
	else
	{
		return SetProfileData( pszSectionName, pszEntryKey, pEntryValue );
	}
}

/*! �G���g���l(std::basic_string<TCHAR>�^)��Profile����ǂݍ��ށC����Profile�֏�������
	
	@param bRead [in] ���[�h(true=�ǂݍ���, false=��������)
	@param strSectionName [in] �Z�N�V������
	@param strEntryKey [in] �G���g����
	@param strEntryValue [i/o] �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2003-10-22 D.S.Koba �쐬
*/
bool CProfile::IOProfileData(
	const bool&							bRead,
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	// �u�ǂݍ��݁v���u�������݁v��
	if( bRead ) return GetProfileData( strSectionName, strEntryKey, strEntryValue );
	else        return SetProfileData( strSectionName, strEntryKey, strEntryValue );
}

/*! �G���g���l��Profile����ǂݍ���
	
	@param strSectionName [in] �Z�N�V������
	@param strEntryKey [in] �G���g����
	@param strEntryValue [out] �G���g���l

	@retval true ����
	@retval false ���s

	@date 2003-10-22 D.S.Koba �쐬
*/
inline bool CProfile::GetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		if( iter->strSectionName == strSectionName )
		{
			mapiter = iter->Data.find( strEntryKey );
			if( iter->Data.end() != mapiter )
			{
				strEntryValue = mapiter->second;
				return true;
			}
		}
	}
	return false;
}

/*! �G���g����Profile�֏�������
	
	@param strSectionName [in] �Z�N�V������
	@param strEntryKey [in] �G���g����
	@param strEntryValue [in] �G���g���l

	@retval true  ����
	@retval false ���s(���������Ă��Ȃ��̂�false�͕Ԃ�Ȃ�)

	@date 2003-10-21 D.S.Koba �쐬
*/
inline bool CProfile::SetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	const std::basic_string< TCHAR >&	strEntryValue
)
{
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		if( iter->strSectionName == strSectionName )
		{
			//�����̃Z�N�V�����̏ꍇ
			mapiter = iter->Data.find( strEntryKey );
			if( iter->Data.end() != mapiter )
			{
				//�����̃G���g���̏ꍇ�͒l���㏑��
				mapiter->second = strEntryValue;
				break;
			}
			else
			{
				//�����̃G���g����������Ȃ��ꍇ�͒ǉ�
				iter->Data.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
				break;
			}
		}
	}
	//�����̃Z�N�V�����ł͂Ȃ��ꍇ�C�Z�N�V�����y�уG���g����ǉ�
	if( enditer == iter )
	{
		Section Buffer;
		Buffer.strSectionName = strSectionName;
		Buffer.Data.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
		m_ProfileData.push_back( Buffer );
	}
	return true;
}



void CProfile::DUMP( void )
{
#ifdef _DEBUG
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	MYTRACE( "\n\nCProfile::DUMP()======================" );
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		MYTRACE( "\n��strSectionName=%s", iter->strSectionName.c_str() );
		mapenditer = iter->Data.end();
		for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
		{
			MYTRACE( "\"%s\" = \"%s\"", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE( "========================================\n" );
#endif
	return;
}

/*[EOF]*/
