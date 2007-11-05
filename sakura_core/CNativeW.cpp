#include "stdafx.h"
#include "CNativeW.h"
#include "charcode.h"
#include "CEOL.h"
#include <mbstring.h>
#include "charset/CShiftJis.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CNativeW2::CNativeW2()
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
}

//! nDataLen�͕����P�ʁB
CNativeW2::CNativeW2( const wchar_t* pData, int nDataLen )
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,nDataLen);
}

CNativeW2::CNativeW2( const wchar_t* pData)
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,wcslen(pData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �o�b�t�@�̓��e��u��������
void CNativeW2::SetString( const wchar_t* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void CNativeW2::SetString( const wchar_t* pszData )
{
	SetString(pszData,wcslen(pszData));
}

// �o�b�t�@�̓��e��u��������
void CNativeW2::SetNativeData( const CNativeW2& pcNative )
{
	CNative::SetRawData(pcNative);
}

//! (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
void CNativeW2::AllocStringBuffer( int nDataLen )
{
	CNative::AllocBuffer(nDataLen * sizeof(wchar_t));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeW2::AppendString( const wchar_t* pszData )
{
	AppendString(pszData,wcslen(pszData));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void CNativeW2::AppendString( const wchar_t* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeW2::AppendNativeData( const CNativeW2& cmemData )
{
	AppendString(cmemData.GetStringPtr(),cmemData.GetStringLength());
}

// -- -- char����̈ڍs�p -- -- //

//! �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
void CNativeW2::SetStringOld( const char* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(char));
	CShiftJis::SJISToUnicode(this->_GetMemory());
}

//! �o�b�t�@�̓��e��u��������
void CNativeW2::SetStringOld( const char* pszData )
{
	SetStringOld(pszData,strlen(pszData));
}

void CNativeW2::AppendStringOld( const char* pData, int nDataLen )
{
	wchar_t* szTmp=mbstowcs_new(pData,nDataLen);
	AppendString(szTmp);
	delete[] szTmp;
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
void CNativeW2::AppendStringOld( const char* pszData )
{
	AppendStringOld(pszData,strlen(pszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�擾�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()�Ɠ��@�\
wchar_t CNativeW2::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}

//! �����񒷂�Ԃ��B�����P�ʁB
CLogicInt CNativeW2::GetStringLength() const
{
	return CLogicInt(CNative::GetRawLength() / sizeof(wchar_t));
}


/* ���������e�� */
bool CNativeW2::IsEqual( const CNativeW2& cmem1, const CNativeW2& cmem2 )
{
	if(&cmem1==&cmem2)return true;

	const wchar_t* psz1;
	const wchar_t* psz2;
	int nLen1;
	int nLen2;

	psz1 = cmem1.GetStringPtr( &nLen1 );
	psz2 = cmem2.GetStringPtr( &nLen2 );
	if( nLen1 == nLen2 ){
		if( 0 == wmemcmp( psz1, psz2, nLen1 ) ){
			return true;
		}
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ϊ��C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ������u��
void CNativeW2::Replace( const wchar_t* pszFrom, const wchar_t* pszTo )
{
	CNativeW2	cmemWork;
	int			nFromLen = wcslen( pszFrom );
	int			nToLen = wcslen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= GetStringLength() - nFromLen ){
		if( 0 == wmemcmp( &GetStringPtr()[nBgn], pszFrom, nFromLen ) ){
			if( 0  < nBgn - nBgnOld ){
				cmemWork.AppendString( &GetStringPtr()[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( 0  < GetStringLength() - nBgnOld ){
		cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
	}
	SetNativeData( cmemWork );
}








// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  static�C���^�[�t�F�[�X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �w�肵���ʒu�̕�����wchar_t��������Ԃ�
CLogicInt CNativeW2::GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen )
		return CLogicInt(0);

	return CLogicInt(1);
}

//! �w�肵���ʒu�̕��������p��������Ԃ�
CLayoutInt CNativeW2::GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	//������͈͊O�Ȃ� 0
	if( nIdx >= nDataLen )
		return CLayoutInt(0);

	//���p�����Ȃ� 1
	if(WCODE::isHankaku(pData[nIdx]) )
		return CLayoutInt(1);

	//�S�p�����Ȃ� 2
	else
		return CLayoutInt(2);
}


/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂� */
const wchar_t* CNativeW2::GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pNext = pDataCurrent + 1;

	if( pNext >= &pData[nDataLen] ){
		pNext = &pData[nDataLen];
	}
	return pNext;
}

/* �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���O�ɂ��镶�����o�b�t�@�̐擪�̈ʒu���z����ꍇ��pData��Ԃ��܂� */
const wchar_t* CNativeW2::GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pPrev = pDataCurrent - 1;
	if( pPrev < pData ){
		pPrev = pData;
	}
	return pPrev;
//	return ::CharPrevW2( pData, pDataCurrent );
}


//ShiftJIS�ɕϊ����ĕԂ�
const char* CNativeW2::GetStringPtrOld() const
{
	return to_achar(GetStringPtr(),GetStringLength());
}

