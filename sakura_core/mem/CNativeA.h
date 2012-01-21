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
#ifndef SAKURA_CNATIVEA_B88E7301_8CD3_4DF8_8750_2FF92F357FA09_H_
#define SAKURA_CNATIVEA_B88E7301_8CD3_4DF8_8750_2FF92F357FA09_H_

#include "CNative.h"

class CNativeA : public CNative{
public:
	CNativeA();
	CNativeA(const CNativeA& rhs);
	CNativeA(const char* szData);
	CNativeA(const char* pData, int nLength);

	//�l�C�e�B�u�ݒ�
	void SetString( const char* pszData );                  //!< �o�b�t�@�̓��e��u��������
	void SetString( const char* pData, int nDataLen );      //!< �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
	void SetNativeData( const CNativeA& pcNative );         //!< �o�b�t�@�̓��e��u��������
	void AppendString( const char* pszData );               //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendString( const char* pszData, int nLength );  //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
	void AppendNativeData( const CNativeA& pcNative );      //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AllocStringBuffer( int nDataLen );            //!< (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B

	//�l�C�e�B�u�擾
	int GetStringLength() const;
	char operator[](int nIndex) const;                 //!< �C�ӈʒu�̕����擾�BnIndex�͕����P�ʁB
	const char* GetStringPtr() const
	{
		return reinterpret_cast<const char*>(GetRawPtr());
	}
	char* GetStringPtr()
	{
		return reinterpret_cast<char*>(GetRawPtr());
	}
	const char* GetStringPtr(int* pnLength) const; //[out]pnLength�͕����P�ʁB

	//���Z�q
	const CNativeA& operator=( char );
	const CNativeA& operator+=( char );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ϊ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�l�C�e�B�u�ϊ�
	void Replace( const char* pszFrom, const char* pszTo );   //!< ������u��
	void Replace_j( const char* pszFrom, const char* pszTo ); //!< ������u���i���{��l���Łj

	//��ʊ֐�
	void ToLower(); // ��������
	void ToUpper(); // ���啶��

	void ToZenkaku( int, int );  // ���p���S�p

	void TABToSPACE( int ); // TAB����
	void SPACEToTAB( int ); // �󔒁�TAB  //---- Stonee, 2001/05/27


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  �^����C���^�[�t�F�[�X                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �g�p�͂ł��邾���T����̂��]�܂����B
	// �ЂƂ̓I�[�o�[�w�b�h��}����Ӗ��ŁB
	// �ЂƂ͕ϊ��ɂ��f�[�^�r����}����Ӗ��ŁB

	//WCHAR
	void SetStringNew(const wchar_t* wszData, int nDataLen);
	void SetStringNew(const wchar_t* wszData);
	void AppendStringNew( const wchar_t* pszData );               //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendStringNew( const wchar_t* pszData, int nDataLen ); //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnDataLen�͕����P�ʁB
	void SetStringW(const wchar_t* pszData)				{ return SetStringNew(pszData); }
	void SetStringW(const wchar_t* pData, int nLength)		{ return SetStringNew(pData,nLength); }
	void AppendStringW(const wchar_t* pszData)				{ return AppendStringNew(pszData); }
	void AppendStringW(const wchar_t* pData, int nLength)	{ return AppendStringNew(pData,nLength); }
	const wchar_t* GetStringW() const;

	//TCHAR
#ifdef _UNICODE
	void SetStringT( const TCHAR* pszData )				{ return SetStringNew(pszData); }
	void SetStringT( const TCHAR* pData, int nLength )	{ return SetStringNew(pData,nLength); }
#else
	void SetStringT( const TCHAR* pszData )				{ return SetString(pszData); }
	void SetStringT( const TCHAR* pData, int nLength )	{ return SetString(pData,nLength); }
#endif

public:
	// -- -- static�C���^�[�t�F�[�X -- -- //
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�
	static const char* GetCharNext( const char* pData, int nDataLen, const char* pDataCurrent ); //!< �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
	static const char* GetCharPrev( const char* pData, int nDataLen, const char* pDataCurrent ); //!< �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
};

#endif /* SAKURA_CNATIVEA_B88E7301_8CD3_4DF8_8750_2FF92F357FA09_H_ */
/*[EOF]*/
