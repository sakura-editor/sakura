/*!	@file
	@brief �������o�b�t�@�N���X

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, Moca, genta, aroka
	Copyright (C) 2005, D.S.Koba

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

#ifndef _CMEMORY_H_
#define _CMEMORY_H_

/*! �t�@�C�������R�[�h�Z�b�g���ʎ��̐�ǂݍő�T�C�Y */
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "_main/global.h"

//! �������o�b�t�@�N���X
class CMemory
{
	//�R���X�g���N�^�E�f�X�g���N�^
public:
	CMemory();
	CMemory(const CMemory& rhs);
	CMemory(const void* pData, int nDataLenBytes);
	virtual ~CMemory();
protected:
	void _init_members();

	//�C���^�[�t�F�[�X
public:
	void AllocBuffer( int );                               //!< �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
	void SetRawData( const void* pData, int nDataLen );    //!< �o�b�t�@�̓��e��u��������
	void SetRawData( const CMemory& );                     //!< �o�b�t�@�̓��e��u��������
	void AppendRawData( const void* pData, int nDataLen ); //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendRawData( const CMemory* );                  //!< �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void Clean(){ _Empty(); }
	void Clear(){ _Empty(); }

	inline const void* GetRawPtr(int* pnLength) const;      //!< �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
	inline void* GetRawPtr(int* pnLength);                  //!< �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
	inline const void* GetRawPtr() const{ return m_pRawData; } //!< �f�[�^�ւ̃|�C���^��Ԃ�
	inline void* GetRawPtr(){ return m_pRawData; }             //!< �f�[�^�ւ̃|�C���^��Ԃ�
	int GetRawLength() const { return m_nRawLen; }                //!<�f�[�^����Ԃ��B�o�C�g�P�ʁB

	// ���Z�q
	const CMemory& operator=( const CMemory& );

	// ��r
	static int IsEqual( CMemory&, CMemory& );	/* ���������e�� */

	// �ϊ��֐�
	static void SwapHLByte( char*, const int ); // ���L�֐���static�֐���
	void SwapHLByte();			// Byte����������


protected:
	/*
	||  �����w���p�֐�
	*/
	void _Empty( void ); //!< �������Bm_pRawData��NULL�ɂȂ�B
	void _AddData( const void*, int );
public:
	void _AppendSz(const char* str);
	void _SetRawLength(int nLength);


protected:
	typedef char* PCHAR;
	PCHAR& _DebugGetPointerRef(){ return m_pRawData; } //�f�o�b�O�p�B�o�b�t�@�|�C���^�̎Q�Ƃ�Ԃ��B

private: // 2002/2/10 aroka �A�N�Z�X���ύX
	/*
	|| �����o�ϐ�
	*/
	int		m_nDataBufSize;	//�o�b�t�@�T�C�Y�B�o�C�g�P�ʁB
	char*	m_pRawData;		//�o�b�t�@
	int		m_nRawLen;		//�f�[�^�T�C�Y(m_nDataBufSize�ȓ�)�B�o�C�g�P�ʁB
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     inline�֐��̎���                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
inline const void* CMemory::GetRawPtr(int* pnLength) const //!< �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
{
	if(pnLength) *pnLength = GetRawLength();
	return m_pRawData;
}
inline void* CMemory::GetRawPtr(int* pnLength) //!< �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
{
	if(pnLength) *pnLength = GetRawLength();
	return m_pRawData;
}

///////////////////////////////////////////////////////////////////////
#endif /* _CMEMORY_H_ */



