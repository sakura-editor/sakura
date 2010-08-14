/*!	@file
	@brief �����L�[���[�h�I���_�C�A���O

	@author MIK
	@date 2005/01/13 �쐬
*/
/*
	Copyright (C) 2005, MIK, genta

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

#ifndef SC_CDLGKEYWORDSELECT_H__
#define SC_CDLGKEYWORDSELECT_H__

#include "dlg/CDialog.h"
#include "config/maxdata.h" // MAX_KEYWORDSET_PER_TYPE
 
class CKeyWordSetMgr;

/*
	�����L�[���[�h�I���\��
	1�`10�͈̔͂Ŏw��ł���B
	�������A�\�[�X�̏C���͕K�v�ł��B
*/

//	2005.01.13 genta ShareData�̒�`�ƘA��������
const int KEYWORD_SELECT_NUM = MAX_KEYWORDSET_PER_TYPE;

class CDlgKeywordSelect : public CDialog
{
public:
	CDlgKeywordSelect();
	~CDlgKeywordSelect();
	int DoModal( HINSTANCE, HWND, int* pnSet );

protected:

	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	int  GetData( void );
	void SetData( void );
	LPVOID GetHelpIdTable( void );

	int m_nSet[ KEYWORD_SELECT_NUM ];
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;
};

#endif


