/*!	@file
	@brief �R�[�h�y�[�W
	
	@author Sakura-Editor collaborators
*/
/*
	Copyright (C) 2010-2012 Moca

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
#ifndef SAKURA_CCODEPAGE_H_
#define SAKURA_CCODEPAGE_H_

#include "CCodeBase.h"
#include <vector>
#include <utility>
#include <string>
#include "CShiftJis.h"

enum EEncodingTrait
{
	ENCODING_TRAIT_ERROR, // error
	ENCODING_TRAIT_ASCII,// ASCII comportible 1byte
	ENCODING_TRAIT_UTF16LE,// UTF-16LE
	ENCODING_TRAIT_UTF16BE,// UTF-16BE
	ENCODING_TRAIT_UTF32LE,// UTF-32LE 0123
	ENCODING_TRAIT_UTF32BE,// UTF-32BE 3210
	ENCODING_TRAIT_EBCDEC_CRLF,// EBCDEC/CR+LF(+NEL)
	ENCODING_TRAIT_EBCDEC,// EBCDEC
};


/*
	�V�X�e���R�[�h�y�[�W�ɂ�镶���R�[�h�ϊ�
*/
class CCodePage : public CCodeBase{
public:
	CCodePage(int codepageEx) : m_nCodePageEx(codepageEx) { }
	
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ return CPToUnicode(cSrc, pDst, m_nCodePageEx); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ return UnicodeToCP(cSrc, pDst, m_nCodePageEx); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾
	void CCodePage::GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult CPToUnicode(const CMemory& cSrc, CNativeW* pDst, int codepageEx);		// CodePage  �� Unicode�R�[�h�ϊ� 
	static EConvertResult UnicodeToCP(const CNativeW& cSrc, CMemory* pDst, int codepageEx);		// Unicode   �� CodePage�R�[�h�ϊ�

	typedef std::vector<std::pair<int, std::wstring> > CodePageList;
	
	//GUI�p�⏕�֐�
	static CCodePage::CodePageList& GetCodePageList();
	static int GetNameNormal(LPTSTR outName, int charcodeEx);
	static int GetNameShort(LPTSTR outName, int charcodeEx);
	static int GetNameLong(LPTSTR outName, int charcodeEx);
	static int GetNameBracket(LPTSTR outName, int charcodeEx);
	static int AddComboCodePages(HWND hwnd, HWND combo, int nSelCode);
	
	//CP�⏕���
	static EEncodingTrait GetEncodingTrait(int charcodeEx);
	
protected:
	// ����
	static EConvertResult CPToUni( const char*, const int, wchar_t*, int, int&, UINT );
	static EConvertResult UniToCP( const wchar_t*, const int, char*, int, int&, UINT );
	
	int m_nCodePageEx;
	
	static BOOL CALLBACK CallBackEnumCodePages( LPCTSTR );

	static int MultiByteToWideChar2(UINT, int, const char*, int, wchar_t*, int);
	static int WideCharToMultiByte2(UINT, int, const wchar_t*, int, char*, int);
	static int S_UTF32LEToUnicode( const char*, int, wchar_t*, int );
	static int S_UTF32BEToUnicode( const char*, int, wchar_t*, int );
	static int S_UnicodeToUTF32LE( const wchar_t*, int, char*, int );
	static int S_UnicodeToUTF32BE( const wchar_t*, int, char*, int );
};

#endif // SAKURA_CCODEPAGE_H_
