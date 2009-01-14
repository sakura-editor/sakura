#pragma once

#include "util/design_template.h"

class CAppNodeGroupHandle;

//! �ҏW�E�B���h�E�m�[�h
struct EditNode {
	int				m_nIndex;
	int				m_nGroup;					//!< �O���[�vID								//@@@ 2007.06.20 ryoji
	HWND			m_hWnd;
	WIN_CHAR		m_szTabCaption[_MAX_PATH];	//!< �^�u�E�C���h�E�p�F�L���v�V������		//@@@ 2003.05.31 MIK
	SFilePath		m_szFilePath;				//!< �^�u�E�C���h�E�p�F�t�@�C����			//@@@ 2006.01.28 ryoji
	BOOL			m_bIsGrep;					//!< Grep�̃E�B���h�E��						//@@@ 2006.01.28 ryoji
	UINT			m_showCmdRestore;			//!< ���̃T�C�Y�ɖ߂��Ƃ��̃T�C�Y���		//@@@ 2007.06.20 ryoji
	BOOL			m_bClosing;					//!< �I�������i�u�Ō�̃t�@�C������Ă�(����)���c���v�p�j	//@@@ 2007.06.20 ryoji

	HWND GetHwnd() const{ return GetSafeHwnd(); }
	HWND GetSafeHwnd() const{ if(this)return m_hWnd; else return NULL; }
	CAppNodeGroupHandle GetGroup() const;
	bool IsTopInGroup() const;
};

struct EditNodeEx{	// �g���\����
	EditNode*	p;	// �ҏW�E�B���h�E�z��v�f�ւ̃|�C���^
	int			nGroupMru;	// �O���[�v�P�ʂ�MRU�ԍ�
};


//���L���������\����
struct SShare_Nodes{
	int					m_nEditArrNum;	//short->int�ɏC��	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//�ő�l�C��	@@@ 2003.05.31 MIK
	LONG				m_nSequences;	/* �E�B���h�E�A�� */
	LONG				m_nGroupSequences;	// �^�u�O���[�v�A��	// 2007.06.20 ryoji
};


//! �m�[�h�A�N�Z�T
class CAppNodeHandle{
public:
	CAppNodeHandle(HWND hwnd);
	EditNode* operator->(){ return m_pNodeRef; }
private:
	EditNode* m_pNodeRef;
};

//! �O���[�v�A�N�Z�T
class CAppNodeGroupHandle{
public:
	CAppNodeGroupHandle(int nGroupId) : m_nGroup(nGroupId) { }
	CAppNodeGroupHandle(HWND hwnd){ m_nGroup = CAppNodeHandle(hwnd)->GetGroup(); }

	EditNode* GetTopEditNode(){ return GetEditNodeAt(0); }	//
	EditNode* GetEditNodeAt( int nIndex );					//!< �w��ʒu�̕ҏW�E�B���h�E�����擾����
	BOOL AddEditWndList( HWND );							//!< �ҏW�E�B���h�E�̓o�^	// 2007.06.26 ryoji nGroup�����ǉ�
	void DeleteEditWndList( HWND );							//!< �ҏW�E�B���h�E���X�g����̍폜
	BOOL RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, BOOL bCheckConfirm, HWND hWndFrom );
															//!< �������̃E�B���h�E�֏I���v�����o��	// 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�	// 2007.06.20 ryoji nGroup�����ǉ�

	int GetEditorWindowsNum( bool bExcludeClosing = true );				/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */	// 2007.06.20 ryoji nGroup�����ǉ�	// 2008.04.19 ryoji bExcludeClosing�����ǉ�

	//�S�E�B���h�E�ꊇ����
	BOOL PostMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */	// 2007.06.20 ryoji nGroup�����ǉ�
	BOOL SendMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂邷�� */	// 2007.06.20 ryoji nGroup�����ǉ�

public:
	bool operator==(const CAppNodeGroupHandle& rhs) const{ return m_nGroup==rhs.m_nGroup; }
	bool IsValidGroup() const{ return m_nGroup>=0; }
	operator int() const{ return m_nGroup; }

private:
	int m_nGroup;
};


class CAppNodeManager : public TSingleton<CAppNodeManager>{
public:
	//�O���[�v
	void ResetGroupId();									/* �O���[�v��ID���Z�b�g���� */

	//�E�B���h�E�n���h�� �� �m�[�h�@�ϊ�
	EditNode* GetEditNode( HWND hWnd );							/* �ҏW�E�B���h�E�����擾���� */

	//�^�u
	bool ReorderTab( HWND hSrcTab, HWND hDstTab );				/* �^�u�ړ��ɔ����E�B���h�E�̕��ёւ� 2007.07.07 genta */
	HWND SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] );/* �^�u�����ɔ����E�B���h�E���� 2007.07.07 genta */

	//�������
	int GetOpenedWindowArr( EditNode** , BOOL, BOOL bGSort = FALSE );				/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */

protected:
	int _GetOpenedWindowArrCore( EditNode** , BOOL, BOOL bGSort = FALSE );			/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ��i�R�A�������j */

public:
	static bool IsSameGroup( HWND hWnd1, HWND hWnd2 );					/* ����O���[�v���ǂ����𒲂ׂ� */
};


inline CAppNodeGroupHandle EditNode::GetGroup() const{ if(this)return m_nGroup; else return NULL; }

inline bool EditNode::IsTopInGroup() const{ return this && (CAppNodeGroupHandle(m_nGroup).GetEditNodeAt(0) == this); }

inline CAppNodeHandle::CAppNodeHandle(HWND hwnd)
{
	m_pNodeRef = CAppNodeManager::Instance()->GetEditNode(hwnd);
}
