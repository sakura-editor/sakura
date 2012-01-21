// �eCRecent�����N���X�̃x�[�X�N���X

// �G�f�B�^�n�t�@�C������include����Ƃ��� CRecent.h ��include
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
#ifndef SAKURA_CRECENTIMP_083F499E_59DF_486A_828B_BF7A535B6893_H_
#define SAKURA_CRECENTIMP_083F499E_59DF_486A_828B_BF7A535B6893_H_

#include "recent/CRecent.h"


template < class DATA_TYPE, class RECEIVE_TYPE = const DATA_TYPE* >
class CRecentImp : public CRecent{
private:
	typedef CRecentImp<DATA_TYPE,RECEIVE_TYPE>	Me;
	typedef DATA_TYPE							DataType;
	typedef RECEIVE_TYPE						ReceiveType;

public:
	CRecentImp(){ Terminate(); }
	virtual ~CRecentImp(){ Terminate(); }

protected:
	//����
	bool Create(
		DataType*		pszItemArray,	//!< �A�C�e���z��ւ̃|�C���^
		int*			pnItemCount,	//!< �A�C�e�����ւ̃|�C���^
		bool*			pbItemFavorite,	//!< ���C�ɓ���ւ̃|�C���^(NULL����)
		int				nArrayCount,	//!< �ő�Ǘ��\�ȃA�C�e����
		int*			pnViewCount		//!< �\����(NULL����)
	);
public:
	void Terminate();
	bool IsAvailable() const;
	void _Recovery();

	//�X�V
	bool ChangeViewCount( int nViewCount );	//�\�����̕ύX
	bool UpdateView();

	//�v���p�e�B�擾�n
	int GetArrayCount() const { return m_nArrayCount; }	//�ő�v�f��
	int GetItemCount() const { return ( IsAvailable() ? *m_pnUserItemCount : 0); }	//�o�^�A�C�e����
	int GetViewCount() const { return ( IsAvailable() ? (m_pnUserViewCount ? *m_pnUserViewCount : m_nArrayCount) : 0); }	//�\����

	//���C�ɓ��萧��n
	bool SetFavorite( int nIndex, bool bFavorite = true);	//���C�ɓ���ɐݒ�
	bool ResetFavorite( int nIndex ) { return SetFavorite( nIndex, false ); }	//���C�ɓ��������
	void ResetAllFavorite();			//���C�ɓ�������ׂĉ���
	bool IsFavorite( int nIndex ) const;			//���C�ɓ��肩���ׂ�

	//�A�C�e������
	bool AppendItem( ReceiveType pItemData );	//�A�C�e����擪�ɒǉ�
	bool DeleteItem( int nIndex );				//�A�C�e�����N���A
	bool DeleteItem( ReceiveType pItemData )
	{
		return DeleteItem( FindItem( pItemData ) );
	}
	bool DeleteItemsNoFavorite();			//���C�ɓ���ȊO�̃A�C�e�����N���A
	void DeleteAllItem();					//�A�C�e�������ׂăN���A

	//�A�C�e���擾
	const DataType* GetItem( int nIndex ) const;
	DataType* GetItem( int nIndex ){ return const_cast<DataType*>(static_cast<const Me*>(this)->GetItem(nIndex)); }
	int FindItem( ReceiveType pItemData ) const;
	bool MoveItem( int nSrcIndex, int nDstIndex );	//�A�C�e�����ړ�


	//�I�[�o�[���C�h�p�C���^�[�t�F�[�X
	virtual int  CompareItem( const DataType* p1, ReceiveType p2 ) const = 0;
	virtual void CopyItem( DataType* dst, ReceiveType src ) const = 0;


	//�����⏕
private:
	const DataType* GetItemPointer(int nIndex) const;
	DataType* GetItemPointer(int nIndex){ return const_cast<DataType*>(static_cast<const Me*>(this)->GetItemPointer(nIndex)); }
	void   ZeroItem( int nIndex );	//�A�C�e�����[���N���A����
	int    GetOldestItem( int nIndex, bool bFavorite );	//�ŌẪA�C�e����T��
	bool   CopyItem( int nSrcIndex, int nDstIndex );

protected:
	//�����t���O
	bool		m_bCreate;				//!< Create�ς݂�

	//�O���Q��
	DataType*	m_puUserItemData;		//!< �A�C�e���z��ւ̃|�C���^
	int*		m_pnUserItemCount;		//!< �A�C�e�����ւ̃|�C���^
	bool*		m_pbUserItemFavorite;	//!< ���C�ɓ���ւ̃|�C���^ (NULL����)
	int			m_nArrayCount;			//!< �ő�Ǘ��\�ȃA�C�e����
	int*		m_pnUserViewCount;		//!< �\���� (NULL����)
};



#include "recent/CRecentFile.h"
#include "CRecentFolder.h"
#include "CRecentSearch.h"
#include "CRecentReplace.h"
#include "CRecentGrepFile.h"
#include "CRecentGrepFolder.h"
#include "CRecentCmd.h"
#include "CRecentEditNode.h"
#include "CRecentTagjumpKeyword.h"

#endif /* SAKURA_CRECENTIMP_083F499E_59DF_486A_828B_BF7A535B6893_H_ */
/*[EOF]*/
