#pragma once

#include "CNative.h"
#include "mem/CNativeT.h"
#include "SakuraBasis.h"

class CNativeW : public CNative{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CNativeW();
	CNativeW( const CNativeW& );
	CNativeW( const wchar_t* pData, int nDataLen ); //!< nDataLen�͕����P�ʁB
	CNativeW( const wchar_t* pData);

	//�Ǘ�
	void AllocStringBuffer( int nDataLen );                    //!< (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B

	//WCHAR
	void SetString( const wchar_t* pData, int nDataLen );      //!< �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
	void SetString( const wchar_t* pszData );                  //!< �o�b�t�@�̓��e��u��������
	void AppendString( const wchar_t* pszData );               //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendString( const wchar_t* pszData, int nLength );  //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB���������true�B�������m�ۂɎ��s������false��Ԃ��B

	//CNativeW
	void SetNativeData( const CNativeW& pcNative );            //!< �o�b�t�@�̓��e��u��������
	void AppendNativeData( const CNativeW& );                  //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����

	//���Z�q
	const CNativeW& operator+=(wchar_t wch)				{ AppendString(&wch,1);   return *this; }
	const CNativeW& operator=(wchar_t wch)				{ SetString(&wch,1);      return *this; }
	const CNativeW& operator+=(const CNativeW& rhs)		{ AppendNativeData(rhs); return *this; }
	const CNativeW& operator=(const CNativeW& rhs)		{ SetNativeData(rhs);    return *this; }
	CNativeW operator+(const CNativeW& rhs) const		{ CNativeW tmp=*this; return tmp+=rhs; }


	//�l�C�e�B�u�擾�C���^�[�t�F�[�X
	wchar_t operator[](int nIndex) const;                    //!< �C�ӈʒu�̕����擾�BnIndex�͕����P�ʁB
	CLogicInt GetStringLength() const;                       //!< �����񒷂�Ԃ��B�����P�ʁB
	const wchar_t* GetStringPtr() const
	{
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
	wchar_t* GetStringPtr()
	{
		return reinterpret_cast<wchar_t*>(GetRawPtr());
	}
	const wchar_t* GetStringPtr(int* pnLength) const //[out]pnLength�͕����P�ʁB
	{
		*pnLength=GetStringLength();
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
#ifdef USE_STRICT_INT
	const wchar_t* GetStringPtr(CLogicInt* pnLength) const //[out]pnLength�͕����P�ʁB
	{
		int n;
		const wchar_t* p=GetStringPtr(&n);
		*pnLength=CLogicInt(n);
		return p;
	}
#endif

	//����
	void _SetStringLength(int nLength)
	{
		_GetMemory()->_SetRawLength(nLength*sizeof(wchar_t));
	}
	//������1�������
	void Chop()
	{
		int n = GetStringLength();
		n-=1;
		if(n>=0){
			_SetStringLength(n);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	//! ����̕�����Ȃ�true
	static bool IsEqual( const CNativeW& cmem1, const CNativeW& cmem2 );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ϊ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


	void Replace( const wchar_t* pszFrom, const wchar_t* pszTo );   //!< ������u��


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  �^����C���^�[�t�F�[�X                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �g�p�͂ł��邾���T����̂��]�܂����B
	// �ЂƂ̓I�[�o�[�w�b�h��}����Ӗ��ŁB
	// �ЂƂ͕ϊ��ɂ��f�[�^�r����}����Ӗ��ŁB

	//ACHAR
	void SetStringOld( const char* pData, int nDataLen );    //!< �o�b�t�@�̓��e��u��������BpData��SJIS�BnDataLen�͕����P�ʁB
	void SetStringOld( const char* pszData );                //!< �o�b�t�@�̓��e��u��������BpszData��SJIS�B
	void AppendStringOld( const char* pData, int nDataLen ); //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
	void AppendStringOld( const char* pszData );             //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
	const char* GetStringPtrOld() const; //ShiftJIS�ɕϊ����ĕԂ�

	//WCHAR
	void SetStringW(const wchar_t* pszData)				{ return SetString(pszData); }
	void SetStringW(const wchar_t* pData, int nLength)		{ return SetString(pData,nLength); }
	void AppendStringW(const wchar_t* pszData)				{ return AppendString(pszData); }
	void AppendStringW(const wchar_t* pData, int nLength)	{ return AppendString(pData,nLength); }
	const wchar_t* GetStringW() const						{ return GetStringPtr(); }

	//TCHAR
#ifdef _UNICODE
	void SetStringT( const TCHAR* pData, int nDataLen )	{ return SetString(pData,nDataLen); }
	void SetStringT( const TCHAR* pszData )				{ return SetString(pszData); }
	void AppendStringT(const TCHAR* pszData)			{ return AppendString(pszData); }
	void AppendStringT(const TCHAR* pData, int nLength)	{ return AppendString(pData,nLength); }
	void AppendNativeDataT(const CNativeT& rhs)			{ return AppendNativeData(rhs); }
	const TCHAR* GetStringT() const						{ return GetStringPtr(); }
#else
	void SetStringT( const TCHAR* pData, int nDataLen )	{ return SetStringOld(pData,nDataLen); }
	void SetStringT( const TCHAR* pszData )				{ return SetStringOld(pszData); }
	void AppendStringT(const TCHAR* pszData)			{ return AppendStringOld(pszData); }
	void AppendStringT(const TCHAR* pData, int nLength)	{ return AppendStringOld(pData,nLength); }
	void AppendNativeDataT(const CNativeT& rhs)			{ return AppendStringOld(rhs.GetStringPtr(), rhs.GetStringLength()); }
	const TCHAR* GetStringT() const						{ return GetStringPtrOld(); }
#endif

private:
	typedef wchar_t* PWCHAR;
	PWCHAR& m_pDebugData; //�f�o�b�O�p�BCMemory�̓��e��wchar_t*�^�ŃE�H�b�`���邽�߂̃��m

public:
	// -- -- static�C���^�[�t�F�[�X -- -- //
	static CLogicInt GetSizeOfChar( const wchar_t* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕�����wchar_t��������Ԃ�
	static CLayoutInt GetKetaOfChar( const wchar_t* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������p��������Ԃ�
	static const wchar_t* GetCharNext( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent ); //!< �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
	static const wchar_t* GetCharPrev( const wchar_t* pData, int nDataLen, const wchar_t* pDataCurrent ); //!< �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
};


//! ������ւ̎Q�Ƃ��擾����C���^�[�t�F�[�X
class IStringRef{
public:
	virtual const wchar_t*	GetPtr()	const = 0;
	virtual int				GetLength()	const = 0;
};

//! ������ւ̎Q�Ƃ�ێ�����N���X
class CStringRef : public IStringRef{
public:
	CStringRef(const wchar_t* pData, int nDataLen) : m_pData(pData), m_nDataLen(nDataLen) { }
	const wchar_t*	GetPtr()	const{ return m_pData;    }
	int				GetLength()	const{ return m_nDataLen; }
private:
	const wchar_t*	m_pData;
	int				m_nDataLen;
};

