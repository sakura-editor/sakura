/*!	@file
	@brief �ŋߎg�������X�g

	���C�ɓ�����܂ލŋߎg�������X�g���Ǘ�����B

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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

#ifndef	_CRECENT_H_
#define	_CRECENT_H_

#include "global.h"



typedef enum {
	RECENT_CMP_STRCMP   = 0,
	RECENT_CMP_STRICMP  = 1,
	RECENT_CMP_STRNCMP  = 2,
	RECENT_CMP_STRNICMP = 3,
	RECENT_CMP_MEMCMP   = 4,
	RECENT_CMP_MEMICMP  = 5
} enumRecentCmp;

typedef enum {
	RECENT_FOR_FILE        = 0,
	RECENT_FOR_FOLDER      = 1,
	RECENT_FOR_SEARCH      = 2,
	RECENT_FOR_REPLACE     = 3,
	RECENT_FOR_GREP_FILE   = 4,
	RECENT_FOR_GREP_FOLDER = 5,
	RECENT_FOR_CMD         = 6,
	RECENT_FOR_EDITNODE    = 7,	//�E�C���h�E���X�g	@@@ 2003.05.31 MIK
	RECENT_FOR_TAGJUMP_KEYWORD = 8	//�^�O�W�����v�L�[���[�h @@@ 2005.04.03 MIK
} enumRecentFor;	//�^���ǉ�	//@@@ 2003.05.12 MIK



class SAKURA_CORE_API CRecent {

public:
	CRecent();
	~CRecent();

	bool IsAvailable( void );

	//��������
	bool Create( 
			char	*pszItemArray,	//�A�C�e���z��ւ̃|�C���^
			int		*pnItemCount,	//�A�C�e�����ւ̃|�C���^
			bool	*pbItemFavorite,	//���C�ɓ���ւ̃|�C���^
			int		nArrayCount, 
			int		*nViewCount, 
			int		nItemSize, 
			int		nOffset, 
			int		nCmpSize, 
			int		nCmpType 
		);

	bool EasyCreate( int nRecentType );

	void Terminate( void );

	bool ChangeViewCount( int nViewCount );	//�\�����̕ύX
	bool UpdateView( void );

	//�A�C�e������n
	bool AppendItem( const char *pszData );	//�A�C�e����擪�ɒǉ�
	const char *GetItem( int nIndex );		//�A�C�e�����擾
	const char *GetDataOfItem( int nIndex );	//�A�C�e���̔�r�v�f���擾
	bool DeleteItem( int nIndex );			//�A�C�e�����N���A
	bool DeleteItem( const char *pszItemData ) { return DeleteItem( FindItem( pszItemData ) ); }
	void DeleteAllItem( void );				//�A�C�e�������ׂăN���A
	int FindItem( const char *pszItemData );
	bool MoveItem( int nSrcIndex, int nDstIndex );	//�A�C�e�����ړ�

	//���C�ɓ��萧��n
	bool SetFavorite( int nIndex, bool bFavorite );	//���C�ɓ���ɐݒ�
	bool SetFavorite( int nIndex ) { return SetFavorite( nIndex, true ); }	//���C�ɓ���ɐݒ�
	bool ResetFavorite( int nIndex ) { return SetFavorite( nIndex, false ); }	//���C�ɓ��������
	void ResetAllFavorite( void );			//���C�ɓ�������ׂĉ���
	bool IsFavorite( int nIndex );			//���C�ɓ��肩���ׂ�

	//�v���p�e�B�擾�n
	int GetArrayCount( void ) { return m_nArrayCount; }	//�ő�v�f��
	int GetItemCount( void ) { return ( IsAvailable() ? *m_pnUserItemCount : 0); }	//�o�^�A�C�e����
	int GetViewCount( void ) { return ( IsAvailable() ? (m_pnUserViewCount ? *m_pnUserViewCount : m_nArrayCount) : 0); }	//�\����
	//int GetItemSize( void ) { return m_nItemSize; }		//�A�C�e���T�C�Y
	//int GetCmpType( void ) { return m_nCmpType; }		//��r�^�C�v
	int GetOffset( void ) { return m_nOffset; }		//��r�ʒu
	//int GetCmpSize( void ) { return m_nCmpSize; }		//��r�T�C�Y


protected:
	//	���L�������A�N�Z�X�p�B
	struct DLLSHAREDATA*	m_pShareData;		//	���L���������Q�Ƃ����B


private:
	bool	m_bCreate;		//Create�ς݂�

	int		m_nArrayCount;	//�z���
	//int		m_nItemCount;	//�L����(���ۂɃf�[�^�������Ă����) �� *m_pnUserItemCount
	//int		m_nViewCount;	//�\����
	int		m_nItemSize;	//�f�[�^�T�C�Y
	int		m_nCmpType;		//��r�^�C�v(0=strcmp,1=stricmp)
	int		m_nOffset;		//��r�ʒu
	int		m_nCmpSize;		//��r�T�C�Y

	char	*m_puUserItemData;	//�A�C�e���f�[�^
	int		*m_pnUserItemCount;
	bool	*m_pbUserItemFavorite;
	int		*m_pnUserViewCount;

	void ZeroItem( int nIndex );	//�A�C�e�����[���N���A����
	int GetOldestItem( int nIndex, bool bFavorite );	//�ŌẪA�C�e����T��
	char *GetArrayOffset( int nIndex ) { return m_puUserItemData + (nIndex * m_nItemSize); }
	bool CopyItem( int nSrcIndex, int nDstIndex );
};

#endif	//_CRECENT_H_

