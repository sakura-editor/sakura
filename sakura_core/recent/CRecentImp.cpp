#include "stdafx.h"
#include "CRecentImp.h"
#include "CShareData.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	������������

	@note
	nCmpType = strcmp, stricmp �̂Ƃ��� nCmpSize = 0 ���w�肷��ƁAAppendItem 
	�ł̃f�[�^��������ł���ƔF������ strcpy ������B
	���̏ꍇ�� memcpy �� nItemSize �����R�s�[����B
	
	pnViewCount = NULL �ɂ���ƁA�[���I�� nViewCount == nArrayCount �ɂȂ�B
*/
template <class T, class S>
bool CRecentImp<T, S>::Create( 
	DataType*		pszItemArray,	//!< �A�C�e���z��ւ̃|�C���^
	int*			pnItemCount,	//!< �A�C�e�����ւ̃|�C���^
	bool*			pbItemFavorite,	//!< ���C�ɓ���ւ̃|�C���^(NULL����)
	int				nArrayCount,	//!< �ő�Ǘ��\�ȃA�C�e����
	int*			pnViewCount		//!< �\����(NULL����)
)
{
	Terminate();

	//�p�����[�^�`�F�b�N
	if( NULL == pszItemArray ) return false;
	if( NULL == pnItemCount ) return false;
	if( nArrayCount <= 0 ) return false;
	if( pnViewCount && (*pnViewCount < 0 || nArrayCount < *pnViewCount) ) return false;

	//�e�p�����[�^�i�[
	m_puUserItemData		= pszItemArray;
	m_pnUserItemCount		= pnItemCount;
	m_pbUserItemFavorite	= pbItemFavorite;
	m_nArrayCount			= nArrayCount;
	m_pnUserViewCount		= pnViewCount;
	m_bCreate = true;

	//�ʂɑ��삳��Ă����Ƃ��̂��߂̑Ή�
	UpdateView();

	return true;
}

/*
	�I������
*/
template <class T, class S>
void CRecentImp<T, S>::Terminate()
{
	m_bCreate = false;

	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;

	m_nArrayCount  = 0;
}


/*
	�������ς݂����ׂ�B
*/
template <class T, class S>
bool CRecentImp<T, S>::IsAvailable() const
{
	if(!m_bCreate)return false;

	//�f�[�^�j�󎞂̃��J�o��������Ă݂��肷��
	const_cast<CRecentImp*>(this)->_Recovery(); 

	return true;
}

//! ���J�o��
template <class T, class S>
void CRecentImp<T, S>::_Recovery()
{
	if( *m_pnUserItemCount < 0             ) *m_pnUserItemCount = 0;
	if( *m_pnUserItemCount > m_nArrayCount ) *m_pnUserItemCount = m_nArrayCount;

	if( m_pnUserViewCount )
	{
		if( *m_pnUserViewCount < 0             ) *m_pnUserViewCount = 0;
		if( *m_pnUserViewCount > m_nArrayCount ) *m_pnUserViewCount = m_nArrayCount;
	}
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���C�ɓ���                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	���C�ɓ����Ԃ�ݒ肷��B

	true	�ݒ�
	false	����
*/
template <class T, class S>
bool CRecentImp<T, S>::SetFavorite( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	m_pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	���ׂĂ̂��C�ɓ����Ԃ���������B
*/
template <class T, class S>
void CRecentImp<T, S>::ResetAllFavorite()
{
	if( ! IsAvailable() ) return;

	for( int i = 0; i < *m_pnUserItemCount; i++ )
	{
		SetFavorite( i, false );
	}
}

/*
	���C�ɓ����Ԃ��ǂ������ׂ�B

	true	���C�ɓ���
	false	�ʏ�
*/
template <class T, class S>
bool CRecentImp<T, S>::IsFavorite( int nIndex ) const
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	return m_pbUserItemFavorite[nIndex];
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�C�e������                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e����擪�ɒǉ�����B

	@note	���łɓo�^�ς݂̏ꍇ�͐擪�Ɉړ�����B
	@note	�����ς��̂Ƃ��͍ŌẪA�C�e�����폜����B
	@note	���C�ɓ���͍폜����Ȃ��B
*/
template <class T, class S>
bool CRecentImp<T, S>::AppendItem( ReceiveType pItemData )
{
	int		i;

	if( !IsAvailable() ) return false;
	if( !pItemData ) return false;

	//�o�^�ς݂����ׂ�B
	int	nIndex = FindItem( pItemData );
	if( nIndex >= 0 )
	{
		CopyItem( GetItemPointer(nIndex), pItemData );

		//�擪�Ɏ����Ă���B
		MoveItem( nIndex, 0 );
		goto reconfigure;
	}

	//�����ς��̂Ƃ��͍ŌÂ̒ʏ�A�C�e�����폜����B
	if( m_nArrayCount <= *m_pnUserItemCount )
	{
		nIndex = GetOldestItem( *m_pnUserItemCount - 1, false );
		if( -1 == nIndex )
		{
			return false;
		}

		DeleteItem( nIndex );
	}

	for( i = *m_pnUserItemCount; i > 0; i-- )
	{
		CopyItem( i - 1, i );
	}

	CopyItem( GetItemPointer(0), pItemData );

	//(void)SetFavorite( 0, true );
	//�����������Ȃ��Ƃ��߁B
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[0] = false;

	*m_pnUserItemCount += 1;


reconfigure:
	//���C�ɓ����\�����Ɉړ�����B
	if( m_pnUserViewCount )
	{
		ChangeViewCount( *m_pnUserViewCount );
	}
	return true;
}


/*
	�A�C�e�����[���N���A����B
*/
template <class T, class S>
void CRecentImp<T, S>::ZeroItem( int nIndex )
{
	if( ! IsAvailable() ) return;
	if( nIndex < 0 || nIndex >= m_nArrayCount ) return;

	memset_raw( GetItemPointer( nIndex ), 0, sizeof(DataType) );

	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nIndex] = false;

	return;
}

/*
	�A�C�e�����폜����B
*/
template <class T, class S>
bool CRecentImp<T, S>::DeleteItem( int nIndex )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;

	ZeroItem( nIndex );

	//�ȍ~�̃A�C�e����O�ɋl�߂�B
	int i;
	for( i = nIndex; i < *m_pnUserItemCount - 1; i++ )
	{
		CopyItem( i + 1, i );
	}
	ZeroItem( i );

	*m_pnUserItemCount -= 1;

	return true;
}

/*
	���ׂẴA�C�e�����폜����B

	@note	�[���N���A���\�Ƃ��邽�߁A���ׂĂ��ΏۂɂȂ�B
*/
template <class T, class S>
void CRecentImp<T, S>::DeleteAllItem()
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < m_nArrayCount; i++ )
	{
		ZeroItem( i );
	}

	*m_pnUserItemCount = 0;

	return;
}

/*
	�A�C�e�����ړ�����B
*/
template <class T, class S>
bool CRecentImp<T, S>::MoveItem( int nSrcIndex, int nDstIndex )
{
	int	i;
	bool	bFavorite;

	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= *m_pnUserItemCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= *m_pnUserItemCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	DataType pri;

	//�ړ��������ޔ�
	memcpy_raw( &pri, GetItemPointer( nSrcIndex ), sizeof(pri) );
	bFavorite = IsFavorite( nSrcIndex );

	if( nSrcIndex < nDstIndex )
	{
		for( i = nSrcIndex; i < nDstIndex; i++ )
		{
			CopyItem( i + 1, i );
		}
	}
	else
	{
		for( i = nSrcIndex; i > nDstIndex; i-- )
		{
			CopyItem( i - 1, i );
		}
	}

	//�V�����ʒu�Ɋi�[
	memcpy_raw( GetItemPointer( nDstIndex ), &pri, sizeof(pri) );
	SetFavorite( nDstIndex, bFavorite );


	return true;
}

template <class T, class S>
bool CRecentImp<T, S>::CopyItem( int nSrcIndex, int nDstIndex )
{
	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= m_nArrayCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= m_nArrayCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	memcpy_raw( GetItemPointer( nDstIndex ), GetItemPointer( nSrcIndex ), sizeof(DataType) );

	//(void)SetFavorite( nDstIndex, IsFavorite( nSrcIndex ) );
	//�����������Ȃ��Ƃ��߁B
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nDstIndex] = m_pbUserItemFavorite[nSrcIndex];

	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�C�e���擾                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, class S>
const T* CRecentImp<T, S>::GetItem( int nIndex ) const
{
	if(!IsAvailable() || nIndex<0 || nIndex>=*m_pnUserItemCount)return NULL;
	return &m_puUserItemData[nIndex];
}

template <class T, class S>
const T* CRecentImp<T, S>::GetItemPointer(int nIndex) const
{
	if(!IsAvailable() || nIndex<0 || nIndex>=m_nArrayCount)return NULL;
	return &m_puUserItemData[nIndex];
}

/*
	�A�C�e������������B
*/
template <class T, class S>
int CRecentImp<T, S>::FindItem( ReceiveType pItemData ) const
{
	if( !IsAvailable() ) return -1;
	if( !pItemData ) return -1;

	for( int i = 0; i < *m_pnUserItemCount; i++ )
	{
		if( CompareItem(GetItemPointer(i), pItemData) == 0 )return i;
	}

	return -1;
}

/*
	�A�C�e�����X�g��������Ƃ��Â��o���C�ɓ���E�ʏ�p�̃A�C�e����T���B

	bFavorite=true	���C�ɓ���̒�����T��
	bFavorite=false	�ʏ�̒�����T��
*/
template <class T, class S>
int CRecentImp<T, S>::GetOldestItem( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return -1;
	if( nIndex >= *m_pnUserItemCount ) nIndex = *m_pnUserItemCount - 1;

	for( int i = nIndex; i >= 0; i-- )
	{
		if( IsFavorite( i ) == bFavorite ) return i;
	}

	return -1;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�Ǘ�����Ă���A�C�e���̂����̕\������ύX����B

	@note	���C�ɓ���͉\�Ȍ���\�����Ɉړ�������B
*/
template <class T, class S>
bool CRecentImp<T, S>::ChangeViewCount( int nViewCount )
{
	int	i;
	int	nIndex;

	//�͈͊O�Ȃ�G���[
	if( ! IsAvailable() ) return false;
	if( nViewCount < 0 || nViewCount > m_nArrayCount ) return false;

	//�\�������X�V����B
	if( m_pnUserViewCount )
	{
		*m_pnUserViewCount = nViewCount;
	}

	//�͈͓��ɂ��ׂĎ��܂��Ă���̂ŉ������Ȃ��Ă悢�B
	if( nViewCount >= *m_pnUserItemCount ) return true;

	//�ł��Â����C�ɓ����T���B
	i = GetOldestItem( *m_pnUserItemCount - 1, true );
	if( -1 == i ) return true;	//�Ȃ��̂ŉ������Ȃ��ŏI��

	//�\���O�A�C�e����\�����Ɉړ�����B
	for( ; i >= nViewCount; i-- )
	{
		if( IsFavorite( i ) )
		{
			//�J�����g�ʒu�����ɒʏ�A�C�e����T��
			nIndex = GetOldestItem( i - 1, false );
			if( -1 == nIndex ) break;	//����1���Ȃ�

			//���������A�C�e�����J�����g�ʒu�Ɉړ�����
			MoveItem( nIndex, i );
		}
	}

	return true;
}

/*
	���X�g���X�V����B
*/
template <class T, class S>
bool CRecentImp<T, S>::UpdateView()
{
	int	nViewCount;

	//�͈͊O�Ȃ�G���[
	if( ! IsAvailable() ) return false;

	if( m_pnUserViewCount ) nViewCount = *m_pnUserViewCount;
	else                    nViewCount = m_nArrayCount;

	return ChangeViewCount( nViewCount );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �C���X�^���X��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "recent/CRecent.h"
template class CRecentImp<CCmdString, LPCTSTR>;
template class CRecentImp<EditNode>;
template class CRecentImp<EditInfo>;
template class CRecentImp<CPathString, LPCTSTR>;
template class CRecentImp<CGrepFileString, LPCTSTR>;
template class CRecentImp<CGrepFolderString, LPCTSTR>;
template class CRecentImp<CReplaceString, LPCWSTR>;
template class CRecentImp<CSearchString, LPCWSTR>;
template class CRecentImp<CTagjumpKeywordString, LPCWSTR>;
