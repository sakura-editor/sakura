//	$Id$
/*!	@file

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D

	@author genta
	@date Oct. 1, 2001
*/

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\�ԍ���Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
*/

#include "CFuncLookup.h"

const char *DynCategory[] = {
	"�O���}�N��"
};

int CFuncLookup::Pos2FuncCode( int category, int position ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppnFuncListArr[category][position];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	�L�[���蓖�ă}�N��
		if( m_pcSMacroMgr->IsEnabled(position))
			return F_USERMACRO_0 + position;
	}
	return 0;
}

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\���̂�Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
	@param ptr [out] ��������i�[����o�b�t�@�̐擪
	@param bufsize [in] ��������i�[����o�b�t�@�̃T�C�Y
*/
bool CFuncLookup::Pos2FuncName( int category, int position, char *ptr, int bufsize ) const
{
	int func;
	if( category < nsFuncCode::nFuncKindNum ){
		func = nsFuncCode::ppnFuncListArr[category][position];
		return ( ::LoadString( m_hInstance, func, ptr, bufsize ) > 0 );
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	�L�[���蓖�ă}�N��
		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	return true;
}

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\���̂�Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
	@param ptr [out] ��������i�[����o�b�t�@�̐擪
	@param bufsize [in] ��������i�[����o�b�t�@�̃T�C�Y
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
	else {
		return ( ::LoadString( m_hInstance, funccode, ptr, bufsize ) > 0 );
	}
	return true;
}

const char* CFuncLookup::Category2Name( int category ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppszFuncKind[category];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		return DynCategory[0];
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
}

/*!	@brief �w�肳�ꂽ���ނɑ�����@�\���X�g��ListBox�ɓo�^����D
	
	@param hListBox [in(out)] �l��ݒ肷�郊�X�g�{�b�N�X
	@param category [in] �@�\����
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
	else if( category == nsFuncCode::nFuncKindNum ){
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
}

int CFuncLookup::GetItemCount(int category) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::pnFuncListNumArr[category];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	�}�N��
		return MAX_CUSTMACRO;
	}
	return 0;
}
