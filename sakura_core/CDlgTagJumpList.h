/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK

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

#include "CDialog.h"

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

	bool AddParam( char *s0, char *s1, int n2, char *s3, char *s4 );	//�o�^
	bool GetSelectedParam( char *s0, char *s1, int *n2, char *s3, char *s4 );	//�擾
	void SetFileName( const char *pszFileName );

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	LPVOID	GetHelpIdTable( void );

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

	char	*GetNameByType( const char type, const char *name );	//�^�C�v�𖼑O�ɕϊ�����B
	int		SearchBestTag( void );	//�����Ƃ��m���̍������ȃC���f�b�N�X��Ԃ��B

private:
	typedef struct {
		char	*s0;	//�L�[���[�h
		char	*s1;	//�t�@�C����
		int		n2;		//�s�ԍ�
		char	*s3;	//�^�C�v
		char	*s4;	//���l
	} ParamTag;

#define	MAX_TAGJUMPLIST	100	//�^�O�W�����v���X�g�̍ő�Ǘ���(����ȏ゠���Ă��I�ׂ�H)
	int			m_nCount;
	ParamTag	m_uParam[MAX_TAGJUMPLIST];
	int			m_nIndex;
	bool		m_bOverflow;	//�o�^�����������邩�H
	char		*m_pszFileName;

};

#endif	//_CDLGTAGJUMPLIST_H_

