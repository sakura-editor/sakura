#include "StdAfx.h"
#include <ShellAPI.h>// HDROP
#include "CClipboard.h"
#include "charset/CShiftJis.h"
#include "charset/CUtf8.h"
#include "CEol.h"

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
	bool			bLineSelect
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
	while(0);	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	// �o�C�i���`���̃f�[�^
	//	(int) �u�f�[�^�v�̒���
	//	�u�f�[�^�v
	HGLOBAL hgClipSakura = NULL;
	do{
		//�T�N���G�f�B�^��p�t�H�[�}�b�g���擾
		CLIPFORMAT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
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
	while(0);	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

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

//! �e�L�X�g���擾����
bool CClipboard::GetText(CNativeW* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect)
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
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
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
	HGLOBAL hUnicode = ::GetClipboardData( CF_UNICODETEXT );
	if( hUnicode != NULL ){
		//DWORD nLen = GlobalSize(hUnicode);
		wchar_t* szData = GlobalLockWChar(hUnicode);
		cmemBuf->SetString( szData );
		::GlobalUnlock(hUnicode);
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
				DragQueryFile(hDrop, nLoop, sTmpPath, _countof(sTmpPath) - 1);
				// 2012.10.05 Moca ANSI�łɍ��킹�čŏI�s�ɂ����s�R�[�h������
				cmemBuf->AppendStringT(sTmpPath);
				if(nMaxCnt > 1){
					cmemBuf->AppendStringT(_TEXT("\r\n"));
				}
			}
			return true;
		}
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
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return CF_OEMTEXT;
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return CF_UNICODETEXT;
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return GetSakuraFormat();
	if(::IsClipboardFormatAvailable(CF_HDROP))return CF_HDROP;
	return -1;
}

