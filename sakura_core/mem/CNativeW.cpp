#include "StdAfx.h"
#include "mem/CNativeW.h"
#include "CEol.h"
#include "charset/CShiftJis.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CNativeW::CNativeW()
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
}

CNativeW::CNativeW(const CNativeW& rhs)
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetNativeData(rhs);
}

//! nDataLen�͕����P�ʁB
CNativeW::CNativeW( const wchar_t* pData, int nDataLen )
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,nDataLen);
}

CNativeW::CNativeW( const wchar_t* pData)
: m_pDebugData((PWCHAR&)_DebugGetPointerRef())
{
	SetString(pData,wcslen(pData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �o�b�t�@�̓��e��u��������
void CNativeW::SetString( const wchar_t* pData, int nDataLen )
{
	CNative::SetRawData(pData,nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void CNativeW::SetString( const wchar_t* pszData )
{
	SetString(pszData,wcslen(pszData));
}

// �o�b�t�@�̓��e��u��������
void CNativeW::SetNativeData( const CNativeW& pcNative )
{
	CNative::SetRawData(pcNative);
}

//! (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
void CNativeW::AllocStringBuffer( int nDataLen )
{
	CNative::AllocBuffer(nDataLen * sizeof(wchar_t));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeW::AppendString( const wchar_t* pszData )
{
	AppendString(pszData,wcslen(pszData));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void CNativeW::AppendString( const wchar_t* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeW::AppendNativeData( const CNativeW& cmemData )
{
	AppendString(cmemData.GetStringPtr(),cmemData.GetStringLength());
}

// -- -- char����̈ڍs�p -- -- //

//! �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
void CNativeW::SetStringOld( const char* pData, int nDataLen )
{
	int nLen;
	wchar_t* szTmp=mbstowcs_new(pData,nDataLen,&nLen);
	SetString(szTmp,nLen);
	delete[] szTmp;
}

//! �o�b�t�@�̓��e��u��������
void CNativeW::SetStringOld( const char* pszData )
{
	SetStringOld(pszData,strlen(pszData));
}

void CNativeW::AppendStringOld( const char* pData, int nDataLen )
{
	int nLen;
	wchar_t* szTmp=mbstowcs_new(pData,nDataLen,&nLen);
	AppendString(szTmp,nLen);
	delete[] szTmp;
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
void CNativeW::AppendStringOld( const char* pszData )
{
	AppendStringOld(pszData,strlen(pszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�擾�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()�Ɠ��@�\
wchar_t CNativeW::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}

//! �����񒷂�Ԃ��B�����P�ʁB
CLogicInt CNativeW::GetStringLength() const
{
	return CLogicInt(CNative::GetRawLength() / sizeof(wchar_t));
}


/* ���������e�� */
bool CNativeW::IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 )
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
void CNativeW::Replace( const wchar_t* pszFrom, const wchar_t* pszTo )
{
	CNativeW	cmemWork;
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
CLogicInt CNativeW::GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen )
		return CLogicInt(0);

	// �T���Q�[�g�`�F�b�N					2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])) {
			// �T���Q�[�g�y�A 2��
			return CLogicInt(2);
		}
	}

	return CLogicInt(1);
}

//! �w�肵���ʒu�̕��������p��������Ԃ�
CLayoutInt CNativeW::GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx )
{
	//������͈͊O�Ȃ� 0
	if( nIdx >= nDataLen )
		return CLayoutInt(0);

	// �T���Q�[�g�`�F�b�N BMP �ȊO�͑S�p����		2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		return CLayoutInt(2);	// ��
	}
	if (IsUTF16Low(pData[nIdx])) {
		if (nIdx > 0 && IsUTF16High(pData[nIdx - 1])) {
			// �T���Q�[�g�y�A�i���ʁj
			return CLayoutInt(0);
		}
		// �P�Ɓi�u���[�N���y�A�j
		// return CLayoutInt(2);
		 if( IsBinaryOnSurrogate(pData[nIdx]) )
			return CLayoutInt(1);
		else
			return CLayoutInt(2);
	}

	//���p�����Ȃ� 1
	if(WCODE::IsHankaku(pData[nIdx]) )
		return CLayoutInt(1);

	//�S�p�����Ȃ� 2
	else
		return CLayoutInt(2);
}


/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂� */
const wchar_t* CNativeW::GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pNext = pDataCurrent + 1;

	if( pNext >= &pData[nDataLen] ){
		return &pData[nDataLen];
	}

	// �T���Q�[�g�y�A�Ή�	2008/7/6 Uchi
	if (IsUTF16High(*pDataCurrent)) {
		if (IsUTF16Low(*pNext)) {
			pNext += 1;
		}
	}

	return pNext;
}

/* �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���O�ɂ��镶�����o�b�t�@�̐擪�̈ʒu���z����ꍇ��pData��Ԃ��܂� */
const wchar_t* CNativeW::GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent )
{
	const wchar_t* pPrev = pDataCurrent - 1;
	if( pPrev <= pData ){
		return pData;
	}

	// �T���Q�[�g�y�A�Ή�	2008/7/6 Uchi
	if (IsUTF16Low(*pPrev)) {
		if (IsUTF16High(*(pPrev-1))) {
			pPrev -= 1;
		}
	}

	return pPrev;
//	return ::CharPrevW_AnyBuild( pData, pDataCurrent );
}


//ShiftJIS�ɕϊ����ĕԂ�
const char* CNativeW::GetStringPtrOld() const
{
	return to_achar(GetStringPtr(),GetStringLength());
}

