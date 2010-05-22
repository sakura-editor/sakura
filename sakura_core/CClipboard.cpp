#include "stdafx.h"
#include <shellapi.h>// HDROP
#include "CClipboard.h"
#include "charset/CShiftJis.h"

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
	bool			bColmnSelect,
	bool			bLineSelect
)
{
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
	do{
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
	}
	while(0);

	// �o�C�i���`���̃f�[�^
	//	(int) �u�f�[�^�v�̒���
	//	�u�f�[�^�v
	HGLOBAL	hgClipSakura = NULL;
	do{
		//�T�N���G�f�B�^��p�t�H�[�}�b�g���擾
		UINT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
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
	}
	while(0);

	// ��`�I���������_�~�[�f�[�^
	HGLOBAL hgClipMSDEVColm = NULL;
	if( bColmnSelect ){
		UINT uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVColm = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColm ){
				BYTE* pClip = GlobalLockBYTE( hgClipMSDEVColm );
				pClip[0] = 0;
				::GlobalUnlock( hgClipMSDEVColm );
				::SetClipboardData( uFormat, hgClipMSDEVColm );
			}
		}
	}

	/* �s�I���������_�~�[�f�[�^ */
	HGLOBAL		hgClipMSDEVLine = NULL;
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

	if( bColmnSelect && !hgClipMSDEVColm ){
		return false;
	}
	if( bLineSelect && !hgClipMSDEVLine ){
		return false;
	}
	if( !hgClipText && !hgClipSakura ){
		return false;
	}
	return true;
}

//! �e�L�X�g���擾����
bool CClipboard::GetText(CNativeW* cmemBuf, bool* pbColmnSelect, bool* pbLineSelect)
{
	if( NULL != pbColmnSelect ){
		*pbColmnSelect = false;
	}

	//��`�I���̃f�[�^������Ύ擾
	if( NULL != pbColmnSelect ){
		// ��`�I���̃e�L�X�g�f�[�^���N���b�v�{�[�h�ɂ��邩
		UINT uFormat = 0;
		while( uFormat = ::EnumClipboardFormats( uFormat ) ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			TCHAR szFormatName[128];
			if( ::GetClipboardFormatName( uFormat, szFormatName, _countof(szFormatName) - 1 ) ){
				if( NULL != pbColmnSelect && 0 == lstrcmp( _T("MSDEVColumnSelect"), szFormatName ) ){
					*pbColmnSelect = TRUE;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmp( _T("MSDEVLineSelect"), szFormatName ) ){
					*pbLineSelect = TRUE;
					break;
				}
			}
		}
	}
	
	//�T�N���`���̃f�[�^������Ύ擾
	UINT uFormatSakuraClip = CClipboard::GetSakuraFormat();
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		HGLOBAL hSakura = ::GetClipboardData( uFormatSakuraClip );
		if (hSakura != NULL) {
			BYTE* pData = (BYTE*)::GlobalLock(hSakura);
			size_t nLength        = *((int*)pData);
			const wchar_t* szData = (const wchar_t*)(pData + sizeof(int));
			cmemBuf->SetString( szData, nLength );
			::GlobalUnlock(hSakura);
			::CloseClipboard();
			return true;
		}
	}

	//UNICODE�`���̃f�[�^������Ύ擾
	// From Here 2005/05/29 novice UNICODE TEXT �Ή�������ǉ�
	HGLOBAL hUnicode = ::GetClipboardData( CF_UNICODETEXT );
	if( hUnicode != NULL ){
		//DWORD nLen = GlobalSize(hUnicode);
		wchar_t* szData = GlobalLockWChar(hUnicode);
		cmemBuf->SetString( szData );
		::GlobalUnlock(hUnicode);
		::CloseClipboard();
		return true;
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT�`���̃f�[�^������Ύ擾
	HGLOBAL hText = ::GetClipboardData( CF_OEMTEXT );
	if( hText != NULL ){
		char* szData = GlobalLockChar(hText);
		//SJIS��UNICODE
		CMemory cmemSjis( szData, GlobalSize(hText) );
		CShiftJis::SJISToUnicode(&cmemSjis);
		cmemBuf->SetString( reinterpret_cast<const wchar_t*>(cmemSjis.GetRawPtr()) );
		::GlobalUnlock(hText);
		::CloseClipboard();
		return true;
	}

	/* 2008.09.10 bosagami �p�X�\��t���Ή� */
	//HDROP�`���̃f�[�^������Ύ擾
	if(::IsClipboardFormatAvailable(CF_HDROP)){
		HDROP hDrop = (HDROP)::GetClipboardData(CF_HDROP);
		if(hDrop != NULL){
			TCHAR sTmpPath[_MAX_PATH + 1] = {0};
			const int nMaxCnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

			for(int nLoop = 0; nLoop < nMaxCnt; nLoop++){
				DragQueryFile(hDrop, nLoop, sTmpPath, sizeof(sTmpPath) - 1);
				if(nLoop > 0){
					cmemBuf->AppendStringT(_TEXT("\r\n"));
				}
				cmemBuf->AppendStringT(sTmpPath);
			}
			::CloseClipboard();
			return true;
		}
	}
	
	::CloseClipboard();
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
UINT CClipboard::GetSakuraFormat()
{
	/*
		2007.09.30 kobake

		UNICODE�`���ŃN���b�v�{�[�h�f�[�^��ێ�����悤�ύX�������߁A
		�ȑO�̃o�[�W�����̃N���b�v�{�[�h�f�[�^�Ƌ������Ȃ��悤��
		�t�H�[�}�b�g����ύX
	*/
	return ::RegisterClipboardFormat( _T("SAKURAClipW") );
}

//!< �N���b�v�{�[�h�f�[�^�`��(CF_UNICODETEXT��)�̎擾
int CClipboard::GetDataType()
{
	//������`�����P�ł������true
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return CF_OEMTEXT;
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return CF_UNICODETEXT;
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return GetSakuraFormat();
	if(::IsClipboardFormatAvailable(CF_HDROP))return CF_HDROP;
	return -1;
}

