#include "stdafx.h"
#include "env/CAppNodeManager.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "util/window.h"
#include "CMutex.h"


// GetOpenedWindowArr�p�ÓI�ϐ��^�\����
static BOOL s_bSort;	// �\�[�g�w��
static BOOL s_bGSort;	// �O���[�v�w��

/*! @brief CShareData::m_pEditArr�ی�pMutex

	�����̃G�f�B�^���񓯊��Ɉ�ē��삵�Ă���Ƃ��ł��ACShareData::m_pEditArr��
	���S�ɑ���ł���悤���쒆��Mutex��Lock()����B

	@par�i�񓯊���ē���̗�j
		�����̃E�B���h�E��\�����Ă��ăO���[�v����L���ɂ����^�X�N�o�[�Łu�O���[�v�����v����������Ƃ�

	@par�i�ی삷��ӏ��̗�j
		CShareData::AddEditWndList(): �G���g���̒ǉ��^���ёւ�
		CShareData::DeleteEditWndList(): �G���g���̍폜
		CShareData::GetOpenedWindowArr(): �z��̃R�s�[�쐬

	����ɂǂ��ɂł������ƃf�b�h���b�N����댯������̂œ����Ƃ��͐T�d�ɁB
	�iLock()���Ԓ���SendMessage()�Ȃǂő��E�B���h�E�̑��������Ɗ댯����j
	CShareData::m_pEditArr�𒼐ڎQ�Ƃ�����ύX����悤�ȉӏ��ɂ͐��ݓI�Ȋ댯�����邪�A
	�Θb�^�ŏ������삵�Ă���͈͂ł���΂܂����͋N���Ȃ��B

	@date 2007.07.05 ryoji �V�K����
	@date 2007.07.07 genta CShareData�̃����o�ֈړ�
*/
static CMutex g_cEditArrMutex( FALSE, GSTR_MUTEX_SAKURA_EDITARR );

// GetOpenedWindowArr�p�\�[�g�֐�
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// �قȂ�O���[�v�̂Ƃ��̓O���[�v��r����
	int nGroup1;
	int nGroup2;

	if( s_bGSort )
	{
		// �I���W�i���̃O���[�v�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}
	else
	{
		// �O���[�v��MRU�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if( nGroup1 != nGroup2 )
	{
		return nGroup1 - nGroup2;	// �O���[�v��r
	}

	// �O���[�v��r���s���Ȃ������Ƃ��̓E�B���h�E��r����
	if( s_bSort )
		return ( ((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex );	// �E�B���h�E�ԍ���r
	return ( ((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p );	// �E�B���h�EMRU��r�i�\�[�g���Ȃ��j
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �O���[�v                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** �w��ʒu�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* CAppNodeGroupHandle::GetEditNodeAt( int nIndex )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	int	i;
	int iIndex;

	iIndex = 0;
	for( i = 0; i < pShare->m_sNodes.m_nEditArrNum; i++ )
	{
		if( m_nGroup == 0 || m_nGroup == pShare->m_sNodes.m_pEditArr[i].m_nGroup )
		{
			if( IsSakuraMainWindow( pShare->m_sNodes.m_pEditArr[i].m_hWnd ) )
			{
				if( iIndex == nIndex )
					return &pShare->m_sNodes.m_pEditArr[i];
				iIndex++;
			}
		}
	}

	return NULL;
}


/** �ҏW�E�B���h�E���X�g�ւ̓o�^

	@param hWnd   [in] �o�^����ҏW�E�B���h�E�̃n���h��

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji �V�K�E�B���h�E�ɂ̓O���[�vID��t�^����
*/
BOOL CAppNodeGroupHandle::AddEditWndList( HWND hWnd )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	int		nSubCommand = TWNT_ADD;
	int		nIndex;
	EditNode	sMyEditNode;
	EditNode	*p;

	memset_raw( &sMyEditNode, 0, sizeof( sMyEditNode ) );
	sMyEditNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock�̈�
		LockGuard<CMutex> guard( g_cEditArrMutex );

		CRecentEditNode	cRecentEditNode;

		//�o�^�ς݂��H
		nIndex = cRecentEditNode.FindItemByHwnd( hWnd );
		if( -1 != nIndex )
		{
			//��������ȏ�o�^�ł��Ȃ����H
			if( cRecentEditNode.GetItemCount() >= cRecentEditNode.GetArrayCount() )
			{
				cRecentEditNode.Terminate();
				return FALSE;
			}
			nSubCommand = TWNT_ORDER;

			//�ȑO�̏����R�s�[����B
			p = cRecentEditNode.GetItem( nIndex );
			if( p )
			{
				memcpy_raw( &sMyEditNode, p, sizeof( sMyEditNode ) );
			}
		}

		/* �E�B���h�E�A�� */

		if( 0 == ::GetWindowLongPtr( hWnd, sizeof(LONG_PTR) ) )
		{
			pShare->m_sNodes.m_nSequences++;
			::SetWindowLongPtr( hWnd, sizeof(LONG_PTR) , (LONG_PTR)pShare->m_sNodes.m_nSequences );

			//�A�Ԃ��X�V����B
			sMyEditNode.m_nIndex = pShare->m_sNodes.m_nSequences;

			/* �^�u�O���[�v�A�� */
			if( m_nGroup > 0 )
			{
				sMyEditNode.m_nGroup = m_nGroup;	// �w��̃O���[�v
			}
			else
			{
				p = cRecentEditNode.GetItem( 0 );
				if( NULL == p )
					sMyEditNode.m_nGroup = ++pShare->m_sNodes.m_nGroupSequences;	// �V�K�O���[�v
				else
					sMyEditNode.m_nGroup = p->m_nGroup;	// �ŋ߃A�N�e�B�u�̃O���[�v
			}

			sMyEditNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			sMyEditNode.m_bClosing = FALSE;
		}

		//�ǉ��܂��͐擪�Ɉړ�����B
		cRecentEditNode.AppendItem( &sMyEditNode );
		cRecentEditNode.Terminate();
	}	// 2007.07.07 genta Lock�̈�I���

	//�E�C���h�E�o�^���b�Z�[�W���u���[�h�L���X�g����B
	CAppNodeGroupHandle(hWnd).PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)nSubCommand, (LPARAM)hWnd, hWnd );

	return TRUE;
}


/** �ҏW�E�B���h�E���X�g����̍폜

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.07.05 ryoji mutex�ŕی�
*/
void CAppNodeGroupHandle::DeleteEditWndList( HWND hWnd )
{
	//�E�C���h�E�����X�g����폜����B
	{	// 2007.07.07 genta Lock�̈�
		LockGuard<CMutex> guard( g_cEditArrMutex );

		CRecentEditNode	cRecentEditNode;
		cRecentEditNode.DeleteItemByHwnd( hWnd );
		cRecentEditNode.Terminate();
	}

	//�E�C���h�E�폜���b�Z�[�W���u���[�h�L���X�g����B
	CAppNodeGroupHandle(m_nGroup).PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_DEL, (LPARAM)hWnd, hWnd);
}

/** �������̃E�B���h�E�֏I���v�����o��

	@param pWndArr [in] EditNode�̔z��Bm_hWnd��NULL�̗v�f�͏������Ȃ�
	@param nArrCnt [in] pWndArr�̒���
	@param bExit [in] TRUE: �ҏW�̑S�I�� / FALSE: ���ׂĕ���
	@param bCheckConfirm [in] FALSE:�����E�B���h�E�����Ƃ��̌x�����o���Ȃ� / TRUE:�x�����o���i�ݒ�ɂ��j
	@param hWndFrom [in] �I���v�����̃E�B���h�E�i�x�����b�Z�[�W�̐e�ƂȂ�j

	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.11.22 syat �S�ā��������ɕύX�B�����E�B���h�E����鎞�̌x�����b�Z�[�W��ǉ�
*/
BOOL CAppNodeGroupHandle::RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, BOOL bCheckConfirm, HWND hWndFrom )
{
	int nCloseCount = 0;

	/* �N���[�Y�ΏۃE�B���h�E�̐��𒲂ׂ� */
	for( int i = 0; i < nArrCnt; i++){
		if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
			if( pWndArr[i].m_hWnd ){
				nCloseCount++;
			}
		}
	}

	if( bCheckConfirm && CShareData::getInstance()->GetShareData()->m_Common.m_sGeneral.m_bCloseAllConfirm ){	//[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
		if( 1 < nCloseCount ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				_T("�����ɕ����̕ҏW�p�E�B���h�E����悤�Ƃ��Ă��܂��B��������܂���?")
			) ){
				return FALSE;
			}
		}
	}

	for( int i = 0; i < nArrCnt; ++i ){
		/* m_hWnd��NULL��ݒ肵��EditNode�͂Ƃ΂� */
		if( pWndArr[i].m_hWnd == NULL )continue;

 		if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
 			if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
 				/* �A�N�e�B�u�ɂ��� */
 				ActivateFrameWindow( pWndArr[i].m_hWnd );
 				/* �g���C����G�f�B�^�ւ̏I���v�� */
 				if( !::SendMessageAny( pWndArr[i].m_hWnd, MYWM_CLOSE, bExit, 0 ) ){	// 2007.02.13 ryoji bExit�������p��
					delete []pWndArr;
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}


/** ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�

	@param bExcludeClosing [in] �I�����̕ҏW�E�B���h�E�̓J�E���g���Ȃ�

	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.04.19 ryoji bExcludeClosing������ǉ�
*/
int CAppNodeGroupHandle::GetEditorWindowsNum( bool bExcludeClosing/* = true */ )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	int		i;
	int		j;

	j = 0;
	for( i = 0; i < pShare->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( pShare->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			if( m_nGroup != 0 && m_nGroup != CAppNodeManager::Instance()->GetEditNode( pShare->m_sNodes.m_pEditArr[i].m_hWnd )->GetGroup() )
				continue;
			if( bExcludeClosing && pShare->m_sNodes.m_pEditArr[i].m_bClosing )
				continue;
			j++;
		}
	}
	return j;

}


/** �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����

	@date 2005.01.24 genta hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CAppNodeGroupHandle::PostMessageToAllEditors(
	UINT		uMsg,		/*!< �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/*!< ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/*!< ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast	/*!< �Ō�ɑ��肽���E�B���h�E */
)
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = CAppNodeManager::Instance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}

/** �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�

	@date 2005.01.24 genta m_hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CAppNodeGroupHandle::SendMessageToAllEditors(
	UINT		uMsg,		/* �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/* ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/* ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast	/* �Ō�ɑ��肽���E�B���h�E */
)
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = CAppNodeManager::Instance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( m_nGroup == 0 || m_nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �}�l�[�W��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** �O���[�v��ID���Z�b�g����

	@date 2007.06.20 ryoji
*/
void CAppNodeManager::ResetGroupId()
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	int nGroup = ++pShare->m_sNodes.m_nGroupSequences;
	for( int i = 0; i < pShare->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( pShare->m_sNodes.m_pEditArr[i].m_hWnd ) )
		{
			pShare->m_sNodes.m_pEditArr[i].m_nGroup = nGroup;
		}
	}
}

/** �ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji

	@warning ���̊֐���m_pEditArr���̗v�f�ւ̃|�C���^��Ԃ��D
	m_pEditArr���ύX���ꂽ��ł̓A�N�Z�X���Ȃ��悤���ӂ��K�v�D
*/
EditNode* CAppNodeManager::GetEditNode( HWND hWnd )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	for( int i = 0; i < pShare->m_sNodes.m_nEditArrNum; i++ )
	{
		if( hWnd == pShare->m_sNodes.m_pEditArr[i].m_hWnd )
		{
			if( IsSakuraMainWindow( pShare->m_sNodes.m_pEditArr[i].m_hWnd ) )
				return &pShare->m_sNodes.m_pEditArr[i];
		}
	}

	return NULL;
}



/** ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

	@param[out] ppEditNode �z����󂯎��|�C���^
		�߂�l��0�̏ꍇ��NULL���Ԃ���邪�C��������҂��Ȃ����ƁD
		�܂��C�s�v�ɂȂ�����delete []���Ȃ��Ă͂Ȃ�Ȃ��D
	@param[in] bSort TRUE: �\�[�g���� / FALSE: �\�[�g����
	@param[in]bGSort TRUE: �O���[�v�\�[�g���� / FALSE: �O���[�v�\�[�g����

	���Ƃ̕ҏW�E�B���h�E���X�g�̓\�[�g���Ȃ���΃E�B���h�E��MRU���ɕ���ł���
	-------------------------------------------------
	bSort	bGSort	��������
	-------------------------------------------------
	FALSE	FALSE	�O���[�vMRU���|�E�B���h�EMRU��
	TRUE	FALSE	�O���[�vMRU���|�E�B���h�E�ԍ���
	FALSE	TRUE	�O���[�v�ԍ����|�E�B���h�EMRU��
	TRUE	TRUE	�O���[�v�ԍ����|�E�B���h�E�ԍ���
	-------------------------------------------------

	@return �z��̗v�f����Ԃ�
	@note �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete []���Ă�������

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji bGroup�����ǉ��A�\�[�g���������O�̂��̂���qsort�ɕύX
*/
int CAppNodeManager::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	int nRet;

	LockGuard<CMutex> guard( g_cEditArrMutex );
	nRet = _GetOpenedWindowArrCore( ppEditNode, bSort, bGSort );

	return nRet;
}

// GetOpenedWindowArr�֐��R�A������
int CAppNodeManager::_GetOpenedWindowArrCore( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	//�ҏW�E�C���h�E�����擾����B
	EditNodeEx*	pNode;	// �\�[�g�����p�̊g�����X�g
	int		nRowNum;	//�ҏW�E�C���h�E��
	int		i;

	//�ҏW�E�C���h�E�����擾����B
	*ppEditNode = NULL;
	if( pShare->m_sNodes.m_nEditArrNum <= 0 )
		return 0;

	//�ҏW�E�C���h�E���X�g�i�[�̈���쐬����B
	*ppEditNode = new EditNode[ pShare->m_sNodes.m_nEditArrNum ];
	if( NULL == *ppEditNode )
		return 0;

	// �g�����X�g���쐬����
	pNode = new EditNodeEx[ pShare->m_sNodes.m_nEditArrNum ];
	if( NULL == pNode )
	{
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g�̊e�v�f�ɕҏW�E�B���h�E���X�g�̊e�v�f�ւ̃|�C���^���i�[����
	nRowNum = 0;
	for( i = 0; i < pShare->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( pShare->m_sNodes.m_pEditArr[ i ].m_hWnd ) )
		{
			pNode[ nRowNum ].p = &pShare->m_sNodes.m_pEditArr[ i ];	// �|�C���^�i�[
			pNode[ nRowNum ].nGroupMru = -1;	// �O���[�v�P�ʂ�MRU�ԍ�������
			nRowNum++;
		}
	}
	if( nRowNum <= 0 )
	{
		delete []pNode;
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g��ŃO���[�v�P�ʂ�MRU�ԍ�������
	if( !bGSort )
	{
		int iGroupMru = 0;	// �O���[�v�P�ʂ�MRU�ԍ�
		int nGroup = -1;
		for( i = 0; i < nRowNum; i++ )
		{
			if( pNode[ i ].nGroupMru == -1 && nGroup != pNode[ i ].p->m_nGroup )
			{
				nGroup = pNode[ i ].p->m_nGroup;
				iGroupMru++;
				pNode[ i ].nGroupMru = iGroupMru;	// MRU�ԍ��t�^

				// ����O���[�v�̃E�B���h�E�ɓ���MRU�ԍ�������
				int j;
				for( j = i + 1; j < nRowNum; j++ )
				{
					if( pNode[ j ].p->m_nGroup == nGroup )
						pNode[ j ].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// �g�����X�g���\�[�g����
	// Note. �O���[�v���P�����̏ꍇ�͏]���ibGSort ���������j�Ɠ������ʂ�������
	//       �i�O���[�v������ݒ�łȂ���΃O���[�v�͂P�j
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort( pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr );

	// �g�����X�g�̃\�[�g���ʂ����ƂɕҏW�E�C���h�E���X�g�i�[�̈�Ɍ��ʂ��i�[����
	for( i = 0; i < nRowNum; i++ )
	{
		(*ppEditNode)[i] = *pNode[i].p;

		//�C���f�b�N�X��t����B
		//���̃C���f�b�N�X�� m_pEditArr �̔z��ԍ��ł��B
		(*ppEditNode)[i].m_nIndex = pNode[i].p - pShare->m_sNodes.m_pEditArr;	// �|�C���^���Z���z��ԍ�
	}

	delete []pNode;

	return nRowNum;
}

/** �E�B���h�E�̕��ёւ�

	@param[in] hSrcTab �ړ�����E�B���h�E
	@param[in] hSrcTab �ړ���E�B���h�E

	@author ryoji
	@date 2007.07.07 genta �E�B���h�E�z�񑀍암��CTabWnd���ړ�
*/
bool CAppNodeManager::ReorderTab( HWND hwndSrc, HWND hwndDst )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	EditNode	*p = NULL;
	int			nCount;
	int			i;

	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<CMutex> guard( g_cEditArrMutex );
	nCount = _GetOpenedWindowArrCore( &p, TRUE );	// ���b�N�͎����ł���Ă���̂Œ��ڃR�A���Ăяo��
	for( i = 0; i < nCount; i++ )
	{
		if( hwndSrc == p[i].m_hWnd )
			nSrcTab = i;
		if( hwndDst == p[i].m_hWnd )
			nDstTab = i;
	}

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
	{
		if( p ) delete []p;
		return false;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[ nDstTab ].m_nIndex;
	nIndex = pShare->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// �^�u���������[�e�[�g
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			pShare->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = pShare->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// �^�u�E�������[�e�[�g
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			pShare->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = pShare->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	pShare->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = nIndex;

	if( p ) delete []p;
	return true;
}

/** �^�u�ړ��ɔ����E�B���h�E����

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E�D�V�K�Ɨ�����NULL�D
	@param[in] bSrcIsTop �ړ�����E�B���h�E�����E�B���h�E�Ȃ�true
	@param[in] notifygroups �^�u�̍X�V���K�v�ȃO���[�v�̃O���[�vID�Dint[2]���Ăяo�����ŗp�ӂ���D

	@return �X�V���ꂽhwndDst (�ړ��悪���ɕ���ꂽ�ꍇ�Ȃǂ�NULL�ɕύX����邱�Ƃ�����)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()���Ɨ�
*/
HWND CAppNodeManager::SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	LockGuard<CMutex> guard( g_cEditArrMutex );

	EditNode* pSrcEditNode = GetEditNode( hwndSrc );
	EditNode* pDstEditNode = GetEditNode( hwndDst );
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if( pDstEditNode == NULL )
	{
		hwndDst = NULL;
		nDstGroup = ++pShare->m_sNodes.m_nGroupSequences;	// �V�K�O���[�v
	}
	else
	{
		nDstGroup = pDstEditNode->m_nGroup;	// �����O���[�v
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++pShare->m_sNodes.m_nSequences;	// �^�u���т̍Ō�i�N�����̍Ō�j�ɂ����Ă���

	// ��\���̃^�u�������O���[�v�Ɉړ�����Ƃ��͔�\���̂܂܂ɂ���̂�
	// ���������擪�ɂ͂Ȃ�Ȃ��悤�A�K�v�Ȃ�擪�E�B���h�E�ƈʒu����������B
	if( !bSrcIsTop && pDstEditNode != NULL )
	{
		if( pSrcEditNode < pDstEditNode )
		{
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}





/** ����O���[�v���ǂ����𒲂ׂ�

	@param[in] hWnd1 ��r����E�B���h�E1
	@param[in] hWnd2 ��r����E�B���h�E2
	
	@return 2�̃E�B���h�E������O���[�v�ɑ����Ă����true

	@date 2007.06.20 ryoji
*/
bool CAppNodeManager::IsSameGroup( HWND hWnd1, HWND hWnd2 )
{
	if( hWnd1 == hWnd2 )
		return true;

	CAppNodeGroupHandle cGroup1 = CAppNodeManager::Instance()->GetEditNode(hWnd1)->GetGroup();
	CAppNodeGroupHandle cGroup2 = CAppNodeManager::Instance()->GetEditNode(hWnd2)->GetGroup();
	if(cGroup1.IsValidGroup() && cGroup1==cGroup2){
		return true;
	}

	return false;
}


