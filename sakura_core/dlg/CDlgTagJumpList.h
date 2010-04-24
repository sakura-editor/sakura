/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.13
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



class CDlgTagJumpList;

#ifndef	_CDLGTAGJUMPLIST_H_
#define	_CDLGTAGJUMPLIST_H_

#include "dlg/CDialog.h"
#include "CSortedTagJumpList.h"

//	@@ 2005.03.31 MIK
//�ҏW�{�b�N�X���R���{���ǂ���
#define TAGJUMP_EDITBOX_IS_COMBO

//�L�[���[�h����͂��ĊY���������\������܂ł̎���(�~���b)
#define TAGJUMP_TIMER_DELAY 700
//�^�O�t�@�C����	//	@@ 2005.03.31 MIK �萔��
#define TAG_FILENAME        L"tags"
//�^�O�t�@�C���̃t�H�[�}�b�g	//	@@ 2005.03.31 MIK �萔��
//	@@ 2005.04.03 MIK �L�[���[�h�ɋ󔒂��܂܂��ꍇ�̍l��
#define TAG_FORMAT       _T("%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%s\t%s")

/*!	@brief �_�C���N�g�^�O�W�����v���ꗗ�_�C�A���O

	�_�C���N�g�^�O�W�����v�ŕ����̌�₪����ꍇ�y��
	�L�[���[�h�w��^�O�W�����v�̂��߂̃_�C�A���O�{�b�N�X����
*/
class SAKURA_CORE_API CDlgTagJumpList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagJumpList();
	~CDlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */

	//	@@ 2005.03.31 MIK �K�w�p�����[�^��ǉ�
	bool AddParam( TCHAR *s0, TCHAR *s1, int n2, TCHAR *s3, TCHAR *s4, int depth );	//�o�^
	bool GetSelectedParam( TCHAR *s0, TCHAR *s1, int *n2, TCHAR *s3, TCHAR *s4, int *depth );	//�擾
	void SetFileName( const TCHAR *pszFileName );
	void SetKeyword( const wchar_t *pszKeyword );	//	@@ 2005.03.31 MIK

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	//	@@ 2005.03.31 MIK �L�[���[�h���̓G���A�̃C�x���g����
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnCbnEditChange( HWND hwndCtl, int wID );
	//BOOL	OnEnChange( HWND hwndCtl, int wID );
	BOOL	OnTimer( WPARAM wParam );
	LPVOID	GetHelpIdTable( void );
	void	StopTimer( void );
	void	StartTimer( void );

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void	UpdateData( void );	//	@@ 2005.03.31 MIK

	TCHAR	*GetNameByType( const TCHAR type, const TCHAR *name );	//�^�C�v�𖼑O�ɕϊ�����B
	int		SearchBestTag( void );	//�����Ƃ��m���̍������ȃC���f�b�N�X��Ԃ��B
	//	@@ 2005.03.31 MIK
	const TCHAR *GetFileName( void );
	const TCHAR *GetFilePath( void ){ return m_pszFileName != NULL ? m_pszFileName : _T(""); }
	void find_key( const wchar_t* keyword );
	void Empty( void );



private:

	int		m_nIndex;		//!< �I�����ꂽ�v�f�ԍ�
	TCHAR	*m_pszFileName;	//!< �ҏW���̃t�@�C����
	wchar_t	*m_pszKeyword;	//!< �L�[���[�h(DoModal��lParam!=0���w�肵���ꍇ�Ɏw��ł���)
	int		m_nLoop;		//!< �����̂ڂ��K�w��
	CSortedTagJumpList	m_cList;	//!< �^�O�W�����v���
	UINT	m_nTimerId;		//!< �^�C�}�ԍ�
	BOOL	m_bTagJumpICase;	//!< �啶���������𓯈ꎋ
	BOOL	m_bTagJumpAnyWhere;	//!< ������̓r���Ƀ}�b�`

};

#endif	//_CDLGTAGJUMPLIST_H_

