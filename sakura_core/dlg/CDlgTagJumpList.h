/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.13
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2010, Moca

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

#ifndef	SAKURA_CDLGTAGJUMPLIST_H_
#define	SAKURA_CDLGTAGJUMPLIST_H_

#include "dlg/CDialog.h"

//�^�O�t�@�C����	//	@@ 2005.03.31 MIK �萔��
#define TAG_FILENAME_T        _T("tags")

// 2010.07.22 ������cpp�ֈړ�

class CSortedTagJumpList;

/*!	@brief �_�C���N�g�^�O�W�����v���ꗗ�_�C�A���O

	�_�C���N�g�^�O�W�����v�ŕ����̌�₪����ꍇ�y��
	�L�[���[�h�w��^�O�W�����v�̂��߂̃_�C�A���O�{�b�N�X����
*/
class CDlgTagJumpList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagJumpList(bool bDirectTagJump);
	~CDlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */

	//	@@ 2005.03.31 MIK �K�w�p�����[�^��ǉ�
//	bool AddParamA( const ACHAR*, const ACHAR*, int, const ACHAR*, const ACHAR*, int depth, int baseDirId );	//�o�^
	bool GetSelectedParam( TCHAR *s0, TCHAR *s1, int *n2, TCHAR *s3, TCHAR *s4, int *depth, TCHAR* fileBase  );	//�擾
	void SetFileName( const TCHAR *pszFileName );
	void SetKeyword( const wchar_t *pszKeyword );	//	@@ 2005.03.31 MIK
	int  FindDirectTagJump();

	bool GetSelectedFullPathAndLine( TCHAR* fullPath, int count, int* lineNum, int* depth );

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
	BOOL	OnSize( WPARAM wParam, LPARAM lParam );
	BOOL	OnMinMaxInfo( LPARAM lParam );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	//	@@ 2005.03.31 MIK �L�[���[�h���̓G���A�̃C�x���g����
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnCbnEditChange( HWND hwndCtl, int wID );
	//BOOL	OnEnChange( HWND hwndCtl, int wID );
	BOOL	OnTimer( WPARAM wParam );
	LPVOID	GetHelpIdTable( void );

private:
	void	StopTimer( void );
	void	StartTimer( int );

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void	UpdateData( bool );	//	@@ 2005.03.31 MIK

	TCHAR	*GetNameByType( const TCHAR type, const TCHAR *name );	//�^�C�v�𖼑O�ɕϊ�����B
	int		SearchBestTag( void );	//�����Ƃ��m���̍������ȃC���f�b�N�X��Ԃ��B
	//	@@ 2005.03.31 MIK
	const TCHAR *GetFileName( void );
	const TCHAR *GetFilePath( void ){ return m_pszFileName != NULL ? m_pszFileName : _T(""); }
	void Empty( void );
	void SetTextDir();
	void FindNext( bool );
	void find_key( const wchar_t* keyword );
	int find_key_core(int, const wchar_t*, bool, bool, bool, bool, int);
	
	bool IsDirectTagJump();
	
	void ClearPrevFindInfo();

	//! depth���犮�S�p�X��(���΃p�X/��΃p�X)���쐬����
	static TCHAR* GetFullPathFromDepth( TCHAR*, int, TCHAR*, const TCHAR*, int );
	static int CalcMaxUpDirectory( const TCHAR* );
	static TCHAR* CopyDirDir( TCHAR* dest, const TCHAR* target, const TCHAR* base );
	static TCHAR* DirUp( TCHAR* dir );

private:

	struct STagFindState{
		int   m_nDepth;
		int   m_nMatchAll;
		int   m_nNextMode;
		int   m_nLoop;
		bool  m_bJumpPath;
		TCHAR m_szCurPath[1024];
	};
	
	bool	m_bDirectTagJump;

	int		m_nIndex;		//!< �I�����ꂽ�v�f�ԍ�
	TCHAR	*m_pszFileName;	//!< �ҏW���̃t�@�C����
	wchar_t	*m_pszKeyword;	//!< �L�[���[�h(DoModal��lParam!=0���w�肵���ꍇ�Ɏw��ł���)
	int		m_nLoop;		//!< �����̂ڂ��K�w��
	CSortedTagJumpList*	m_pcList;	//!< �^�O�W�����v���
	UINT	m_nTimerId;		//!< �^�C�}�ԍ�
	BOOL	m_bTagJumpICase;	//!< �啶���������𓯈ꎋ
	BOOL	m_bTagJumpAnyWhere;	//!< ������̓r���Ƀ}�b�`
	BOOL	m_bTagJumpExactMatch; //! ���S��v(��ʖ���)

	int 	m_nTop;			//!< �y�[�W�߂���̕\���̐擪(0�J�n)
	bool	m_bNextItem;	//!< �܂����Ƀq�b�g������̂�����

	// �i�荞�݌����p
	STagFindState* m_psFindPrev; //<! �O��̍Ō�Ɍ����������
	STagFindState* m_psFind0Match; //<! �O���1��Hit���Ȃ������Ō��tags

	CNativeW	m_strOldKeyword;	//!< �O��̃L�[���[�h
	BOOL	m_bOldTagJumpICase;	//!< �O��̑啶���������𓯈ꎋ
	BOOL	m_bOldTagJumpAnyWhere;	//!< �O��̕�����̓r���Ƀ}�b�`
	
	POINT	m_ptDefaultSize;
	RECT	m_rcItems[11];
};

#endif	//SAKURA_CDLGTAGJUMPLIST_H_

