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

#include "StdAfx.h"
#include "CDocEditor.h"
#include "CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CAppMode.h"
#include "CEol.h"
#include "window/CEditWnd.h"
#include "debug/CRunningTimer.h"

CDocEditor::CDocEditor(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_bIsDocModified( false )	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
, m_pcOpeBlk( NULL )
{
	//	Oct. 2, 2005 genta �}�����[�h
	this->SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
}


/*! �ύX�t���O�̐ݒ�

	@param flag [in] �ݒ肷��l�Dtrue: �ύX�L�� / false: �ύX����
	@param redraw [in] true: �^�C�g���̍ĕ`����s�� / false: �s��Ȃ�
	
	@author genta
	@date 2002.01.22 �V�K�쐬
*/
void CDocEditor::SetModified( bool flag, bool redraw)
{
	if( m_bIsDocModified == flag )	//	�ύX���Ȃ���Ή������Ȃ�
		return;

	m_bIsDocModified = flag;
	if( redraw )
		m_pcDocRef->m_pcEditWnd->UpdateCaption();
}

void CDocEditor::OnBeforeLoad(SLoadInfo* sLoadInfo)
{
	//�r���[�̃e�L�X�g�I������
	GetListeningDoc()->m_pcEditWnd->Views_DisableSelectArea(true);
}

void CDocEditor::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	May 12, 2000 genta
	//	�ҏW�p���s�R�[�h�̐ݒ�
	{
		const STypeConfig& type = pcDoc->m_cDocType.GetDocumentAttribute();
		if ( pcDoc->m_cDocFile.GetCodeSet() == type.m_encoding.m_eDefaultCodetype ){
			SetNewLineCode( type.m_encoding.m_eDefaultEoltype );	// 2011.01.24 ryoji �f�t�H���gEOL
		}
		else{
			SetNewLineCode( EOL_CRLF );
		}
		CDocLine*	pFirstlineinfo = pcDoc->m_cDocLineMgr.GetLine( CLogicInt(0) );
		if( pFirstlineinfo != NULL ){
			EEolType t = pFirstlineinfo->GetEol();
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	this->SetImeMode( pcDoc->m_cDocType.GetDocumentAttribute().m_nImeState );

	// �J�����g�f�B���N�g���̕ύX
	::SetCurrentDirectory( pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str() );

	CAppMode::getInstance()->SetViewMode(sLoadInfo.bViewMode);		// �r���[���[�h	##�������A�A������
}

void CDocEditor::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	this->SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	this->m_cOpeBuf.SetNoModified();

	// �J�����g�f�B���N�g���̕ύX
	::SetCurrentDirectory( pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str() );
}



//	From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void CDocEditor::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( CEditWnd::getInstance()->GetHwnd() ); //######���v�H

	//	�ŉ��ʃr�b�g��IME���g��On/Off����
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( CEditWnd::getInstance()->GetHwnd(), hIme ); //######���v�H
}
//	To Here Nov. 20, 2000 genta










/*!
	�����ɍs��ǉ�

	@version 1.5

	@param pData    [in] �ǉ����镶����ւ̃|�C���^
	@param nDataLen [in] ������̒����B�����P�ʁB
	@param cEol     [in] �s���R�[�h

*/
void CDocEditAgent::AddLineStrX( const wchar_t* pData, int nDataLen )
{
	//�`�F�[���K�p
	CDocLine* pDocLine = m_pcDocLineMgr->AddNewLine();

	//�C���X�^���X�ݒ�
	pDocLine->SetDocLineString(pData, nDataLen);
}


