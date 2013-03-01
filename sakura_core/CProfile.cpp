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
	Copyright (C) 2009, ryoji

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
#include "StdAfx.h"
#include "CProfile.h"
#include "io/CTextStream.h"
#include "charset/CUtf8.h"		// Resource�ǂݍ��݂Ɏg�p
#include "CEol.h"
#include "util/file.h"

using namespace std;

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
	const wstring& line
)
{
	//	��s��ǂݔ�΂�
	if( line.empty() )
		return;

	//�R�����g�s��ǂ݂Ƃ΂�
	if( 0 == line.compare( 0, 2, LTEXT("//") ))
		return;

	// �Z�N�V�����擾
	//	Jan. 29, 2004 genta compare�g�p
	if( line.compare( 0, 1, LTEXT("[") ) == 0 
			&& line.find( LTEXT("=") ) == line.npos
			&& line.find( LTEXT("]") ) == ( line.size() - 1 ) ) {
		Section Buffer;
		Buffer.strSectionName = line.substr( 1, line.size() - 1 - 1 );
		m_ProfileData.push_back( Buffer );
	}
	// �G���g���擾
	else if( !m_ProfileData.empty() ) {	//�ŏ��̃Z�N�V�����ȑO�̍s�̃G���g���͖���
		wstring::size_type idx = line.find( LTEXT("=") );
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

	CTextInputStream in(m_strProfileName.c_str());
	if(!in){
		return false;
	}

	try{
		while( in ){
			//1�s�Ǎ�
			wstring line=in.ReadLineW();

			//���
			ReadOneline(line);
		}
	}
	catch( ... ){
		return false;
	}

	return true;
}


/*! Profile�����\�[�X����ǂݏo��
	
	@param pName [in] ���\�[�X��
	@param pType [in] ���\�[�X�^�C�v

	@retval true  ����
	@retval false ���s

	@date 2010/5/19 MainMenu�p�ɍ쐬

	1�s300�����܂łɐ���
*/
bool CProfile::ReadProfileRes( const TCHAR* pName, const TCHAR* pType )
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	HRSRC		hRsrc;
	HGLOBAL		hGlobal;
	size_t		nSize;
	char*		psMMres;
	char*		p;
	char		sLine[300+1];
	char*		pn;
	size_t		lnsz;
	wstring		line;

	m_strProfileName = _T("-Res-");

	if (( hRsrc = ::FindResource( 0, pName, pType )) != NULL
	 && ( hGlobal = ::LoadResource( 0, hRsrc )) != NULL
	 && ( psMMres = (char *)::LockResource(hGlobal)) != NULL
	 && ( nSize = (size_t)::SizeofResource( 0, hRsrc )) != 0) {
		p    = psMMres;
		if (nSize >= sizeof(UTF8_BOM) && memcmp( p, UTF8_BOM, sizeof(UTF8_BOM) )==0) {
			// Skip BOM
			p += sizeof(UTF8_BOM);
		}
		for (; p < psMMres + nSize ; p = pn) {
			// 1�s�؂���i���������ꍇ�؎̂āj
			pn = strpbrk(p, "\n");
			if (pn == NULL) {
				// �ŏI�s
				pn = psMMres + nSize;
			}
			else {
				pn++;
			}
			lnsz = (pn-p)<=300 ? (pn-p) : 300;
			memcpy(sLine, p, lnsz);
			sLine[lnsz] = '\0';
			if (sLine[lnsz-1] == '\n')	sLine[--lnsz] = '\0';
			if (sLine[lnsz-1] == '\r')	sLine[--lnsz] = '\0';
			
			// UTF-8 -> UNICODE
			CMemory cmLine( sLine, lnsz );
			CUtf8::UTF8ToUnicode( &cmLine );
			line = (const wchar_t*)cmLine.GetRawPtr();

			//���
			ReadOneline(line);
		}
	}
	return true;
}

/*! Profile���t�@�C���֏����o��
	
	@param pszProfileName [in] �t�@�C����(NULL=�Ō�ɓǂݏ��������t�@�C��)
	@param pszComment [in] �R�����g��(NULL=�R�����g�ȗ�)

	@retval true  ����
	@retval false ���s

	@date 2003-10-21 D.S.Koba STL�ŏ�������
	@date 2004-01-28 D.S.Koba �t�@�C���������ݕ��𕪗�
	@date 2009.06.24 ryoji �ʃt�@�C���ɏ�������ł���u�������鏈����ǉ�
*/
bool CProfile::WriteProfile(
	const TCHAR* pszProfileName,
	const WCHAR* pszComment
)
{
	if( pszProfileName!=NULL ) {
		m_strProfileName = pszProfileName;
	}
    
	std::vector< wstring > vecLine;
	if( NULL != pszComment ) {
		vecLine.push_back( LTEXT(";") + wstring( pszComment ) );		// //->;	2008/5/24 Uchi
		vecLine.push_back( LTEXT("") );
	}
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		//�Z�N�V����������������
		vecLine.push_back( LTEXT("[") + iter->strSectionName + LTEXT("]") );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			//�G���g������������
			vecLine.push_back( mapiter->first + LTEXT("=") + mapiter->second );
		}
		vecLine.push_back( LTEXT("") );
	}

	// �ʃt�@�C���ɏ�������ł���u��������i�v���Z�X�����I���Ȃǂւ̈��S�΍�j
	TCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = _T('\0');
	TCHAR szPath[_MAX_PATH];
	LPTSTR lpszName;
	DWORD nLen = ::GetFullPathName(m_strProfileName.c_str(), _countof(szPath), szPath, &lpszName);
	if( 0 < nLen && nLen < _countof(szPath)
		&& (lpszName - szPath + 11) < _countof(szMirrorFile) )	// path\preuuuu.TMP
	{
		*lpszName = _T('\0');
		::GetTempFileName(szPath, _T("sak"), 0, szMirrorFile);
	}

	if( !_WriteFile(szMirrorFile[0]? szMirrorFile: m_strProfileName, vecLine) )
		return false;

	if( szMirrorFile[0] ){
		BOOL (__stdcall *pfnReplaceFile)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);
		HMODULE hModule = ::GetModuleHandle(_T("KERNEL32"));
		pfnReplaceFile = (BOOL (__stdcall *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID))
#ifndef _UNICODE
			::GetProcAddress(hModule, "ReplaceFileA");
#else
			::GetProcAddress(hModule, "ReplaceFileW");
#endif
		if( !pfnReplaceFile || !pfnReplaceFile(m_strProfileName.c_str(), szMirrorFile, NULL, 0, NULL, NULL) ){
			if (fexist(m_strProfileName.c_str())) {
				if (!::DeleteFile(m_strProfileName.c_str())) {
					return false;
				}
			}
			if (!::MoveFile(szMirrorFile, m_strProfileName.c_str())) {
				return false;
			}
		}
	}

	return true;
}

/*! �t�@�C���֏�������
	
	@retval true  ����
	@retval false ���s

	@date 2004-01-28 D.S.Koba WriteProfile()���番��
	@date 2004-01-29 genta stream�g�p����߂�C���C�u�����g�p�ɁD
*/
bool CProfile::_WriteFile(
	const tstring&			strFilename,	//!< [in]  �t�@�C����
	const vector<wstring>&	vecLine			//!< [out] ������i�[��
)
{
	CTextOutputStream out(strFilename.c_str());
	if(!out){
		return false;
	}

	for(int i=0;i<(int)vecLine.size();i++){
		// �o��
		out.WriteString(vecLine[i].c_str());
		out.WriteString(L"\n");
	}

	out.Close();

	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                            Imp                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �G���g���l��Profile����ǂݍ���
	
	@retval true ����
	@retval false ���s

	@date 2003-10-22 D.S.Koba �쐬
*/
bool CProfile::GetProfileDataImp(
	const wstring&	strSectionName,	//!< [in] �Z�N�V������
	const wstring&	strEntryKey,	//!< [in] �G���g����
	wstring&		strEntryValue	//!< [out] �G���g���l
)
{
	wstring strWork;
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
	
	@retval true  ����
	@retval false ���s(���������Ă��Ȃ��̂�false�͕Ԃ�Ȃ�)

	@date 2003-10-21 D.S.Koba �쐬
*/
bool CProfile::SetProfileDataImp(
	const wstring&	strSectionName,	//!< [in] �Z�N�V������
	const wstring&	strEntryKey,	//!< [in] �G���g����
	const wstring&	strEntryValue	//!< [in] �G���g���l
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
	MYTRACE_A( "\n\nCProfile::DUMP()======================" );
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		MYTRACE_A( "\n��strSectionName=%ls", iter->strSectionName.c_str() );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			MYTRACE_A( "\"%ls\" = \"%ls\"\n", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE_A( "========================================\n" );
#endif
	return;
}


