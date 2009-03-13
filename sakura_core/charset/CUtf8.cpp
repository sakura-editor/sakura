// 2008.11.10 �ϊ����W�b�N����������

#include "stdafx.h"
#include "CUtf8.h"
#include "charset/codechecker.h"

//! BOM�f�[�^�擾
void CUtf8::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	pcmemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}





/*!
	UTF-8 �� Unicode ����

	@param[in] bCESU8Mode CESU-8 ����������ꍇ true
*/
int CUtf8::Utf8ToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool bCESU8Mode )
{
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	int nclen;
	ECharSet echarset;

	if( nSrcLen < 1 ){
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc+nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; ; ){

		// �������`�F�b�N
		if( bCESU8Mode != true ){
			nclen = CheckUtf8Char( reinterpret_cast<const char*>(pr), pr_end-pr, &echarset, true, 0 );
		}else{
			nclen = CheckCesu8Char( reinterpret_cast<const char*>(pr), pr_end-pr, &echarset, 0 );
		}
		if( nclen < 1 ){
			break;
		}

		// �ϊ�
		if( echarset != CHARSET_BINARY ){
			pw += _Utf8ToUni_char( pr, nclen, pw, bCESU8Mode );
			pr += nclen;
		}else{
			if( nclen != 1 ){	// �ی�R�[�h
				nclen = 1;
			}
			pw += BinToText( pr, 1, pw );
			++pr;
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



//! UTF-8��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
EConvertResult CUtf8::_UTF8ToUnicode( CMemory* pMem, bool bCESU8Mode/*, bool decodeMime*/ )
{
	// �G���[���
	bool bError = false;

	// �f�[�^�擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( pMem->GetRawPtr(&nSrcLen) );
 
	const char* psrc = pSrc;
	int nsrclen = nSrcLen;

//	CMemory cmem;
//	// MIME �w�b�_�[�f�R�[�h
//	if( decodeMime == true ){
//		bool bret = MIMEHeaderDecode( pSrc, nSrcLen, &cmem, CODE_UTF8 );
//		if( bret == true ){
//			psrc = reinterpret_cast<char*>( cmem.GetRawPtr() );
//			nsrclen = cmem.GetRawLength();
//		}
//	}

	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂĊm�ۂ���
	wchar_t* pDst;
	try{
		pDst = new wchar_t[nsrclen];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = Utf8ToUni( psrc, nsrclen, pDst, bCESU8Mode );

	// pMem ���X�V
	pMem->SetRawData( pDst, nDstLen*sizeof(wchar_t) );

	// ��n��
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}








/*!
	Unicode -> UTF-8 ����

	@param[in] bCESU8Mode CESU-8 ����������ꍇ true
*/
int CUtf8::UniToUtf8( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError, bool bCESU8Mode )
{
	const unsigned short* pr = reinterpret_cast<const unsigned short*>(pSrc);
	const unsigned short* pr_end = reinterpret_cast<const unsigned short*>(pSrc+nSrcLen);
	unsigned char* pw = reinterpret_cast<unsigned char*>(pDst);
	int nclen;
	bool berror=false;
	ECharSet echarset;

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset, 0)) > 0 ){
		// �ی�R�[�h
		switch( echarset ){
		case CHARSET_UNI_NORMAL:
			nclen = 1;
			break;
		case CHARSET_UNI_SURROG:
			nclen = 2;
			break;
		default:
			echarset = CHARSET_BINARY;
			nclen = 1;
		}
		if( echarset != CHARSET_BINARY ){
			pw += _UniToUtf8_char( pr, nclen, pw, bCESU8Mode );
			pr += nclen;
		}else{
			if(  nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>( TextToBin(*pr) & 0x000000ff );
				++pw;
			}else{
				berror = true;
				*pw = '?';
				++pw;
			}
			++pr;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}


//! �R�[�h�ϊ� Unicode��UTF-8
EConvertResult CUtf8::_UnicodeToUTF8( CMemory* pMem, bool bCesu8Mode )
{
	// �G���[���
	bool bError = false;

	// �\�[�X���擾
	const wchar_t* pSrc = reinterpret_cast<wchar_t*>( pMem->GetRawPtr() );
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);


	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂă��������m��
	char* pDst;
	try{
		pDst = new char[nSrcLen * 3];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = UniToUtf8( pSrc, nSrcLen, pDst, &bError, bCesu8Mode );

	// pMem ���X�V
	pMem->SetRawData( pDst, nDstLen );

	// ��n��
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}

// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/21 Uchi
EConvertResult CUtf8::_UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const bool bCESUMode)
{
	CMemory	cBuff;
	EConvertResult	res;
	int				i;
	TCHAR*			pd;
	unsigned char*	ps;
	bool			bbinary=false;

	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUtf8Codepoint) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		cBuff.SetRawData(cSrc,4);
	}
	else {
		cBuff.SetRawData(cSrc,2);
		if( IsBinaryOnSurrogate(cSrc[0]) ){
			bbinary = true;
		}
	}

	// UTF-8/CESU-8 �ϊ�
	if (bCESUMode != true) {
		res = UnicodeToUTF8(&cBuff);
	}
	else {
		res = UnicodeToCESU8(&cBuff);
	}
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	ps = reinterpret_cast<unsigned char*>( cBuff.GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cBuff.GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, _T("%02x"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02x"), *ps );
	}

	return RESULT_COMPLETE;
}
