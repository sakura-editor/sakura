/*!	@file
	@brief INI�t�@�C�����o��

	@author D.S.Koba
	@date 2003-10-21 D.S.Koba �����o�֐��̖��O�ƈ��������̂܂܂ɂ��ă����o�ϐ��C�֐��̒��g����������
	@date 2004-01-10 D.S.Koba �Ԓl��BOOL����bool�֕ύX�BIOProfileData���^�ʂ̊֐��ɕ����C���������炷
	@date 2006-02-11 D.S.Koba �ǂݍ���/�����o���������łȂ��C�����o�Ŕ���
	@date 2006-02-12 D.S.Koba IOProfileData�̒��g�̓ǂݍ��݂Ə����o�����֐��ɕ�����
*/
/*
	Copyright (C) 2003, D.S.Koba
	Copyright (C) 2004, D.S.Koba, MIK, genta
	Copyright (C) 2006, D.S.Koba, ryoji

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
#include "stdafx.h"
#ifdef USE_STREAM
#  include <fstream>
#else
#  include <stdio.h>
#  include <string.h>
#endif
//#include <sstream>
#include <algorithm>
#include "CProfile.h"
#include "Debug.h"


/*! Profile��������
	
	@date 2003-10-21 D.S.Koba STL�ŏ�������
*/
void CProfile::Init( void )
{
	m_strProfileName = _T("");
	m_ProfileData.clear();
	m_bRead = true;
	return;
}

/*!
	sakura.ini��1�s����������D

	1�s�̓ǂݍ��݂��������邲�ƂɌĂ΂��D
	
	@param line [in] �ǂݍ��񂾍s
*/
void CProfile::ReadOneline(
	const std::basic_string< TCHAR >& line
)
{
	//	��s��ǂݔ�΂�
	if( line.empty() )
		return;

	//�R�����g�s��ǂ݂Ƃ΂�
	if( 0 == line.compare( 0, 2, _T("//") ))
		return;

	// �Z�N�V�����擾
	//	Jan. 29, 2004 genta compare�g�p
	if( line.compare( 0, 1, _T("[") ) == 0 
			&& line.find( _T("=") ) == line.npos
			&& line.find( _T("]") ) == ( line.size() - 1 ) ) {
		Section Buffer;
		Buffer.strSectionName = line.substr( 1, line.size() - 1 - 1 );
		m_ProfileData.push_back( Buffer );
	}
	// �G���g���擾
	else if( !m_ProfileData.empty() ) {	//�ŏ��̃Z�N�V�����ȑO�̍s�̃G���g���͖���
		std::basic_string< TCHAR >::size_type idx = line.find( _T("=") );
		if( line.npos != idx ) {
			m_ProfileData.back().mapEntries.insert( PAIR_STR_STR( line.substr(0,idx), line.substr(idx+1) ) );
		}
	}
}

/*! Profile���t�@�C������ǂݏo��
	
	@param pszProfileName [in] �t�@�C����

	@retval true  ����
	@retval false ���s

	@date 2003-10-21 D.S.Koba STL�ŏ�������
	@date 2003-10-26 D.S.Koba ReadProfile()���番��
	@date 2004-01-29 genta stream�g�p����߂�C���C�u�����g�p�ɁD
	@date 2004-01-31 genta �s�̉�͂̕���ʊ֐��ɂ���ReadFile��ReadProfile��
		
*/
bool CProfile::ReadProfile( const TCHAR* pszProfileName )
{
	m_strProfileName = pszProfileName;
#ifdef USE_STREAM
	std::basic_ifstream< TCHAR > ifs( m_strProfileName.c_str() );
	if(!ifs.is_open()) return false;
	
	std::basic_string< TCHAR > strWork;
	try {
		for(;;) {
			std::getline( ifs, strWork );
			ReadOneline( strWork );
			if(ifs.eof()) break;
		}
	} //try
	catch(...) {
		//�t�@�C���̓ǂݍ��ݎ��s
		ifs.close();
		return false;
	}
	ifs.close();
#else
	FILE* fp = _tfopen( m_strProfileName.c_str(), _T( "r" ));
	if( fp == NULL ){
		return false;
	}

	const int CHUNK_SIZE = 2048;			// �Ă��Ɓ`
	TCHAR* buf = new TCHAR[ CHUNK_SIZE ];	//	�ǂݍ��ݗp
	std::basic_string< TCHAR > bstr;		//	��Ɨp
	unsigned int offset = 0;				//	�o�͍ςݗ̈�`�F�b�N�p

	try {
		while( !feof( fp )){
			int length = fread( buf, sizeof( TCHAR ), CHUNK_SIZE, fp );
			
			//	�G���[�`�F�b�N
			if( ferror( fp )){
				delete [] buf;
				fclose( fp );
				return false;
			}
			
			bstr = bstr.substr( offset ) + std::basic_string< TCHAR >( buf, length );
			offset = 0;
			
			int pos;
			//	\n��������ԃ��[�v
			while(( pos = bstr.find( _T('\n' ), offset ) ) != bstr.npos ){
				//	���s�R�[�h�͓n���Ȃ�
				ReadOneline( bstr.substr( offset, pos - offset ) );
				offset = pos + 1;
			}

			if( feof( fp )){
				if( offset < bstr.size() ){
					//	�ŏI�s��\n�������Ă��Ƃ肠����
					ReadOneline( bstr.substr( offset ) );
				}
				break;
			}
		}
	}
	catch( ... ){
		delete [] buf;
		fclose( fp );
		return false;
	}
	delete [] buf;
	fclose( fp );
#endif

#ifdef _DEBUG
	//DUMP();
#endif
	return true;
}


/*! Profile���t�@�C���֏����o��
	
	@param pszProfileName [in] �t�@�C����(NULL=�Ō�ɓǂݏ��������t�@�C��)
	@param pszComment [in] �R�����g��(NULL=�R�����g�ȗ�)

	@retval true  ����
	@retval false ���s

	@date 2003-10-21 D.S.Koba STL�ŏ�������
	@date 2004-01-28 D.S.Koba �t�@�C���������ݕ��𕪗�
*/
bool CProfile::WriteProfile(
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{
	if( NULL != pszProfileName ) {
		m_strProfileName = pszProfileName;
	}
    
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( NULL != pszComment ) {
		vecLine.push_back( _T("//") + std::basic_string< TCHAR >( pszComment ) );
		vecLine.push_back( _T("") );
	}
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		//�Z�N�V����������������
		vecLine.push_back( _T("[") + iter->strSectionName + _T("]") );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			//�G���g������������
			vecLine.push_back( mapiter->first + _T("=") + mapiter->second );
		}
		vecLine.push_back( _T("") );
	}

	return WriteFile( m_strProfileName, vecLine );
}

/*! �t�@�C���֏�������
	
	@param strFilename [in] �t�@�C����
	@param vecLine [out] ������i�[��

	@retval true  ����
	@retval false ���s

	@date 2004-01-28 D.S.Koba WriteProfile()���番��
	@date 2004-01-29 genta stream�g�p����߂�C���C�u�����g�p�ɁD
*/
bool CProfile::WriteFile(
	const std::basic_string< TCHAR >& strFilename,
	std::vector< std::basic_string< TCHAR > >& vecLine
)
{
#ifdef USE_STREAM
	std::basic_ofstream< TCHAR > ofs( strFilename.c_str() );
	if(!ofs.is_open()) return false;

	std::vector< std::basic_string< TCHAR > >::iterator iter;
	std::vector< std::basic_string< TCHAR > >::iterator iterEnd = vecLine.end();

	try {
		for( iter = vecLine.begin(); iter != iterEnd; iter++ ) {
			ofs << iter->c_str() << _T("\n");
		}
	}
	catch(...) {
		ofs.close();
		return false;
	}
	ofs.close();

#else
	//	Jan. 29, 2004 genta standard i/o version
	FILE* fp = _tfopen( strFilename.c_str(), _T( "w" ));
	if( fp == NULL ){
		return false;
	}

	std::vector< std::basic_string< TCHAR > >::iterator iter;
	std::vector< std::basic_string< TCHAR > >::iterator iterEnd = vecLine.end();

	for( iter = vecLine.begin(); iter != iterEnd; iter++ ) {
		//	�������\0���܂ޏꍇ���l�����ăo�C�i���o��
		if( fwrite( iter->data(), sizeof( TCHAR ), iter->size(), fp ) != iter->size() ){
			fclose( fp );
			return false;
		}
		if( _fputtc( _T('\n'), fp ) == _TEOF ){
			fclose( fp );
			return false;
		}
	}
	fclose( fp );
#endif
	return true;
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, bool& bEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, bEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, bEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, int& nEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, nEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, nEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, WORD& wEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, wEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, wEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, TCHAR& chEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, chEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, chEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, pEntryValue, nEntryValueSize );
	else		return SetProfileData( pszSectionName, pszEntryKey, pEntryValue, nEntryValueSize );
}

bool CProfile::IOProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	if(m_bRead)	return GetProfileData( strSectionName, strEntryKey, strEntryValue );
	else		return SetProfileData( strSectionName, strEntryKey, strEntryValue );
}

/*! �G���g���l(bool�^)��Profile����ǂݍ���
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[out] bEntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	bool&			bEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( strWork != _T("0") )	bEntryValue = true;
	else						bEntryValue = false;
	return true;
}

/*! �G���g���l(bool�^)��Profile�֏�������
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[in] bEntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const bool&		bEntryValue
)
{
	std::basic_string< TCHAR > strNewEntryValue;
	if( bEntryValue == true )	strNewEntryValue = _T("1");
	else						strNewEntryValue = _T("0");
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! �G���g���l(int�^)��Profile����ǂݍ���
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[out] nEntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
	@date 2004-02-14 MIK _tstoi��_ttoi
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	int&			nEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	nEntryValue = _ttoi( strWork.c_str() );
	//std::basic_stringstream< TCHAR > stream;
	//stream << strWork;
	//stream >> EntryValue;
	return true;
}


/*! �G���g���l(int�^)��Profile�֏�������
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[in] EntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
	@date 2004-02-14 MIK _tstoi��_ttoi
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const int&		nEntryValue
)
{
	TCHAR szWork[32];
	_itot( nEntryValue, szWork, 10 );
	std::basic_string< TCHAR > strNewEntryValue( szWork );
	//std::basic_string< TCHAR > strNewEntryValue;
	//std::basic_stringstream< TCHAR > stream;
	//stream << EntryValue;
	//stream >> strNewEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! �G���g���l(WORD�^)��Profile����ǂݍ���
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[out] wEntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	WORD&			wEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	wEntryValue = _ttoi( strWork.c_str() );
	//std::basic_stringstream< TCHAR > stream;
	//stream << strWork;
	//stream >> EntryValue;
	return true;
}

/*! �G���g���l(WORD�^)��Profile�֏�������
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[in] EntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const WORD&		wEntryValue
)
{
	TCHAR szWork[32];
	_itot( wEntryValue, szWork, 10 );
	std::basic_string< TCHAR > strNewEntryValue( szWork );
	//std::basic_string< TCHAR > strNewEntryValue;
	//std::basic_stringstream< TCHAR > stream;
	//stream << EntryValue;
	//stream >> strNewEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! �G���g���l(TCHAR�^)��Profile����ǂݍ���
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[out] EntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR&			chEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( 0 == strWork.length() )	chEntryValue = _T('\0');
	else						chEntryValue = strWork.at(0);
	return true;
}

/*! �G���g���l(TCHAR�^)��Profile�֏�������
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[in] chEntryValue �G���g���l

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const TCHAR&	chEntryValue
)
{
	std::basic_string< TCHAR > strNewEntryValue;
	if( _T('\0') == chEntryValue )	strNewEntryValue = _T("");
	else							strNewEntryValue = chEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! �G���g���l(NULL�����I�[�̕�����)��Profile����ǂݍ���
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[out] pEntryValue �G���g���l
	@param[in] nEntryValueSize pEntryValue�̊m�ۃ������T�C�Y(�T�C�Y���\���ł��邱�Ƃ����m�Ȃ� 0 �w���)

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( nEntryValueSize > static_cast<int>(strWork.length())
			|| nEntryValueSize ==0 ) {
		strWork.copy( pEntryValue, strWork.length() );
		//copy()��NULL�����I�[���Ȃ��̂�NULL�����ǉ�
		pEntryValue[strWork.length()] = _T('\0');
	}
	else {
		strWork.copy( pEntryValue, nEntryValueSize-1 );
		//copy()��NULL�����I�[���Ȃ��̂�NULL�����ǉ�
		pEntryValue[nEntryValueSize-1] = _T('\0');
	}
	return true;
}

/*! �G���g���l(NULL�����I�[�̕�����)��Profile�֏�������
	
	@param[in] pszSectionName �Z�N�V������
	@param[in] pszEntryKey �G���g����
	@param[in] pEntryValue �G���g���l
	@param[in] nEntryValueSize pEntryValue�̊m�ۃ������T�C�Y(�g�p���Ȃ�)

	@retval true  ����
	@retval false ���s

	@date 2004-01-10 D.S.Koba �G���g���̌^�ʂɊ֐��𕪗�
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const TCHAR*	pEntryValue,
	const int&		nEntryValueSize
)
{
	return SetProfileData( pszSectionName, pszEntryKey, std::basic_string< TCHAR >(pEntryValue) );
}

/*! �G���g���l��Profile����ǂݍ���
	
	@param[in] strSectionName �Z�N�V������
	@param[in] strEntryKey �G���g����
	@param[out] strEntryValue �G���g���l

	@retval true ����
	@retval false ���s

	@date 2003-10-22 D.S.Koba �쐬
*/
bool CProfile::GetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		if( iter->strSectionName == strSectionName ) {
			mapiter = iter->mapEntries.find( strEntryKey );
			if( iter->mapEntries.end() != mapiter ) {
				strEntryValue = mapiter->second;
				return true;
			}
		}
	}
	return false;
}

/*! �G���g����Profile�֏�������
	
	@param[in] strSectionName �Z�N�V������
	@param[in] strEntryKey �G���g����
	@param[in] strEntryValue �G���g���l

	@retval true  ����
	@retval false ���s(���������Ă��Ȃ��̂�false�͕Ԃ�Ȃ�)

	@date 2003-10-21 D.S.Koba �쐬
*/
bool CProfile::SetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	const std::basic_string< TCHAR >&	strEntryValue
)
{
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		if( iter->strSectionName == strSectionName ) {
			//�����̃Z�N�V�����̏ꍇ
			mapiter = iter->mapEntries.find( strEntryKey );
			if( iter->mapEntries.end() != mapiter ) {
				//�����̃G���g���̏ꍇ�͒l���㏑��
				mapiter->second = strEntryValue;
				break;
			}
			else {
				//�����̃G���g����������Ȃ��ꍇ�͒ǉ�
				iter->mapEntries.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
				break;
			}
		}
	}
	//�����̃Z�N�V�����ł͂Ȃ��ꍇ�C�Z�N�V�����y�уG���g����ǉ�
	if( iterEnd == iter ) {
		Section Buffer;
		Buffer.strSectionName = strSectionName;
		Buffer.mapEntries.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
		m_ProfileData.push_back( Buffer );
	}
	return true;
}



void CProfile::DUMP( void )
{
#ifdef _DEBUG
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	//	2006.02.20 ryoji: MAP_STR_STR_ITER�폜���̏C���R��ɂ��R���p�C���G���[�C��
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	MYTRACE( "\n\nCProfile::DUMP()======================" );
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		MYTRACE( "\n��strSectionName=%s", iter->strSectionName.c_str() );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			MYTRACE( "\"%s\" = \"%s\"\n", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE( "========================================\n" );
#endif
	return;
}

/*[EOF]*/
