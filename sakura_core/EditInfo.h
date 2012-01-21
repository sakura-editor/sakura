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
#ifndef SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_
#define SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "types/CType.h"


/*! �t�@�C�����

	@date 2002.03.07 genta m_szDocType�ǉ�
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y�ǉ�
*/
struct EditInfo {
	//�t�@�C��
	TCHAR		m_szPath[_MAX_PATH];					//!< �t�@�C����
	ECodeType	m_nCharCode;							//!< �����R�[�h���
	TCHAR		m_szDocType[MAX_DOCTYPE_LEN + 1];		//!< �����^�C�v
	CTypeConfig m_nType;								//!< �����^�C�v(MRU)

	//�\����
	CLayoutInt	m_nViewTopLine;							//!< �\����̈�ԏ�̍s(0�J�n)
	CLayoutInt	m_nViewLeftCol;							//!< �\����̈�ԍ��̌�(0�J�n)

	//�L�����b�g
	CLogicPoint m_ptCursor;								//!< �L�����b�g�ʒu

	//�e����
	bool		m_bIsModified;							//!< �ύX�t���O

	//GREP���[�h
	bool		m_bIsGrep;								//!< Grep�̃E�B���h�E��
	wchar_t		m_szGrepKey[1024];

	//�f�o�b�O���j�^ (�A�E�g�v�b�g�E�B���h�E) ���[�h
	bool		m_bIsDebug;								//!< �f�o�b�O���j�^���[�h (�A�E�g�v�b�g�E�B���h�E) ��

	//�u�b�N�}�[�N���
	wchar_t		m_szMarkLines[MAX_MARKLINES_LEN + 1];	//!< �u�b�N�}�[�N�̕����s���X�g

	//�E�B���h�E
	int			m_nWindowSizeX;							//!< �E�B���h�E  ��(�s�N�Z����)
	int			m_nWindowSizeY;							//!< �E�B���h�E  ����(�s�N�Z����)
	int			m_nWindowOriginX;						//!< �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��)
	int			m_nWindowOriginY;						//!< �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��)
	
	// Mar. 7, 2002 genta
	// Constructor �m���ɏ��������邽��
	EditInfo()
	: m_nViewTopLine( -1 )
	, m_nViewLeftCol( -1 )
	, m_nType( -1 )
	, m_ptCursor(CLogicInt(-1), CLogicInt(-1))
	, m_bIsModified( 0 )
	, m_nCharCode( CODE_AUTODETECT )
	, m_bIsGrep( false )
	, m_bIsDebug( false )
	, m_nWindowSizeX( -1 )
	, m_nWindowSizeY( -1 )
	, m_nWindowOriginX( CW_USEDEFAULT )	//	2004.05.13 Moca �g�w�薳���h��-1����CW_USEDEFAULT�ɕύX
	, m_nWindowOriginY( CW_USEDEFAULT )
	{
		m_szPath[0] = '\0';
		m_szMarkLines[0] = L'\0';
		m_szDocType[0] = '\0';
	}
};

#endif /* SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_ */
/*[EOF]*/
