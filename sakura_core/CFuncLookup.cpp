/*!	@file
	@brief �\���p�����񓙂̎擾

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D

	@author genta
	@date Oct.  1, 2001 �}�N��
	@date Oct. 15, 2001 �J�X�^�����j���[
*/
/*
	Copyright (C) 2001, genta

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

#include "stdafx.h"
#include "CFuncLookup.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka

//	�I�t�Z�b�g�l
const int LUOFFSET_MACRO = 0;
const int LUOFFSET_CUSTMENU = 1;

//! ���I�ɓ��e���ς�镪�ނ̖��O
const char *DynCategory[] = {
	"�O���}�N��",
	"�J�X�^�����j���["
};

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\�ԍ���Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
*/
int CFuncLookup::Pos2FuncCode( int category, int position ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppnFuncListArr[category][position];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO){
		//	�L�[���蓖�ă}�N��
		if( m_pcSMacroMgr->IsEnabled(position))
			return F_USERMACRO_0 + position;
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU){
		//	�L�[���蓖�ă}�N��
		if( position == 0 )
			return F_MENU_RBUTTON;
		else if( 1 <= position && position <= MAX_CUSTOM_MENU - 1 )
			return F_CUSTMENU_BASE + position;
	}
	return 0;
}

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\���̂�Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
	@param ptr [out] ��������i�[����o�b�t�@�̐擪
	@param bufsize [in] ��������i�[����o�b�t�@�̃T�C�Y

	@retval true ���̂̐ݒ�ɐ���
	@retval false ���s�B������͊i�[����Ă��Ȃ�
*/
bool CFuncLookup::Pos2FuncName( int category, int position, char *ptr, int bufsize ) const
{
	int func;
	if( category < nsFuncCode::nFuncKindNum ){
		func = nsFuncCode::ppnFuncListArr[category][position];
		return ( ::LoadString( m_hInstance, func, ptr, bufsize ) > 0 );
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO){
		//	�L�[���蓖�ă}�N��
		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU){
		//	�L�[���蓖�ă}�N��
		if( 0 <= position && position < MAX_CUSTOM_MENU )
		{
			strncpy( ptr, m_pCommon->m_szCustMenuNameArr[position], bufsize );
			ptr[bufsize-1] = '\0';
		}
		else
			return false;
	}
	return true;
}

/*!	@brief �@�\�ԍ��ɑΉ�����@�\���̂�Ԃ��D

	@param funccode [in] �@�\�ԍ�
	@param ptr [out] ��������i�[����o�b�t�@�̐擪
	@param bufsize [in] ��������i�[����o�b�t�@�̃T�C�Y
	
	@retval true ���̂̐ݒ�ɐ���
	@retval false ���s�B������͊i�[����Ă��Ȃ�
*/
bool CFuncLookup::Funccode2Name( int funccode, char *ptr, int bufsize ) const
{
	if( F_USERMACRO_0 <= funccode && funccode < F_USERMACRO_0 + MAX_CUSTMACRO ){
		int position = funccode - F_USERMACRO_0;
		if( !m_pcSMacroMgr->IsEnabled( position )){
			*ptr = '\0';
			return false;
		}

		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	else if( funccode == F_MENU_RBUTTON ){
		strncpy( ptr, m_pCommon->m_szCustMenuNameArr[0], bufsize );
		ptr[bufsize-1] = '\0';
	}
	else if( F_CUSTMENU_1 <= funccode && funccode < F_CUSTMENU_BASE + MAX_CUSTMACRO ){
		strncpy( ptr, m_pCommon->m_szCustMenuNameArr[ funccode - F_CUSTMENU_BASE ], bufsize );
		ptr[bufsize-1] = '\0';
	}
	else {
		return ( ::LoadString( m_hInstance, funccode, ptr, bufsize ) > 0 );
	}
	return true;
}

/*!	@brief �@�\���ޔԍ��ɑΉ�����@�\���̂�Ԃ��D

	@param category [in] �@�\���ޔԍ�
	
	@return NULL ���ޖ��́D�擾�Ɏ��s������NULL�D
*/
const char* CFuncLookup::Category2Name( int category ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppszFuncKind[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		return DynCategory[0];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		return DynCategory[1];
	}
	return NULL;
}

/*!	@brief ComboBox�ɗ��p�\�ȋ@�\���ވꗗ��o�^����

	@param hComboBox [in(out)] �f�[�^��ݒ肷��R���{�{�b�N�X
*/
void CFuncLookup::SetCategory2Combo( HWND hComboBox ) const
{
	int i;

	//	���X�g������������
	::SendMessage( hComboBox, CB_RESETCONTENT, 0, (LPARAM)0 );

	//	�Œ�@�\���X�g
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
	}

	//	���[�U�}�N��
	::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)DynCategory[0] );
	//	�J�X�^�����j���[
	::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)DynCategory[1] );
}

/*!	@brief �w�肳�ꂽ���ނɑ�����@�\���X�g��ListBox�ɓo�^����D
	
	@param hListBox [in(out)] �l��ݒ肷�郊�X�g�{�b�N�X
	@param category [in] �@�\���ޔԍ�
*/
void CFuncLookup::SetListItem( HWND hListBox, int category ) const
{
	char pszLabel[256];
	int i;

	//	���X�g������������
	::SendMessage( hListBox, LB_RESETCONTENT , 0, (LPARAM)0 );

	if( category < nsFuncCode::nFuncKindNum ){
		for( i = 0; i < nsFuncCode::pnFuncListNumArr[category]; ++i ){
			if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[category])[i], pszLabel, 255 ) ){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)pszLabel );
			}else{
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"--����`--" );
			}
		}
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	�}�N��
		for( i = 0; i < MAX_CUSTMACRO ; ++i ){
			if( m_pcSMacroMgr->IsEnabled(i)){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)m_pcSMacroMgr->GetTitle(i));
			}
			else {
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"unavailable" );
			}
		}
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		for( i = 0; i < MAX_CUSTOM_MENU ; ++i ){
			::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)m_pCommon->m_szCustMenuNameArr[i] );
		}
	}
}

/*!
	�w�蕪�ޒ��̋@�\�����擾����D
	
	@param category [in] �@�\���ޔԍ�
*/
int CFuncLookup::GetItemCount(int category) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::pnFuncListNumArr[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	�}�N��
		return MAX_CUSTMACRO;
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		//	�}�N��
		return MAX_CUSTOM_MENU;
	}
	return 0;
}
