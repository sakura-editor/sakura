#include "stdafx.h"
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

bool CClipboard::SetText(const wchar_t* pszText,bool bColmnSelect, bool bLineSelect)
{
	UINT		uFormat;

	// �k���I�[�܂ł̒���
	int nTextLen = wcslen( pszText );

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
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		(nTextLen + 1) * sizeof(wchar_t)
	);
	if( hgClipText ){
		wchar_t* pszClip = GlobalLockWChar( hgClipText );
		wmemcpy( pszClip, pszText, nTextLen + 1 );
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_UNICODETEXT, hgClipText );
	}

	// �o�C�i���`���̃f�[�^
	//	(int) �u�f�[�^�v�̒���
	//	�u�f�[�^�v
	UINT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
	HGLOBAL	hgClipSakura = NULL;
	if( 0 != uFormatSakuraClip ){
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(int) + (nTextLen + 1) * sizeof(wchar_t)
		);
		if( hgClipSakura ){
			BYTE* pClip = GlobalLockBYTE( hgClipSakura );
			*((int*)pClip) = nTextLen;
			wmemcpy( (wchar_t*)(pClip + sizeof(int)), pszText, nTextLen + 1 );
			::GlobalUnlock( hgClipSakura );
			::SetClipboardData( uFormatSakuraClip, hgClipSakura );
		}
	}

	// ��`�I���������_�~�[�f�[�^
	HGLOBAL hgClipMSDEVColm = NULL;
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
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
		uFormat = ::RegisterClipboardFormat( _T("MSDEVLineSelect") );
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
		return FALSE;
	}
	if( bLineSelect && !hgClipMSDEVLine ){
		return FALSE;
	}
	if( !hgClipText && !hgClipSakura ){
		return FALSE;
	}
	return true;
}

//! �e�L�X�g���擾����
bool CClipboard::GetText(CNativeW2* cmemBuf, BOOL* pbColmnSelect, BOOL* pbLineSelect)
{
	if( NULL != pbColmnSelect ){
		*pbColmnSelect = FALSE;
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
			return TRUE;
		}
	}

	//UNICODE�`���̃f�[�^������Ύ擾
	// From Here 2005/05/29 novice UNICODE TEXT �Ή�������ǉ�
	HGLOBAL hUnicode = ::GetClipboardData( CF_UNICODETEXT );
	if( hUnicode != NULL ){
		wchar_t* szData = GlobalLockWChar(hUnicode);
		cmemBuf->SetString( szData );
		::GlobalUnlock(hUnicode);
		::CloseClipboard();
		return TRUE;
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT�`���̃f�[�^������Ύ擾
	HGLOBAL hText = ::GetClipboardData( CF_OEMTEXT );
	if( hText != NULL ){
		char* szData = GlobalLockChar(hText);
		//SJIS��UNICODE
		CMemory cmemSjis( szData, GlobalSize(szData) );
		CShiftJis::SJISToUnicode(&cmemSjis);
		cmemBuf->SetString( reinterpret_cast<const wchar_t*>(cmemSjis.GetRawPtr()) );
		::GlobalUnlock(hText);
		::CloseClipboard();
		return TRUE;
	}

	::CloseClipboard();
	return FALSE;
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

