/*!	@file
	�L�[�{�[�h�}�N��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca, hor
	Copyright (C) 2003, �S, ryoji, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, zenryaku, FILE
	Copyright (C) 2006, �����
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "func/Funccode.h"
#include "CMacro.h"
#include "CControlTray.h"
#include "view/CEditView.h" //2002/2/10 aroka
#include "macro/CSMacroMgr.h" //2002/2/10 aroka
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "debug/Debug.h"
#include "OleTypes.h" //2003-02-21 �S
#include "io/CTextStream.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"

CMacro::CMacro( EFunctionCode nFuncID )
{
	m_nFuncID = nFuncID;
	m_pNext = NULL;
	m_pParamTop = m_pParamBot = NULL;
}

CMacro::~CMacro( void )
{
	CMacroParam* p = m_pParamTop;
	CMacroParam* del_p;
	while (p){
		del_p = p;
		p = p->m_pNext;
		delete[] del_p->m_pData;
		delete del_p;
	}
	return;
}

/*	�����̌^�U�蕪��
	�@�\ID�ɂ���āA���҂���^�͈قȂ�܂��B
	�����ŁA�����̌^���@�\ID�ɂ���ĐU�蕪���āAAddParam���܂��傤�B
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�������肷��̂��A�����ł��܂��U�蕪�����邱�Ƃ����҂��Ă��܂��B

	lParam�́AHandleCommand��param�ɒl��n���Ă���R�}���h�̏ꍇ�ɂ̂ݎg���܂��B
*/
void CMacro::AddLParam( LPARAM lParam, const CEditView* pcEditView )
{
	switch( m_nFuncID ){
	/*	������p�����[�^��ǉ� */
	case F_INSTEXT_W:
	case F_FILEOPEN:
	case F_EXECMD:
	case F_EXECEXTMACRO:
		{
			AddStringParam( (const wchar_t*)lParam );	//	lParam��ǉ��B
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_nExecFlgOpt;
			AddIntParam( lFlag );
		}
		break;

	case F_JUMP:	//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		{
			AddIntParam( pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_nLineNum );
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_bLineNumIsCRLF_ForJump		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddIntParam( lFlag );
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			AddIntParam( lFlag );
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	//	lParam��ǉ��B
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nPaste					? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSelectedArea					? 0x80 : 0x00;	//	�u�����鎞�͑I�ׂȂ�
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nReplaceTarget << 8;	//	8bit�V�t�g�i0x100�Ŋ|���Z�j
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll				? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam( lFlag );
		}
		break;
	case F_GREP:
		{
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	//	lParam��ǉ��B
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFiles[0] );	//	lParam��ǉ��B
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFolders[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder				? 0x01 : 0x00;
			//			���̕ҏW���̃e�L�X�g���猟������(0x02.������)
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase						? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp					? 0x08 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca ���ʌ݊����̂��߂̏���
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepOutputLine				? 0x20 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet << 8;
			AddIntParam( lFlag );
		}
		break;
	/*	���l�p�����[�^��ǉ� */
	case F_WCHAR:
		AddIntParam( lParam ); //�������R�[�h���n�����
		break;

	/*	�W�����p�����[�^��ǉ� */
	default:
		AddIntParam( lParam );
		break;
	}
}

/*	�����ɕ������ǉ��B
*/
void CMacro::AddStringParam( const WCHAR* szParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	�K�v�ȗ̈���m�ۂ��ăR�s�[�B
	int nLen = auto_strlen( szParam );
	param->m_pData = new WCHAR[nLen + 1];
	auto_memcpy(param->m_pData, szParam, nLen );
	param->m_pData[nLen] = LTEXT('\0');

	//	���X�g�̐�������ۂ�
	if (m_pParamTop){
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}
	else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

/*	�����ɐ��l��ǉ��B
*/
void CMacro::AddIntParam( const int nParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	�K�v�ȗ̈���m�ۂ��ăR�s�[�B
	param->m_pData = new WCHAR[16];	//	���l�i�[�i�ő�16���j�p
	_itow(nParam, param->m_pData, 10);

	//	���X�g�̐�������ۂ�
	if (m_pParamTop){
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}
	else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

/*	�R�}���h�����s����ipcEditView->GetCommander().HandleCommand�𔭍s����j
	m_nFuncID�ɂ���āA�����̌^�𐳊m�ɓn���Ă����܂��傤�B
	
	paramArr�͉����̃|�C���^�i�A�h���X�j��LONG�ł���킵���l�ɂȂ�܂��B
	������char*�̂Ƃ��́AparamArr[i]�����̂܂�HandleCommand�ɓn���Ă��܂��܂���B
	������int�̂Ƃ��́A*((int*)paramArr[i])�Ƃ��ēn���܂��傤�B
	
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�A4�߂̈����������B�������肷��ꍇ�́A���̂悤�ɂ��܂��傤�B
	pcEditView->GetCommander().HandleCommand( m_nFuncID, TRUE, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
*/
void CMacro::Exec( CEditView* pcEditView ) const
{
	const WCHAR* paramArr[4] = {NULL, NULL, NULL, NULL};	//	4�Ɍ���B
	
	CMacroParam* p = m_pParamTop;
	int i = 0;
	for (i = 0; i < 4; i++) {
		if (!p) break;	//	p���������break;
		paramArr[i] = p->m_pData;
		p = p->m_pNext;
	}
	CMacro::HandleCommand(pcEditView, m_nFuncID, paramArr, i);
}

/*	CMacro���Č����邽�߂̏���hFile�ɏ����o���܂��B

	InsText("�Ȃ�Ƃ�");
	�̂悤�ɁB
*/
void CMacro::Save( HINSTANCE hInstance, CTextOutputStream& out ) const
{
	WCHAR			szFuncName[1024];
	WCHAR			szFuncNameJapanese[500];
	int				nTextLen;
	const WCHAR*	pText;
	CNativeW		cmemWork;

	/* 2002.2.2 YAZAKI CSMacroMgr�ɗ��� */
	if (CSMacroMgr::GetFuncInfoByID( hInstance, m_nFuncID, szFuncName, szFuncNameJapanese)){
		switch ( m_nFuncID ){
		case F_INSTEXT_W:
		case F_FILEOPEN:
		case F_EXECEXTMACRO:
			//	�����ЂƂ������ۑ�
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF(
				LTEXT("S_%ls(\'%ls\');\t// %ls\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				szFuncNameJapanese
			);
			break;
		case F_JUMP:		//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
			out.WriteF(
				LTEXT("S_%ls(%d, %d);\t// %ls\r\n"),
				szFuncName,
				(m_pParamTop->m_pData ? _wtoi(m_pParamTop->m_pData) : 1),
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		case F_SEARCH_NEXT:
		case F_SEARCH_PREV:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF(
				LTEXT("S_%ls(\'%ls\', %d);\t// %ls\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_EXECMD:
			//	�����ЂƂ������ۑ�
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF(
				LTEXT("S_%ls(\'%ls\', %d);\t// %ls\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_REPLACE:
		case F_REPLACE_ALL:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			{
				CNativeW cmemWork2(m_pParamTop->m_pNext->m_pData);
				cmemWork2.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork2.Replace( LTEXT("\'"), LTEXT("\\\'") );
				out.WriteF(
					LTEXT("S_%ls(\'%ls\', \'%ls\', %d);\t// %ls\r\n"),
					szFuncName,
					cmemWork.GetStringPtr(),
					cmemWork2.GetStringPtr(),
					m_pParamTop->m_pNext->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
			}
			break;
		case F_GREP:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			{
				CNativeW cmemWork2(m_pParamTop->m_pNext->m_pData);
				cmemWork2.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork2.Replace( LTEXT("\'"), LTEXT("\\\'") );

				CNativeW cmemWork3(m_pParamTop->m_pNext->m_pNext->m_pData);
				cmemWork3.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork3.Replace( LTEXT("\'"), LTEXT("\\\'") );
				out.WriteF(
					LTEXT("S_%ls(\'%ls\', \'%ls\', \'%ls\', %d);\t// %ls\r\n"),
					szFuncName,
					cmemWork.GetStringPtr(),
					cmemWork2.GetStringPtr(),
					cmemWork3.GetStringPtr(),
					m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
			}
			break;
		default:
			if( 0 == m_pParamTop ){
				out.WriteF( LTEXT("S_%ls();\t// %ls\r\n"), szFuncName, szFuncNameJapanese );
			}else{
				out.WriteF( LTEXT("S_%ls(%d);\t// %ls\r\n"), szFuncName, m_pParamTop->m_pData ? _wtoi(m_pParamTop->m_pData) : 0, szFuncNameJapanese );
			}
			break;
		}
		return;
	}
	out.WriteF( LTEXT("CMacro::GetFuncInfoByID()�ɁA�o�O������̂ŃG���[���o�܂���������������������\r\n") );
}

/*!	Macro�R�}���h��CEditView::HandleCommand�Ɉ����n���B

	�������Ȃ��}�N���������C�}�N����HandleCommand�ł̑Ή��������Œ�`����K�v������D

	@param pcEditView	[in]	����Ώ�EditView
	@param Index	[in] �@�\ ID
	@param Argument [in] ����
	@param ArgSize	[in] �����̐�
	
*/
void CMacro::HandleCommand(
	CEditView*			pcEditView,
	const EFunctionCode	Index,
	const WCHAR*		Argument[],
	const int			ArgSize
)
{
	const TCHAR EXEC_ERROR_TITLE[] = _T("Macro���s�G���[");
	switch (Index) 
	{
	case F_WCHAR:		//	�������́B���l�͕����R�[�h
	case F_IME_CHAR:	//	���{�����
	case F_CHGMOD_EOL:	//	���͉��s�R�[�h�w��BenumEOLType�̐��l���w��B2003.06.23 Moca
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("�}�����ׂ������R�[�h���w�肳��Ă��܂���D")
			);
			break;
		}
		/* NO BREAK */
	case F_TEXTWRAPMETHOD:	//	�e�L�X�g�̐܂�Ԃ����@�̎w��B���l�́A0x0�i�܂�Ԃ��Ȃ��j�A0x1�i�w�茅�Ő܂�Ԃ��j�A0x2�i�E�[�Ő܂�Ԃ��j	// 2008.05.30 nasukoji
	case F_GOLINETOP:	//	�s���Ɉړ��B���l�́A0x0�i�f�t�H���g�j�A0x1�i�󔒂𖳎����Đ擪�Ɉړ��j�A0x2�i����`�j�A0x4�i�I�����Ĉړ��j�A0x8�i���s�P�ʂŐ擪�Ɉړ��F�������j
		//	��ڂ̈��������l�B
		pcEditView->GetCommander().HandleCommand( Index, FALSE, (Argument[0] != NULL ? _wtoi(Argument[0]) : 0 ), 0, 0, 0 );
		break;
	case F_INSTEXT_W:		//	�e�L�X�g�}��
	case F_ADDTAIL_W:		//	���̑���̓L�[�{�[�h����ł͑��݂��Ȃ��̂ŕۑ����邱�Ƃ��ł��Ȃ��H
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta �ǉ� �e�L�X�g����1�����}�N���͂����ɓ������Ă������D
		//	��ڂ̈�����������B
		//	������2�ڂ̈����͕������B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("����(������)���w�肳��Ă��܂���D")
			);
			break;
		}
		{
			int len = wcslen(Argument[0]);
			pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], len, 0, 0 );	//	�W��
		}
		break;
	/* ��ځA��ڂƂ������͐��l */
	case F_JUMP:		//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		//	Argument[0]�փW�����v�B�I�v�V������Argument[1]�ɁB
		//		******** �ȉ��u�s�ԍ��̒P�ʁv ********
		//		0x00	�܂�Ԃ��P�ʂ̍s�ԍ�
		//		0x01	���s�P�ʂ̍s�ԍ�
		//		**************************************
		//		0x02	PL/SQL�R���p�C���G���[�s����������
		//		����`	�e�L�X�g�́��s�ڂ��u���b�N��1�s�ڂƂ���
		//		����`	���o���ꂽPL/SQL�p�b�P�[�W�̃u���b�N����I��
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("�W�����v��s�ԍ����w�肳��Ă��܂���D")
			);
			break;
		}
		{
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_nLineNum = _wtoi(Argument[0]);	//�W�����v��
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 1; // �f�t�H���g1
			GetDllShareData().m_bLineNumIsCRLF_ForJump = ((lFlag & 0x01)!=0);
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->GetCommander().HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	�W��
		}
		break;
	/*	��ڂ̈����͕�����A��ڂ̈����͐��l	*/
	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("�}�[�N�s�̃p�^�[�����w�肳��Ă��܂���D")
			);
			break;
		}
		/* NO BREAK */
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		//	Argument[0]�������B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		0x01	�P��P�ʂŒT��
		//		0x02	�p�啶���Ə���������ʂ���
		//		0x04	���K�\��
		//		0x08	������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��
		//		0x10	�����_�C�A���O�������I�ɕ���
		//		0x20	�擪�i�����j����Č�������
		//	�e�l��ShareData�ɐݒ肵�ăR�}���h�𔭍s���AShareData�̒l�����ɖ߂��B
		{
			if( 0 < wcslen( Argument[0] ) ){
				/* ���K�\�� */
				if( GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp
					&& !CheckRegexpSyntax( Argument[0], NULL, true )
				)
				{
					break;
				}

				/* ���������� */
				CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );
			}
			//	�ݒ�l�o�b�N�A�b�v
			//	�}�N���p�����[�^���ݒ�l�ϊ�
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly			= lFlag & 0x01 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase			= lFlag & 0x02 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp		= lFlag & 0x04 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			//	�R�}���h���s
		//	pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
			pcEditView->GetCommander().HandleCommand( Index, FALSE, 0, 0, 0, 0);
		}
		break;
	case F_DIFF:
		//	Argument[0]��Diff�����\���B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		���̐��l�̘a�B
		//		0x0001 -i ignore-case         �啶�����������ꎋ
		//		0x0002 -w ignore-all-space    �󔒖���
		//		0x0004 -b ignore-space-change �󔒕ύX����
		//		0x0008 -B ignore-blank-lines  ��s����
		//		0x0010 -t expand-tabs         TAB-SPACE�ϊ�
		//		0x0020    (�ҏW���̃t�@�C�������t�@�C��)
		//		0x0040    (DIFF�������Ȃ��Ƃ��Ƀ��b�Z�[�W�\��)
		/* NO BREAK */

	case F_EXECMD:
		//	Argument[0]�����s�B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		���̐��l�̘a�B
		//		0x01	�W���o�͂𓾂�
		//		0x02	�W���o�͂��L�����b�g�ʒu��	//	2007.01.02 maru �����̊g��
		//		0x04	�ҏW���t�@�C����W�����͂�	//	2007.01.02 maru �����̊g��
		/* NO BREAK */

	case F_TRACEOUT:		// 2006.05.01 �}�N���p�A�E�g�v�b�g�E�C���h�E�ɏo��
		//	Argument[0]���o�́B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		���̐��l�̘a�B
		//		0x01	ExpandParameter�ɂ�镶����W�J���s��
		//		0x02	�e�L�X�g�����ɉ��s�R�[�h��t�����Ȃ�
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T(	"����(������)���w�肳��Ă��܂���D" )
			);
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)(Argument[1] != NULL ? _wtoi(Argument[1]) : 0 ), 0, 0);
		}
		break;

	/* �͂��߂̈����͕�����B�Q�ڂƂR�ڂ͐��l */
	case F_PUTFILE:		// 2006.12.10 ��ƒ��t�@�C���̈ꎞ�o��
		//	Argument[0]�ɏo�́BArgument[1]�ɕ����R�[�h�B�I�v�V������Argument[2]�ɁB
		//	Argument[2]:
		//		���̒l�̘a
		//		0x01	�I��͈͂��o�́i��I����ԂȂ��t�@�C���𐶐��j
		// no break

	case F_INSFILE:		// 2006.12.10 �L�����b�g�ʒu�Ƀt�@�C���}��
		//	Argument[0]�ɏo�́BArgument[1]�ɕ����R�[�h�B�I�v�V������Argument[2]�ɁB
		//	Argument[2]:
		//		���݂͓��ɂȂ�
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�t�@�C�������w�肳��Ă��܂���D" ));
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand(
				Index,
				FALSE,
				(LPARAM)Argument[0], 
				(LPARAM)(Argument[1] != NULL ? _wtoi(Argument[1]) : 0 ),
				(LPARAM)(Argument[2] != NULL ? _wtoi(Argument[2]) : 0 ),
				0
			);
		}
		break;

	/* �͂��߂�2�̈����͕�����B3�ڂ͐��l */
	case F_REPLACE:
	case F_REPLACE_ALL:
		//	Argument[0]���AArgument[1]�ɒu���B�I�v�V������Argument[2]�Ɂi�����\��j
		//	Argument[2]:
		//		���̐��l�̘a�B
		//		0x001	�P��P�ʂŒT��
		//		0x002	�p�啶���Ə���������ʂ���
		//		0x004	���K�\��
		//		0x008	������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��
		//		0x010	�����_�C�A���O�������I�ɕ���
		//		0x020	�擪�i�����j����Č�������
		//		0x040	�N���b�v�{�[�h����\��t����
		//		******** �ȉ��u�u���͈́v ********
		//		0x000	�t�@�C���S��
		//		0x080	�I��͈�
		//		**********************************
		//		******** �ȉ��u�u���Ώہv ********
		//		0x000	��������������ƒu��
		//		0x100	��������������̑O�ɑ}��
		//		0x200	��������������̌�ɒǉ�
		//		**********************************
		//		0x400	�u���ׂĒu���v�͒u���̌J�Ԃ��iON:�A���u��, OFF:�ꊇ�u���j
		//	�e�l��ShareData�ɐݒ肵�ăR�}���h�𔭍s���AShareData�̒l�����ɖ߂��B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("�u�����p�^�[�����w�肳��Ă��܂���D"));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("�u����p�^�[�����w�肳��Ă��܂���D"));
			break;
		}
		{
			/* ���K�\�� */
			if( GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp
				&& !CheckRegexpSyntax( Argument[0], NULL, true )
			)
			{
				break;
			}

			/* ���������� */
			CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );

			/* ���������� */
			CSearchKeywordManager().AddToReplaceKeyArr( Argument[1] );

			LPARAM lFlag = Argument[2] != NULL ? _wtoi(Argument[2]) : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly			= lFlag & 0x01 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase			= lFlag & 0x02 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp		= lFlag & 0x04 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nPaste			= lFlag & 0x40 ? 1 : 0;	//	CShareData�ɓ���Ȃ��Ă����́H
//			GetDllShareData().m_Common.m_sSearch.m_bSelectedArea		= 0;	//	lFlag & 0x80 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll	= lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (Index == F_REPLACE) {
				//	�u�����鎞�͑I�ׂȂ�
				GetDllShareData().m_Common.m_sSearch.m_bSelectedArea	= 0;
			}
			else if (Index == F_REPLACE_ALL) {
				//	�S�u���̎��͑I�ׂ�H
				GetDllShareData().m_Common.m_sSearch.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	//	8bit�V�t�g�i0x100�Ŋ���Z�j	// 2007.01.16 ryoji ���� 2bit�������o��
			//	�R�}���h���s
			pcEditView->GetCommander().HandleCommand( Index, FALSE, 0, 0, 0, 0);
		}
		break;
	case F_GREP:
		//	Argument[0]	����������
		//	Argument[1]	�����Ώۂɂ���t�@�C����
		//	Argument[2]	�����Ώۂɂ���t�H���_��
		//	Argument[3]:
		//		���̐��l�̘a�B
		//		0x01	�T�u�t�H���_�������������
		//		0x02	���̕ҏW���̃e�L�X�g���猟������i�������j
		//		0x04	�p�啶���Ɖp����������ʂ���
		//		0x08	���K�\��
		//		0x10	�����R�[�h��������
		//		******** �ȉ��u���ʏo�́v ********
		//		0x00	�Y���s
		//		0x20	�Y������
		//		**********************************
		//		******** �ȉ��u�o�͌`���v ********
		//		0x00	�m�[�}��
		//		0x40	�t�@�C����
		//		**********************************
		//		0x0100 �` 0xff00	�����R�[�h�Z�b�g�ԍ� * 0x100
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("GREP�p�^�[�����w�肳��Ă��܂���D"));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("�t�@�C����ʂ��w�肳��Ă��܂���D"));
			break;
		}
		if( Argument[2] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("������t�H���_���w�肳��Ă��܂���D"));
			break;
		}
		{
			//	��ɊO���E�B���h�E�ɁB
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
			CNativeT cmWork1;	cmWork1.SetStringW( Argument[0] );	cmWork1.Replace( _T("\""), _T("\"\"") );	//	����������
			CNativeT cmWork2;	cmWork2.SetStringW( Argument[1] );	cmWork2.Replace( _T("\""), _T("\"\"") );	//	�t�@�C����
			CNativeT cmWork3;	cmWork3.SetStringW( Argument[2] );	cmWork3.Replace( _T("\""), _T("\"\"") );	//	�t�H���_��

			LPARAM lFlag = Argument[3] != NULL ? _wtoi(Argument[3]) : 5;

			// 2002/09/21 Moca �����R�[�h�Z�b�g
			ECodeType	nCharSet;
			{
				nCharSet = CODE_SJIS;
				if( lFlag & 0x10 ){	// �����R�[�h��������(���ʌ݊��p)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // ������ 7-15 �r�b�g��(0�J�n)���g��
				if( IsValidCodeTypeExceptSJIS(nCode) || CODE_AUTODETECT == nCode ){
					nCharSet = (ECodeType)nCode;
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			CCommandLineString cCmdLine;
			TCHAR	pOpt[64];
			cCmdLine.AppendF(
				_T("-GREPMODE -GKEY=\"%ls\" -GFILE=\"%ls\" -GFOLDER=\"%ls\" -GCODE=%d"),
				cmWork1.GetStringPtr(),
				cmWork2.GetStringPtr(),
				cmWork3.GetStringPtr(),
				nCharSet
			);

			//GOPT�I�v�V����
			pOpt[0] = '\0';
			if( lFlag & 0x01 )_tcscat( pOpt, _T("S") );	/* �T�u�t�H���_������������� */
			if( lFlag & 0x04 )_tcscat( pOpt, _T("L") );	/* �p�啶���Ɖp����������ʂ��� */
			if( lFlag & 0x08 )_tcscat( pOpt, _T("R") );	/* ���K�\�� */
			if( lFlag & 0x20 )_tcscat( pOpt, _T("P") );	/* �s���o�͂��邩�Y�����������o�͂��邩 */
			if( lFlag & 0x40 )_tcscat( pOpt, _T("2") );	/* Grep: �o�͌`�� */
			else _tcscat( pOpt, _T("1") );
			if( 0 < _tcslen( pOpt ) ){
				cCmdLine.AppendF( _T(" -GOPT=%ts"), pOpt );
			}

			/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
			SLoadInfo sLoadInfo;
			sLoadInfo.cFilePath = _T("");
			sLoadInfo.eCharCode = CODE_DEFAULT;
			sLoadInfo.bViewMode = false;
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				pcEditView->GetHwnd(),
				sLoadInfo,
				cCmdLine.c_str()
			);
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
		}
		break;
	case F_FILEOPEN:
		//	Argument[0]���J���B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("�ǂݍ��݃t�@�C�������w�肳��Ă��܂���D"));
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_FILESAVEAS:
		//	Argument[0]��ʖ��ŕۑ��B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("�ۑ��t�@�C�������w�肳��Ă��܂���D"));
			break;
		}
		{
			// �����R�[�h�Z�b�g
			//	Sep. 11, 2004 genta �����R�[�h�ݒ�͈̔̓`�F�b�N
			ECodeType nCharCode = CODE_AUTODETECT;	//�f�t�H���g�l
			if (Argument[1] != NULL){
				nCharCode = (ECodeType)_wtoi( Argument[1] );
			}
			if(	IsValidCodeType(nCharCode) && nCharCode != pcEditView->m_pcEditDoc->GetDocumentEncoding() ){
				pcEditView->m_pcEditDoc->SetDocumentEncoding(nCharCode);
				//	From Here Jul. 26, 2003 ryoji BOM��Ԃ�������
				switch( pcEditView->m_pcEditDoc->GetDocumentEncoding() ){
				case CODE_UNICODE:
				case CODE_UNICODEBE:
					pcEditView->m_pcEditDoc->m_cDocFile.SetBomMode(true);
					break;
				case CODE_UTF8:
				default:
					pcEditView->m_pcEditDoc->m_cDocFile.SetBomMode(false);
					break;
				}
				//	To Here Jul. 26, 2003 ryoji BOM��Ԃ�������
			}

			// ���s�R�[�h
			int nSaveLineCode = 0;	//�f�t�H���g�l	//Sep. 11, 2004 genta �����l���u�ύX���Ȃ��v��
			if (Argument[2] != NULL){
				nSaveLineCode = _wtoi( Argument[2] );
			}
			EEolType eEol;
			switch (nSaveLineCode){
			case 0:		eEol = EOL_NONE;	break;
			case 1:		eEol = EOL_CRLF;	break;
			case 2:		eEol = EOL_LF;		break;
			case 3:		eEol = EOL_CR;		break;
			default:	eEol = EOL_NONE;	break;
			}
			
			pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, (LPARAM)eEol, 0);
		}
		break;
	/* 2�̈����������� */
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2009.06.14 syat
		pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
		break;
	//	From Here Dec. 4, 2002 genta
	case F_FILE_REOPEN				://�J������
	case F_FILE_REOPEN_SJIS			://SJIS�ŊJ������
	case F_FILE_REOPEN_JIS			://JIS�ŊJ������
	case F_FILE_REOPEN_EUC			://EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE		://Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8			://UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7			://UTF-7�ŊJ������
		{
			int noconfirm = 0;
			if (Argument[0] != NULL){
				noconfirm = ( _wtoi( Argument[0] ) != 0 );
			}
			pcEditView->GetCommander().HandleCommand( Index, FALSE, noconfirm, 0, 0, 0 );
		}
		break;
	//	To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if( Argument[0] != NULL ){
				lparam1 = _wtoi( Argument[0] );
				pcEditView->GetCommander().HandleCommand( Index, FALSE, lparam1, 0, 0, 0 );
			}
		}
		break;	//	Jan. 29, 2005 genta �����Ă���
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			//������NULL�ł�OK
			pcEditView->GetCommander().HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		pcEditView->GetDocument()->HandleCommand( Index );	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOW�����삵�Ȃ������̂��C��
		break;
	default:
		//	�����Ȃ��B
		pcEditView->GetCommander().HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	�W��
		break;
	}
}

/*!	�l��Ԃ��֐�����������

	@param View      [in] �ΏۂƂȂ�View
	@param ID        [in] �֐��ԍ�
	@param Arguments [in] �����̔z��
	@param ArgSize   [in] �����̐�(Argument)
	@param Result  [out] ���ʂ̒l��Ԃ��ꏊ�B�߂�l��false�̂Ƃ��͕s��B
	
	@return true: ����, false: ���s

	@author �S
	@date 2003.02.21 �S
	@date 2003.06.01 Moca �֐��ǉ�
	@date 2005.08.05 maru,zenryaku �֐��ǉ�
	@date 2005.11.29 FILE VariantChangeType�Ή�
*/
bool CMacro::HandleFunction(CEditView *View, int ID, VARIANT *Arguments, int ArgSize, VARIANT &Result)
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

	//2003-02-21 �S
	switch(ID)
	{
	case F_GETFILENAME:
		{
			const TCHAR* FileName = View->m_pcEditDoc->m_cDocFile.GetFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if(View->GetSelectionInfo().IsTextSelected())
			{
				CNativeW cMem;
				if(!View->GetSelectedData(&cMem, FALSE, NULL, FALSE, FALSE)) return false;
				SysString S(cMem.GetStringPtr(), cMem.GetStringLength());
				Wrap(&Result)->Receive(S);
			}
			else
			{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_EXPANDPARAMETER:
		// 2003.02.24 Moca
		{
			if(ArgSize != 1) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			//void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);
			//pszSource��W�J���āApszBuffer�ɃR�s�[
			wchar_t *Source;
			int SourceLength;
			Wrap(&varCopy.Data.bstrVal)->GetW(&Source, &SourceLength);
			wchar_t Buffer[2048];
			CSakuraEnvironment::ExpandParameter(Source, Buffer, 2047);
			delete[] Source;
			SysString S(Buffer, wcslen(Buffer));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETLINESTR:
		//	2003.06.01 Moca �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			if( -1 < varCopy.Data.lVal ){
				const wchar_t *Buffer;
				CLogicInt nLength;
				CLogicInt nLine;
				if( 0 == varCopy.Data.lVal ){
					nLine = View->GetCaret().GetCaretLogicPos().GetY2();
				}else{
					nLine = CLogicInt(varCopy.Data.lVal - 1);
				}
				Buffer = View->m_pcEditDoc->m_cDocLineMgr.GetLine(nLine)->GetDocLineStrWithEOL(&nLength);
				if( Buffer != NULL ){
					SysString S( Buffer, nLength );
					Wrap( &Result )->Receive( S );
				}else{
					Result.vt = VT_BSTR;
					Result.bstrVal = SysAllocString(L"");
				}
			}else{
				return false;
			}
		}
		return true;
	case F_GETLINECOUNT:
		//	2003.06.01 Moca �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			if( 0 == varCopy.Data.lVal ){
				int nLineCount;
				nLineCount = View->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				Wrap( &Result )->Receive( nLineCount );
			}else{
				return false;
			}
		}
		return true;
	case F_CHGTABWIDTH:
		//	2004.03.16 zenryaku �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			Wrap( &Result )->Receive( (Int)View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			View->m_pcEditDoc->m_pcEditWnd->ChangeLayoutParam(
				false, 
				CLayoutInt(varCopy.Data.iVal),
				View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()
			);
		}
		return true;
	case F_ISTEXTSELECTED:
		//	2005.07.30 maru �}�N���ǉ�
		{
			if(View->GetSelectionInfo().IsTextSelected()) {
				if(View->GetSelectionInfo().IsBoxSelecting()) {
					Wrap( &Result )->Receive( 2 );	//��`�I��
				} else {
					Wrap( &Result )->Receive( 1 );	//�I��
				}
			} else {
				Wrap( &Result )->Receive( 0 );		//��I��
			}
		}
		return true;
	case F_GETSELLINEFROM:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetFrom().y + 1 );
		}
		return true;
	case F_GETSELCOLMFROM:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetFrom().x + 1 );
		}
		return true;
	case F_GETSELLINETO:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetTo().y + 1 );
		}
		return true;
	case F_GETSELCOLMTO:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetTo().x + 1);
		}
		return true;
	case F_ISINSMODE:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive(View->IsInsMode() /* Oct. 2, 2005 genta */);
		}
		return true;
	case F_GETCHARCODE:
		//	2005.07.31 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive(View->m_pcEditDoc->GetDocumentEncoding());
		}
		return true;
	case F_GETLINECODE:
		//	2005.08.04 maru �}�N���ǉ�
		{
			switch( View->m_pcEditDoc->m_cDocEditor.GetNewLineCode() ){
			case EOL_CRLF:
				Wrap( &Result )->Receive( 0 );
				break;
			case EOL_CR:
				Wrap( &Result )->Receive( 1 );
				break;
			case EOL_LF:
				Wrap( &Result )->Receive( 2 );
				break;
			}
		}
		return true;
	case F_ISPOSSIBLEUNDO:
		//	2005.08.04 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cDocEditor.IsEnableUndo() );
		}
		return true;
	case F_ISPOSSIBLEREDO:
		//	2005.08.04 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cDocEditor.IsEnableRedo() );
		}
		return true;
	case F_CHGWRAPCOLM:
		//	2008.06.19 ryoji �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			Wrap( &Result )->Receive( (Int)View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() );
			if( varCopy.Data.iVal < MINLINEKETAS || varCopy.Data.iVal > MAXLINEKETAS )
				return true;
			View->m_pcEditDoc->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
			View->m_pcEditDoc->m_bTextWrapMethodCurTemp = !( View->m_pcEditDoc->m_nTextWrapMethodCur == View->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
			View->m_pcEditDoc->m_pcEditWnd->ChangeLayoutParam(
				false, 
				View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace(),
				CLayoutInt(varCopy.Data.iVal)
			);
		}
		return true;
	default:
		return false;
	}
}


