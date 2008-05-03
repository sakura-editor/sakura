#include "stdafx.h"
#include "CCodeMediator.h"
#include "charset/charcode.h"
#include "charset/CESI.h"
#include "io/CBinaryStream.h"

/*
|| �t�@�C���̓��{��R�[�h�Z�b�g����
||
|| �y�߂�l�z2007.08.14 kobake �߂�l��int����ECodeType�֕ύX
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
||	�G���[		CODE_ERROR
*/
ECodeType CCodeMediator::CheckKanjiCodeOfFile( const TCHAR* pszFile )
{
	// �I�[�v��
	CBinaryInputStream in(pszFile);
	if(!in){
		return CODE_ERROR;
	}

	// �f�[�^���擾
	int nBufLen = in.GetLength();
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}

	// 0�o�C�g�Ȃ�SJIS����
	if( 0 == nBufLen ){
		return CODE_SJIS;
	}

	// �f�[�^�m��
	CMemory cMem;
	cMem.AllocBuffer(nBufLen);
	void* pBuf = cMem.GetRawPtr();

	// �ǂݍ���
	nBufLen = in.Read(pBuf, nBufLen);

	// �N���[�Y
	in.Close();

	// ���{��R�[�h�Z�b�g����
	ECodeType nCodeType = CESI::DetectUnicodeBom( reinterpret_cast<const char*>(pBuf), nBufLen );
	if( nCodeType == CODE_NONE ){
		// Unicode BOM �͌��o����܂���ł����D
		nCodeType = CheckKanjiCode( reinterpret_cast<const uchar_t*>(pBuf), nBufLen );
	}

	return nCodeType;
}


/*
|| ���{��R�[�h�Z�b�g����
||
|| �y�߂�l�z2007.08.14 kobake �߂�l��int����ECodeType�֕ύX
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
*/

// 2006.12.16  rastiv   �A���S���Y��������D
ECodeType CCodeMediator::CheckKanjiCode( const unsigned char* pBuf, int nBufLen )
{
	CESI cesi;
	WCCODE_INFO wci;
	MBCODE_INFO mbci;
	int nPt;	// 
	
	if( !cesi.ScanEncoding(reinterpret_cast<const char*>(pBuf), nBufLen) ){
		// �X�L�����Ɏ��s���܂����D
		return CODE_SJIS;  // ���f�t�H���g�����R�[�h��ԋp�D
	}
	
	nPt = cesi.DetectUnicode( &wci );
	if( 0 != nPt ){
		// UNICODE �����o����܂���.
		return wci.eCodeID;
	}
	nPt = cesi.DetectMultibyte( &mbci );
	//nPt := ���L�o�C�g�� �| �s���o�C�g��
	if( 0 < nPt ){
		return mbci.eCodeID;
	}

	return CODE_SJIS;  // ���f�t�H���g�����R�[�h��ԋp�D
}
