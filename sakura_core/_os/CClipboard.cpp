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

#include "StdAfx.h"
#include <ShellAPI.h>// HDROP
#include "CClipboard.h"
#include "doc/CEditDoc.h"
#include "charset/CCodeMediator.h"
#include "charset/CCodeFactory.h"
#include "charset/CShiftJis.h"
#include "charset/CUtf8.h"
#include "CEol.h"

// MinGW<=4.5.0�̃R���p�C���G���[�΍�
#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CClipboard::CClipboard(HWND hwnd)
{
	m_hwnd = hwnd;
	m_bOpenResult = ::OpenClipboard(hwnd);
}

CClipboard::~CClipboard()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CClipboard::Empty()
{
	::EmptyClipboard();
}

void CClipboard::Close()
{
	if(m_bOpenResult){
		::CloseClipboard();
		m_bOpenResult=FALSE;
	}
}

bool CClipboard::SetText(
	const wchar_t*	pData,			//!< �R�s�[����UNICODE������
	int				nDataLen,		//!< pData�̒����i�����P�ʁj
	bool			bColumnSelect,
	bool			bLineSelect,
	UINT			uFormat
)
{
	if( !m_bOpenResult ){
		return false;
	}

	/*
	// �e�L�X�g�`���̃f�[�^ (CF_OEMTEXT)
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextLen + 1
	);
	if( hgClipText ){
		char* pszClip = GlobalLockChar( hgClipText );
		memcpy( pszClip, pszText, nTextLen );
		pszClip[nTextLen] = '\0';
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_OEMTEXT, hgClipText );
	}
	*/

	// UNICODE�`���̃f�[�^ (CF_UNICODETEXT)
	HGLOBAL hgClipText = NULL;
	bool bUnicodeText = (uFormat == (UINT)-1 || uFormat == CF_UNICODETEXT);
	while(bUnicodeText){
		//�̈�m��
		hgClipText = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			(nDataLen + 1) * sizeof(wchar_t)
		);
		if( !hgClipText )break;

		//�m�ۂ����̈�Ƀf�[�^���R�s�[
		wchar_t* pszClip = GlobalLockWChar( hgClipText );
		wmemcpy( pszClip, pData, nDataLen );	//�f�[�^
		pszClip[nDataLen] = L'\0';				//�I�[�k��
		::GlobalUnlock( hgClipText );

		//�N���b�v�{�[�h�ɐݒ�
		::SetClipboardData( CF_UNICODETEXT, hgClipText );
		bUnicodeText = false;
	}
	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	// �o�C�i���`���̃f�[�^
	//	(int) �u�f�[�^�v�̒���
	//	�u�f�[�^�v
	HGLOBAL hgClipSakura = NULL;
	//�T�N���G�f�B�^��p�t�H�[�}�b�g���擾
	CLIPFORMAT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
	bool bSakuraText = (uFormat == (UINT)-1 || uFormat == uFormatSakuraClip);
	while(bSakuraText){
		if( 0 == uFormatSakuraClip )break;

		//�̈�m��
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(int) + (nDataLen + 1) * sizeof(wchar_t)
		);
		if( !hgClipSakura )break;

		//�m�ۂ����̈�Ƀf�[�^���R�s�[
		BYTE* pClip = GlobalLockBYTE( hgClipSakura );
		*((int*)pClip) = nDataLen; pClip += sizeof(int);								//�f�[�^�̒���
		wmemcpy( (wchar_t*)pClip, pData, nDataLen ); pClip += nDataLen*sizeof(wchar_t);	//�f�[�^
		*((wchar_t*)pClip) = L'\0'; pClip += sizeof(wchar_t);							//�I�[�k��
		::GlobalUnlock( hgClipSakura );

		//�N���b�v�{�[�h�ɐݒ�
		::SetClipboardData( uFormatSakuraClip, hgClipSakura );
		bSakuraText = false;
	}
	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	// ��`�I���������_�~�[�f�[�^
	HGLOBAL hgClipMSDEVColumn = NULL;
	if( bColumnSelect ){
		UINT uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVColumn = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColumn ){
				BYTE* pClip = GlobalLockBYTE( hgClipMSDEVColumn );
				pClip[0] = 0;
				::GlobalUnlock( hgClipMSDEVColumn );
				::SetClipboardData( uFormat, hgClipMSDEVColumn );
			}
		}
	}

	/* �s�I���������_�~�[�f�[�^ */
	HGLOBAL hgClipMSDEVLine = NULL;		// VS2008 �ȑO�̌`��
	if( bLineSelect ){
		UINT uFormat = ::RegisterClipboardFormat( _T("MSDEVLineSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVLine = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine ){
				BYTE* pClip = (BYTE*)::GlobalLock( hgClipMSDEVLine );
				pClip[0] = 0x01;
				::GlobalUnlock( hgClipMSDEVLine );
				::SetClipboardData( uFormat, hgClipMSDEVLine );
			}
		}
	}
	HGLOBAL hgClipMSDEVLine2 = NULL;	// VS2010 �`��
	if( bLineSelect ){
		UINT uFormat = ::RegisterClipboardFormat( _T("VisualStudioEditorOperationsLineCutCopyClipboardTag") );
		if( 0 != uFormat ){
			hgClipMSDEVLine2 = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine2 ){
				BYTE* pClip = (BYTE*)::GlobalLock( hgClipMSDEVLine2 );
				pClip[0] = 0x01;	// �� ClipSpy �Œ��ׂ�ƃf�[�^�͂���Ƃ͈Ⴄ�����e�ɂ͖��֌W�ɓ������ۂ�
				::GlobalUnlock( hgClipMSDEVLine2 );
				::SetClipboardData( uFormat, hgClipMSDEVLine2 );
			}
		}
	}

	if( bColumnSelect && !hgClipMSDEVColumn ){
		return false;
	}
	if( bLineSelect && !(hgClipMSDEVLine && hgClipMSDEVLine2) ){
		return false;
	}
	if( !(hgClipText && hgClipSakura) ){
		return false;
	}
	return true;
}

bool CClipboard::SetHtmlText(const CNativeW& cmemBUf)
{
	if( !m_bOpenResult ){
		return false;
	}

	CNativeA cmemUtf8;
	CUtf8().UnicodeToCode(cmemBUf, cmemUtf8._GetMemory());

	CNativeA cmemHeader;
	char szFormat[32];
	size_t size = cmemUtf8.GetStringLength() + 134;
	cmemHeader.AppendString("Version:0.9\r\n");
	cmemHeader.AppendString("StartHTML:00000097\r\n");
	sprintf( szFormat, "EndHTML:%08d\r\n", size + 36 );
	cmemHeader.AppendString(szFormat);
	cmemHeader.AppendString("StartFragment:00000134\r\n");
	sprintf( szFormat, "EndFragment:%08d\r\n", size );
	cmemHeader.AppendString(szFormat);
	cmemHeader.AppendString("<html><body>\r\n<!--StartFragment -->\r\n");
	CNativeA cmemFooter;
	cmemFooter.AppendString("\r\n<!--EndFragment-->\r\n</body></html>\r\n");

	HGLOBAL hgClipText = NULL;
	size_t nLen = cmemHeader.GetStringLength() + cmemUtf8.GetStringLength() + cmemFooter.GetStringLength();
	//�̈�m��
	hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nLen + 1
	);
	if( !hgClipText ) return false;

	//�m�ۂ����̈�Ƀf�[�^���R�s�[
	char* pszClip = GlobalLockChar( hgClipText );
	memcpy_raw( pszClip, cmemHeader.GetStringPtr(), cmemHeader.GetStringLength() );	//�f�[�^
	memcpy_raw( pszClip + cmemHeader.GetStringLength(), cmemUtf8.GetStringPtr(), cmemUtf8.GetStringLength() );	//�f�[�^
	memcpy_raw( pszClip + cmemHeader.GetStringLength() + cmemUtf8.GetStringLength(), cmemFooter.GetStringPtr(), cmemFooter.GetStringLength() );	//�f�[�^
	pszClip[nLen] = '\0';				//�I�[�k��
	::GlobalUnlock( hgClipText );

	//�N���b�v�{�[�h�ɐݒ�
	UINT uFormat = ::RegisterClipboardFormat( _T("HTML Format") );
	::SetClipboardData( uFormat, hgClipText );
	return true;
}

/*! �e�L�X�g���擾����
	@param [out] cmemBuf �擾�����e�L�X�g�̊i�[��
	@param [in,out] pbColumnSelect ��`�I���`��
	@param [in,out] pbLineSelect �s�I���`��
	@param [in] cEol HDROP�`���̂Ƃ��̉��s�R�[�h
	@param [in] uGetFormat �N���b�v�{�[�h�`��
*/
bool CClipboard::GetText(CNativeW* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat)
{
	if( !m_bOpenResult ){
		return false;
	}
	if( NULL != pbColumnSelect ){
		*pbColumnSelect = false;
	}
	if( NULL != pbLineSelect ){
		*pbLineSelect = false;
	}

	//��`�I����s�I���̃f�[�^������Ύ擾
	if( NULL != pbColumnSelect || NULL != pbLineSelect ){
		UINT uFormat = 0;
		while( ( uFormat = ::EnumClipboardFormats( uFormat ) ) != 0 ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			TCHAR szFormatName[128];
			if( ::GetClipboardFormatName( uFormat, szFormatName, _countof(szFormatName) - 1 ) ){
				if( NULL != pbColumnSelect && 0 == lstrcmpi( _T("MSDEVColumnSelect"), szFormatName ) ){
					*pbColumnSelect = true;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmpi( _T("MSDEVLineSelect"), szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmpi( _T("VisualStudioEditorOperationsLineCutCopyClipboardTag"), szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
			}
		}
	}

	//�T�N���`���̃f�[�^������Ύ擾
	CLIPFORMAT uFormatSakuraClip = CClipboard::GetSakuraFormat();
	if( (uGetFormat == -1 || uGetFormat == uFormatSakuraClip)
		&& ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		HGLOBAL hSakura = ::GetClipboardData( uFormatSakuraClip );
		if (hSakura != NULL) {
			BYTE* pData = (BYTE*)::GlobalLock(hSakura);
			size_t nLength        = *((int*)pData);
			const wchar_t* szData = (const wchar_t*)(pData + sizeof(int));
			cmemBuf->SetString( szData, nLength );
			::GlobalUnlock(hSakura);
			return true;
		}
	}

	//UNICODE�`���̃f�[�^������Ύ擾
	// From Here 2005/05/29 novice UNICODE TEXT �Ή�������ǉ�
	HGLOBAL hUnicode = NULL;
	if( uGetFormat == -1 || uGetFormat == CF_UNICODETEXT ){
		hUnicode = ::GetClipboardData( CF_UNICODETEXT );
	}
	if( hUnicode != NULL ){
		//DWORD nLen = GlobalSize(hUnicode);
		wchar_t* szData = GlobalLockWChar(hUnicode);
		cmemBuf->SetString( szData );
		::GlobalUnlock(hUnicode);
		return true;
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT�`���̃f�[�^������Ύ擾
	HGLOBAL hText = NULL;
	if( uGetFormat == -1 || uGetFormat == CF_OEMTEXT ){
		hText = ::GetClipboardData( CF_OEMTEXT );
	}
	if( hText != NULL ){
		char* szData = GlobalLockChar(hText);
		//SJIS��UNICODE
		CMemory cmemSjis( szData, GlobalSize(hText) );
		CNativeW cmemUni;
		CShiftJis::SJISToUnicode(cmemSjis, &cmemUni);
		cmemSjis.Clean();
		// '\0'�܂ł��擾
		cmemUni._SetStringLength(auto_strlen(cmemUni.GetStringPtr()));
		cmemUni.swap(*cmemBuf);
		::GlobalUnlock(hText);
		return true;
	}

	/* 2008.09.10 bosagami �p�X�\��t���Ή� */
	//HDROP�`���̃f�[�^������Ύ擾
	if( (uGetFormat == -1 || uGetFormat == CF_HDROP)
		&& ::IsClipboardFormatAvailable(CF_HDROP) ){
		HDROP hDrop = (HDROP)::GetClipboardData(CF_HDROP);
		if(hDrop != NULL){
			TCHAR sTmpPath[_MAX_PATH + 1] = {0};
			const int nMaxCnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

			for(int nLoop = 0; nLoop < nMaxCnt; nLoop++){
				DragQueryFile(hDrop, nLoop, sTmpPath, _countof(sTmpPath) - 1);
				// 2012.10.05 Moca ANSI�łɍ��킹�čŏI�s�ɂ����s�R�[�h������
				cmemBuf->AppendStringT(sTmpPath);
				if(nMaxCnt > 1){
					cmemBuf->AppendString( cEol.GetValue2() );
				}
			}
			return true;
		}
	}

	return false;
}



struct SSystemClipFormatNames
{
	CLIPFORMAT	m_nClipFormat;
	const wchar_t* m_pszName;
};
static const SSystemClipFormatNames sClipFormatNames[] =
{
	{CF_TEXT        ,L"CF_TEXT"},
	{CF_BITMAP      ,L"CF_BITMAP"},
	{CF_METAFILEPICT,L"CF_METAFILEPICT"},
	{CF_SYLK        ,L"CF_SYLK"},
	{CF_DIF         ,L"CF_DIF"},
	{CF_TIFF        ,L"CF_TIFF"},
	{CF_OEMTEXT     ,L"CF_OEMTEXT"},
	{CF_DIB         ,L"CF_DIB"},
	{CF_PALETTE     ,L"CF_PALETTE"},
	{CF_PENDATA     ,L"CF_PENDATA"},
	{CF_RIFF        ,L"CF_RIFF"},
	{CF_WAVE        ,L"CF_WAVE"},
	{CF_UNICODETEXT ,L"CF_UNICODETEXT"},
	{CF_ENHMETAFILE ,L"CF_ENHMETAFILE"},
	{CF_HDROP       ,L"CF_HDROP"},
	{CF_LOCALE      ,L"CF_LOCALE"},
	{CF_DIBV5       ,L"CF_DIBV5"},
};

static CLIPFORMAT GetClipFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = (CLIPFORMAT)-1;
	if( pFormatName[0] == L'\0' ){
		return uFormat;
	}
	for(int i = 0; i < _countof(sClipFormatNames); i++){
		if( 0 == wcsicmp(pFormatName, sClipFormatNames[i].m_pszName) ){
			uFormat = sClipFormatNames[i].m_nClipFormat;
		}
	}
	if( uFormat == (CLIPFORMAT)-1 ){
		bool bNumber = true;
		for( int i =0; pFormatName[i]; i++ ){
			if( !WCODE::Is09(pFormatName[i]) ){
				bNumber = false;
			}
		}
		if( bNumber ){
			uFormat = _wtoi(pFormatName);
		}else{
			uFormat = ::RegisterClipboardFormat( to_tchar(pFormatName) );
		}
	}
	return uFormat;
}

bool CClipboard::IsIncludeClipboradFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( ::IsClipboardFormatAvailable(uFormat) ){
		return true;
	}
	return false;
}


static int GetEndModeByMode(int nMode, int nEndMode)
{
	if( nEndMode == -1 ){
		switch(nMode){
			case -1: nEndMode = 0; break;
			case CODE_AUTODETECT: nEndMode = 0; break;
			case CODE_SJIS: nEndMode = 1; break;
			case CODE_EUC: nEndMode = 1; break;
			case CODE_LATIN1: nEndMode = 1; break;
			case CODE_UNICODE: nEndMode = 2; break;
			case CODE_UNICODEBE: nEndMode = 2; break;
			case CODE_UTF8: nEndMode = 1; break;
			case CODE_CESU8: nEndMode = 1; break;
			case CODE_UTF7: nEndMode = 1; break;
			default: nEndMode = 0; break;
		}
	}
	return nEndMode;
}



bool CClipboard::SetClipboradByFormat(const CStringRef& cstr, const wchar_t* pFormatName, int nMode, int nEndMode)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( uFormat == (CLIPFORMAT)-1 ){
		return false;
	}
	if( nMode == -2 ){
		if( uFormat == CF_UNICODETEXT || uFormat == GetSakuraFormat() ){
			return SetText(cstr.GetPtr(), cstr.GetLength(), false, false, uFormat);
		}
		return false;
	}

	CMemory cmemBuf;
	char* pBuf = NULL;
	size_t nTextByteLen = 0;
	if( nMode == -1 ){
		// �o�C�i�����[�h U+00 - U+ff��0x00 - 0xff�Ƀ}�b�s���O
		cmemBuf.AllocBuffer(cstr.GetLength());
		cmemBuf._SetRawLength(cstr.GetLength());
		pBuf = (char*)cmemBuf.GetRawPtr();
		size_t len = cstr.GetLength();
		const wchar_t* pMem = cstr.GetPtr();
		for(size_t i = 0; i < len; i++){
			pBuf[i] = (unsigned char)pMem[i];
			if( 0xff < pMem[i] ){
				return false;
			}
		}
		nTextByteLen = len;
 	}else{
		ECodeType eMode = (ECodeType)nMode;
		if( !IsValidCodeType(eMode) ){
			return false;
		}
		if( eMode == CODE_UNICODE ){
			pBuf = (char*)cstr.GetPtr();
			nTextByteLen = cstr.GetLength() * sizeof(wchar_t);
		}else{
			CCodeBase* pCode = CCodeFactory::CreateCodeBase(eMode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode());
			if( RESULT_FAILURE == pCode->UnicodeToCode(cstr, &cmemBuf) ){
				return false;
			}
			delete pCode;
			pBuf = (char*)cmemBuf.GetRawPtr();
			nTextByteLen = cmemBuf.GetRawLength();
		}
 	}
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nulLen = 0;
	switch( nEndMode ){
	case 1: nulLen = 1; break;
	case 2: nulLen = 2; break;
	case 4: nulLen = 4; break;
	case 0: nulLen = 0; break;
	default: nulLen = 0; break;
	}
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextByteLen + nulLen
	);
	if( !hgClipText ){
		return false;
	}
	char* pszClip = GlobalLockChar( hgClipText );
	memcpy( pszClip, pBuf, nTextByteLen );
	if( nulLen ){
		memset( &pszClip[nTextByteLen], 0, nulLen );
	}
	::GlobalUnlock( hgClipText );
	::SetClipboardData( uFormat, hgClipText );

	return true;
}



static int GetLengthByMode(HGLOBAL hClipData, const BYTE* pData, int nMode, int nEndMode)
{
	size_t nMemLength = ::GlobalSize(hClipData);
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nLength;
	if( nEndMode == 1 ) {
		nLength = strnlen((const char *)pData, nMemLength);
	}else if( nEndMode == 2 ){
		nLength = wcsnlen((const wchar_t *)pData, nMemLength / 2) * 2;
	}else if( nEndMode == 4 ){
		const wchar32_t* pData32 = (const wchar32_t*)pData;
		const size_t len = nMemLength / 4;
		nLength = 0;
		while( pData32[nLength] != 0 && nLength < len ){
			nLength++;
		}
		nLength *= 4;
	}else{
		nLength = nMemLength;
	}
	return nLength;
}

/*!
	�w��̃N���b�v�{�[�h�`���Ŏ擾
	
	@param nMode -2:�ʏ�̃T�N���̏���, -1:�o�C�i�����[�h, ����ȊO:�����R�[�h
	@param nEndMode -1:�����R�[�h�Ɉˑ� 0:GlobalSize 1:strlen 2:wcslen 4:wchar32_t�̕�����
	@date 2013.06.12 Moca �V�K�쐬
*/
bool CClipboard::GetClipboradByFormat(CNativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const CEol& cEol)
{
	mem.SetString(L"");
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( uFormat == (CLIPFORMAT)-1 ){
		return false;
	}
	if( !::IsClipboardFormatAvailable(uFormat) ){
		return false;
	}
	if( nMode == -2 ){
		bool bret = false;
		if( -1 != GetDataType() ){
			bret = GetText(&mem, NULL, NULL, cEol, uFormat);
			if( !bret ){
				mem.SetString(L"");
			}
		}
		return bret;
	}
	HGLOBAL hClipData = ::GetClipboardData( uFormat );
	if( hClipData != NULL ){
		bool retVal = true;
		const BYTE* pData = (BYTE*)::GlobalLock( hClipData );
		if( pData == NULL ){
			return false;
		}

		// �����I�v�V�����̉���
		size_t nLength = GetLengthByMode(hClipData, pData, nMode, nEndMode);

		// �G���R�[�h�I�v�V����
		if( nMode == -1 ){
			// �o�C�i�����[�h�B1byte��U+00-U+ff�Ƀ}�b�s���O
			mem.AllocStringBuffer(nLength);
			mem._SetStringLength(nLength);
			wchar_t* pBuf = (wchar_t *)mem.GetStringPtr();
			for( size_t i = 0; i < nLength; i++ ){
				pBuf[i] = (unsigned char)pData[i];
			}
		}else{
			ECodeType eMode = (ECodeType)nMode;
			if( !IsValidCodeType(eMode) ){
				// �R�[�h�s����99�͎�������
				ECodeType nBomCode = CCodeMediator::DetectUnicodeBom((const char*)pData, nLength);
				if( nBomCode != CODE_NONE ){
					eMode = nBomCode;
				}else{
					const STypeConfig& type = CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute();
					CCodeMediator mediator(type.m_encoding);
					eMode = mediator.CheckKanjiCode((const char*)pData, nLength);
				}
				if( !IsValidCodeType(eMode) ){
					eMode = CODE_DEFAULT;
				}
				if( -1 == nEndMode ){
					// nLength �Đݒ�
					nLength = GetLengthByMode(hClipData, pData, eMode, nEndMode);
				}
			}
			if( eMode == CODE_UNICODE ){
				mem.SetString((wchar_t *)pData, nLength / sizeof(wchar_t));
			}else{
				CMemory cmem;
				cmem.SetRawData(pData, nLength);
				if( NULL != cmem.GetRawPtr() ){
					CCodeBase* pCode = CCodeFactory::CreateCodeBase(eMode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode());
					if( RESULT_FAILURE == pCode->CodeToUnicode(cmem, &mem) ){
						mem.SetString(L"");
						retVal = false;
					}
					delete pCode;
				}
			}
		}
		::GlobalUnlock(hClipData);
		return retVal;
	}
	return false;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  static�C���^�[�t�F�[�X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//! �N���b�v�{�[�h���ɁA�T�N���G�f�B�^�ň�����f�[�^�������true
bool CClipboard::HasValidData()
{
	//������`�����P�ł������true
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return true;
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return true;
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return true;
	/* 2008.09.10 bosagami �p�X�\��t���Ή� */
	if(::IsClipboardFormatAvailable(CF_HDROP))return true;
	return false;
}

//!< �T�N���G�f�B�^�Ǝ��̃N���b�v�{�[�h�f�[�^�`��
CLIPFORMAT CClipboard::GetSakuraFormat()
{
	/*
		2007.09.30 kobake

		UNICODE�`���ŃN���b�v�{�[�h�f�[�^��ێ�����悤�ύX�������߁A
		�ȑO�̃o�[�W�����̃N���b�v�{�[�h�f�[�^�Ƌ������Ȃ��悤��
		�t�H�[�}�b�g����ύX
	*/
	return (CLIPFORMAT)::RegisterClipboardFormat( _T("SAKURAClipW") );
}

//!< �N���b�v�{�[�h�f�[�^�`��(CF_UNICODETEXT��)�̎擾
int CClipboard::GetDataType()
{
	//������`�����P�ł������true
	// 2013.06.11 GetText�̎擾���ɕύX
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return GetSakuraFormat();
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return CF_UNICODETEXT;
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return CF_OEMTEXT;
	if(::IsClipboardFormatAvailable(CF_HDROP))return CF_HDROP;
	return -1;
}

