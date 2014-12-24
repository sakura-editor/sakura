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
	Copyright (C) 2006, �����, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2011, syat

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
#include "func/Funccode.h"
#include "CMacro.h"
#include "_main/CControlTray.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h" //2002/2/10 aroka
#include "macro/CSMacroMgr.h" //2002/2/10 aroka
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "_os/OleTypes.h" //2003-02-21 �S
#include "io/CTextStream.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"
#include "dlg/CDlgInput1.h"
#include "dlg/CDlgOpenFile.h"
#include "util/format.h"
#include "util/shell.h"
#include "uiparts/CWaitCursor.h"

CMacro::CMacro( EFunctionCode nFuncID )
{
	m_nFuncID = nFuncID;
	m_pNext = NULL;
	m_pParamTop = m_pParamBot = NULL;
}

CMacro::~CMacro( void )
{
	ClearMacroParam();
}

void CMacro::ClearMacroParam()
{
	CMacroParam* p = m_pParamTop;
	CMacroParam* del_p;
	while (p){
		del_p = p;
		p = p->m_pNext;
		delete del_p;
	}
	m_pParamTop = NULL;
	m_pParamBot = NULL;
	return;
}

/*	�����̌^�U�蕪��
	�@�\ID�ɂ���āA���҂���^�͈قȂ�܂��B
	�����ŁA�����̌^���@�\ID�ɂ���ĐU�蕪���āAAddParam���܂��傤�B
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�������肷��̂��A�����ł��܂��U�蕪�����邱�Ƃ����҂��Ă��܂��B

	lParam�́AHandleCommand��param�ɒl��n���Ă���R�}���h�̏ꍇ�ɂ̂ݎg���܂��B
*/
void CMacro::AddLParam( const LPARAM* lParams, const CEditView* pcEditView )
{
	LPARAM lParam = lParams[0];
	switch( m_nFuncID ){
	/*	������p�����[�^��ǉ� */
	case F_INSTEXT_W:
	case F_FILEOPEN:
	case F_EXECEXTMACRO:
		{
			AddStringParam( (const wchar_t*)lParam );	//	lParam��ǉ��B
		}
		break;

	case F_EXECMD:
		{
			AddStringParam( (const wchar_t*)lParam );	//	lParam��ǉ��B
			AddIntParam( (int)lParams[1] );
			if( lParams[2] != 0 ){
				AddStringParam( (const wchar_t*)lParams[2] );
			}
		}
		break;

	case F_JUMP:	//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		{
			AddIntParam( pcEditView->m_pcEditWnd->m_cDlgJump.m_nLineNum );
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_bLineNumIsCRLF_ForJump		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pcEditWnd->m_cDlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddIntParam( lFlag );
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam( pcEditView->m_strCurSearchKey.c_str() );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			AddIntParam( lFlag );
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam( pcEditView->m_strCurSearchKey.c_str() );	//	lParam��ǉ��B
			AddStringParam( pcEditView->m_pcEditWnd->m_cDlgReplace.m_strText2.c_str() );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			lFlag |= pcEditView->m_pcEditWnd->m_cDlgReplace.m_nPaste					? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSelectedArea					? 0x80 : 0x00;	//	�u�����鎞�͑I�ׂȂ�
			lFlag |= pcEditView->m_pcEditWnd->m_cDlgReplace.m_nReplaceTarget << 8;	//	8bit�V�t�g�i0x100�Ŋ|���Z�j
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll				? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam( lFlag );
		}
		break;
	case F_GREP_REPLACE:
	case F_GREP:
		{
			CDlgGrep* pcDlgGrep;
			CDlgGrepReplace* pcDlgGrepRep;
			if( F_GREP == m_nFuncID ){
				pcDlgGrep = &pcEditView->m_pcEditWnd->m_cDlgGrep;
				pcDlgGrepRep = NULL;
				AddStringParam( pcDlgGrep->m_strText.c_str() );
			}else{
				pcDlgGrep = pcDlgGrepRep = &pcEditView->m_pcEditWnd->m_cDlgGrepReplace;
				AddStringParam( pcDlgGrep->m_strText.c_str() );
				AddStringParam( pcEditView->m_pcEditWnd->m_cDlgGrepReplace.m_strText2.c_str() );
			}
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFiles[0] );	//	lParam��ǉ��B
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFolders[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder				? 0x01 : 0x00;
			//			���̕ҏW���̃e�L�X�g���猟������(0x02.������)
			lFlag |= pcDlgGrep->m_sSearchOption.bLoHiCase		? 0x04 : 0x00;
			lFlag |= pcDlgGrep->m_sSearchOption.bRegularExp	? 0x08 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca ���ʌ݊����̂��߂̏���
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepOutputLine				? 0x20 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle == 3)		? 0x80 : 0x00;
			ECodeType code = GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet;
			if( IsValidCodeType(code) || CODE_AUTODETECT == code ){
				lFlag |= code << 8;
			}
			lFlag |= pcDlgGrep->m_sSearchOption.bWordOnly								? 0x10000 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepOutputFileOnly			? 0x20000 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepOutputBaseFolder		? 0x40000 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepSeparateFolder			? 0x80000 : 0x00;
			if( F_GREP_REPLACE == m_nFuncID ){
				lFlag |= pcDlgGrepRep->m_bPaste											? 0x100000 : 0x00;
				lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepBackup				? 0x200000 : 0x00;
			}
			AddIntParam( lFlag );
			AddIntParam( code );
		}
		break;
	/*	���l�p�����[�^��ǉ� */
	case F_WCHAR:
	case F_CTRL_CODE:
		AddIntParam( lParam ); //�������R�[�h���n�����
		break;
	case F_CHGMOD_EOL:
		{
			// EOL�^�C�v�l���}�N�������l�ɕϊ�����	// 2009.08.18 ryoji
			int nFlag;
			switch( (int)lParam ){
			case EOL_CRLF:	nFlag = 1; break;
//			case EOL_LFCR:	nFlag = 2; break;
			case EOL_LF:	nFlag = 3; break;
			case EOL_CR:	nFlag = 4; break;
			case EOL_NEL:	nFlag = 5; break;
			case EOL_LS:	nFlag = 6; break;
			case EOL_PS:	nFlag = 7; break;
			default:		nFlag = 0; break;
			}
			AddIntParam( nFlag );
		}
		break;
	case F_SETFONTSIZE:
		{
			AddIntParam( lParam );
			AddIntParam( lParams[1] );
			AddIntParam( lParams[2] );
		}
		break;

	/*	�W�����p�����[�^��ǉ� */
	default:
		AddIntParam( lParam );
		break;
	}
}

void CMacroParam::SetStringParam( const WCHAR* szParam, int nLength )
{
	Clear();
	int nLen;
	if( nLength == -1 ){
		nLen = auto_strlen( szParam );
	}else{
		nLen = nLength;
	}
	m_pData = new WCHAR[nLen + 1];
	auto_memcpy( m_pData, szParam, nLen );
	m_pData[nLen] = LTEXT('\0');
	m_nDataLen = nLen;
	m_eType = EMacroParamTypeStr;
}

void CMacroParam::SetIntParam( const int nParam )
{
	Clear();
	m_pData = new WCHAR[16];	//	���l�i�[�i�ő�16���j�p
	_itow(nParam, m_pData, 10);
	m_nDataLen = auto_strlen(m_pData);
	m_eType = EMacroParamTypeInt;
}

/*	�����ɕ������ǉ��B
*/
void CMacro::AddStringParam( const WCHAR* szParam, int nLength )
{
	CMacroParam* param = new CMacroParam();

	param->SetStringParam( szParam, nLength );

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
	CMacroParam* param = new CMacroParam();

	param->SetIntParam( nParam );

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

/**	�R�}���h�����s����ipcEditView->GetCommander().HandleCommand�𔭍s����j
	m_nFuncID�ɂ���āA�����̌^�𐳊m�ɓn���Ă����܂��傤�B
	
	@note
	paramArr�͉����̃|�C���^�i�A�h���X�j��LONG�ł���킵���l�ɂȂ�܂��B
	������char*�̂Ƃ��́AparamArr[i]�����̂܂�HandleCommand�ɓn���Ă��܂��܂���B
	������int�̂Ƃ��́A*((int*)paramArr[i])�Ƃ��ēn���܂��傤�B
	
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�A4�߂̈����������B�������肷��ꍇ�́A���̂悤�ɂ��܂��傤�B
	pcEditView->GetCommander().HandleCommand( m_nFuncID, true, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
	
	@date 2007.07.20 genta : flags�ǉ��DFA_FROMMACRO��flags�Ɋ܂߂ēn�����̂Ƃ���D
		(1�R�}���h���s���ɖ��񉉎Z����K�v�͂Ȃ��̂�)
*/
bool CMacro::Exec( CEditView* pcEditView, int flags ) const
{
	const int maxArg = 8;
	const WCHAR* paramArr[maxArg] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	int paramLenArr[maxArg] = {0, 0, 0, 0, 0, 0, 0, 0};

	CMacroParam* p = m_pParamTop;
	int i = 0;
	for (i = 0; i < maxArg; i++) {
		if (!p) break;	//	p���������break;
		paramArr[i] = p->m_pData;
		paramLenArr[i] = wcslen(paramArr[i]);
		p = p->m_pNext;
	}
	return CMacro::HandleCommand(pcEditView, (EFunctionCode)(m_nFuncID | flags), paramArr, paramLenArr, i);
}

WCHAR* CMacro::GetParamAt(CMacroParam* p, int index)
{
	CMacroParam* x = p;
	int i = 0;
	while(i < index){
		if( x == NULL ){
			return NULL;
		}
		x = x->m_pNext;
		i++;
	}
	if( x == NULL ){
		return NULL;
	}
	return x->m_pData;
}

int CMacro::GetParamCount() const
{
	CMacroParam* p = m_pParamTop;
	int n = 0;
	while( p ){
		n++;
		p = p->m_pNext;
	}
	return n;
}

static inline int wtoi_def( const WCHAR* arg, int def_val )
{
	return (arg == NULL ? def_val: _wtoi(arg));
}

static inline const WCHAR* wtow_def( const WCHAR* arg, const WCHAR* def_val )
{
	return (arg == NULL ? def_val: arg);
}

/*	CMacro���Č����邽�߂̏���hFile�ɏ����o���܂��B

	InsText("�Ȃ�Ƃ�");
	�̂悤�ɁB
	AddLParam�ȊO��CKeyMacroMgr::LoadKeyMacro�ɂ���Ă�CMacro���쐬�����_�ɒ���
*/
void CMacro::Save( HINSTANCE hInstance, CTextOutputStream& out ) const
{
	WCHAR			szFuncName[1024];
	WCHAR			szFuncNameJapanese[500];
	int				nTextLen;
	const WCHAR*	pText;
	CNativeW		cmemWork;
	int nFuncID = m_nFuncID;

	/* 2002.2.2 YAZAKI CSMacroMgr�ɗ��� */
	if (CSMacroMgr::GetFuncInfoByID( hInstance, nFuncID, szFuncName, szFuncNameJapanese)){
		// 2014.01.24 Moca �}�N�������o����m_eType��ǉ����ē���
		out.WriteF( L"S_%ls(", szFuncName );
		CMacroParam* pParam = m_pParamTop;
		while( pParam ){
			if( pParam != m_pParamTop ){
				out.WriteString( L", " );
			}
			switch( pParam->m_eType ){
			case EMacroParamTypeInt:
				out.WriteString( pParam->m_pData );
				break;
			case EMacroParamTypeStr:
				pText = pParam->m_pData;
				nTextLen = pParam->m_nDataLen;
				cmemWork.SetString( pText, nTextLen );
				cmemWork.Replace( L"\\", L"\\\\" );
				cmemWork.Replace( L"\'", L"\\\'" );
				cmemWork.Replace( L"\r", L"\\\r" );
				cmemWork.Replace( L"\n", L"\\\n" );
				cmemWork.Replace( L"\t", L"\\\t" );
				cmemWork.Replace( L"\0", 1, L"\\u0000", 6 );
				cmemWork.Replace( L"\u85", L"\\u0085" );
				cmemWork.Replace( L"\u2028", L"\\u2028" );
				cmemWork.Replace( L"\u2029", L"\\u2029" );
				for( int c = 0; c < 0x20; c++ ){
					int nLen = cmemWork.GetStringLength();
					const wchar_t* p = cmemWork.GetStringPtr();
					for( int i = 0; i < nLen; i++ ){
						if( p[i] == c ){
							wchar_t from[2];
							wchar_t to[7];
							from[0] = c;
							from[1] = L'\0';
							auto_sprintf( to, L"\\u%4x", c );
							cmemWork.Replace( from, to );
							break;
						}
					}
				}
				cmemWork.Replace( L"\u7f", L"\\u007f" );
				out.WriteString( L"'" );
				out.WriteString( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );
				out.WriteString( L"'" );
				break;
			}
			pParam = pParam->m_pNext;
		}
		out.WriteF( L");\t// %ls\r\n", szFuncNameJapanese );
		return;
	}
	out.WriteF( LSW(STR_ERR_DLGMACRO01) );
}

/**	�}�N�������ϊ�

	Macro�R�}���h��pcEditView->GetCommander().HandleCommand�Ɉ����n���D
	�������Ȃ��}�N���������C�}�N����HandleCommand�ł̑Ή��������Œ�`����K�v������D

	@param pcEditView	[in]	����Ώ�EditView
	@param Index	[in] ����16bit: �@�\ID, ��ʃ��[�h�͂��̂܂�CMacro::HandleCommand()�ɓn���D
	@param Argument [in] ����
	@param ArgSize	[in] �����̐�
	
	@date 2007.07.08 genta Index�̃R�}���h�ԍ������ʃ��[�h�ɐ���
*/
bool CMacro::HandleCommand(
	CEditView*			pcEditView,
	const EFunctionCode	Index,
	const WCHAR*		Argument[],
	const int			ArgLengths[],
	const int			ArgSize
)
{
	std::tstring EXEC_ERROR_TITLE_string = LS(STR_ERR_DLGMACRO02);
	const TCHAR* EXEC_ERROR_TITLE = EXEC_ERROR_TITLE_string.c_str();

	switch ( LOWORD(Index) ) 
	{
	case F_WCHAR:		//	�������́B���l�͕����R�[�h
	case F_IME_CHAR:	//	���{�����
	case F_CTRL_CODE:
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03)
			);
			return false;
		}
	case F_PASTE:	// 2011.06.26 Moca
	case F_PASTEBOX:	// 2011.06.26 Moca
	case F_TEXTWRAPMETHOD:	//	�e�L�X�g�̐܂�Ԃ����@�̎w��B���l�́A0x0�i�܂�Ԃ��Ȃ��j�A0x1�i�w�茅�Ő܂�Ԃ��j�A0x2�i�E�[�Ő܂�Ԃ��j	// 2008.05.30 nasukoji
	case F_GOLINETOP:	//	�s���Ɉړ��B���l�́A0x0�i�f�t�H���g�j�A0x1�i�󔒂𖳎����Đ擪�Ɉړ��j�A0x2�i����`�j�A0x4�i�I�����Ĉړ��j�A0x8�i���s�P�ʂŐ擪�Ɉړ��j
	case F_GOLINETOP_SEL:
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND:	//	�s���Ɉړ�
	case F_GOLINEEND_SEL:
	case F_GOLINEEND_BOX:
	case F_SELECT_COUNT_MODE:	//	�����J�E���g�̕��@���w��B���l�́A0x0�i�ύX�����擾�̂݁j�A0x1�i�������j�A0x2�i�o�C�g���j�A0x3�i�������̃o�C�g���g�O���j	// 2009.07.06 syat
	case F_OUTLINE:	//	�A�E�g���C����͂̃A�N�V�������w��B���l�́A0x0�i��ʕ\���j�A0x1�i��ʕ\�����ĉ�́j�A0x2�i��ʕ\���g�O���j
	case F_CHANGETYPE:
	case F_TOGGLE_KEY_SEARCH:
		//	��ڂ̈��������l�B
	case F_WHEELUP:
	case F_WHEELDOWN:
	case F_WHEELLEFT:
	case F_WHEELRIGHT:
	case F_WHEELPAGEUP:
	case F_WHEELPAGEDOWN:
	case F_WHEELPAGELEFT:
	case F_WHEELPAGERIGHT:
		pcEditView->GetCommander().HandleCommand( Index, true, (Argument[0] != NULL ? _wtoi(Argument[0]) : 0 ), 0, 0, 0 );
		break;
	case F_CHGMOD_EOL:	//	���͉��s�R�[�h�w��BEEolType�̐��l���w��B2003.06.23 Moca
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03_1)
			);
			return false;
		}
		{
			// �}�N�������l��EOL�^�C�v�l�ɕϊ�����	// 2009.08.18 ryoji
			int nEol;
			switch( Argument[0] != NULL ? _wtoi(Argument[0]) : 0 ){
			case 1:		nEol = EOL_CRLF; break;
//			case 2:		nEol = EOL_LFCR; break;
			case 3:		nEol = EOL_LF; break;
			case 4:		nEol = EOL_CR; break;
			case 5:		nEol = EOL_NEL; break;
			case 6:		nEol = EOL_LS; break;
			case 7:		nEol = EOL_PS; break;
			default:	nEol = EOL_NONE; break;
			}
			if( nEol != EOL_NONE ){
				pcEditView->GetCommander().HandleCommand( Index, true, nEol, 0, 0, 0 );
			}
		}
		break;
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta �ǉ� �e�L�X�g����1�����}�N���͂����ɓ������Ă������D
		{
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO04)
			);
			return false;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], 0, 0, 0 );	//	�W��
		}
		}
		break;
	case F_INSTEXT_W:		//	�e�L�X�g�}��
	case F_ADDTAIL_W:		//	���̑���̓L�[�{�[�h����ł͑��݂��Ȃ��̂ŕۑ����邱�Ƃ��ł��Ȃ��H
	case F_INSBOXTEXT:
		//	��ڂ̈�����������B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO04)
			);
			return false;
		}
		{
			int len = ArgLengths[0];
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], len, 0, 0 );	//	�W��
		}
		break;
	/* ��ځA��ڂƂ������͐��l */
	case F_CHG_CHARSET:
		{
			int		nCharSet = ( Argument[0] == NULL || Argument[0][0] == '\0' ) ? CODE_NONE : _wtoi(Argument[0]);
			BOOL	bBOM = ( Argument[1] == NULL ) ? FALSE : (_wtoi(Argument[1]) != 0);
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)nCharSet, (LPARAM)bBOM, 0, 0 );
		}
		break;
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
				LS(STR_ERR_DLGMACRO05)
			);
			return false;
		}
		{
			pcEditView->m_pcEditWnd->m_cDlgJump.m_nLineNum = _wtoi(Argument[0]);	//�W�����v��
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 1; // �f�t�H���g1
			GetDllShareData().m_bLineNumIsCRLF_ForJump = ((lFlag & 0x01)!=0);
			pcEditView->m_pcEditWnd->m_cDlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->GetCommander().HandleCommand( Index, true, 0, 0, 0, 0 );	//	�W��
		}
		break;
	/*	��ڂ̈����͕�����A��ڂ̈����͐��l	*/
	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO06)
			);
			return false;
		}
		/* NO BREAK */
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		//	Argument[0] �������B(�ȗ����A���̌���������E�I�v�V�������g��)
		//	Argument[1]:�I�v�V���� (�ȗ����A0�݂̂Ȃ�)
		//		0x01	�P��P�ʂŒT��
		//		0x02	�p�啶���Ə���������ʂ���
		//		0x04	���K�\��
		//		0x08	������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��
		//		0x10	�����_�C�A���O�������I�ɕ���
		//		0x20	�擪�i�����j����Č�������
		//		0x800	(�}�N����p)�����L�[�𗚗��ɓo�^���Ȃ�
		//		0x1000	(�}�N����p)�����I�v�V���������ɖ߂�
		{
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			SSearchOption sSearchOption;
			sSearchOption.bWordOnly			= (0 != (lFlag & 0x01));
			sSearchOption.bLoHiCase			= (0 != (lFlag & 0x02));
			sSearchOption.bRegularExp		= (0 != (lFlag & 0x04));
			bool bAddHistory = (0 == (lFlag & 0x800));
			bool bBackupFlag = (0 != (lFlag & 0x1000));
			CommonSetting_Search backupFlags;
			SSearchOption backupLocalFlags;
			std::wstring backupStr;
			bool backupKeyMark;
			int nBackupSearchKeySequence;
			if( bBackupFlag ){
				backupFlags = GetDllShareData().m_Common.m_sSearch;
				backupLocalFlags = pcEditView->m_sCurSearchOption;
				backupStr = pcEditView->m_strCurSearchKey;
				backupKeyMark = pcEditView->m_bCurSrchKeyMark;
				nBackupSearchKeySequence = pcEditView->m_nCurSearchKeySequence;
				bAddHistory = false;
			}
			const WCHAR* pszSearchKey = wtow_def(Argument[0], L"");
			int nLen = wcslen( pszSearchKey );
			if( 0 < nLen ){
				/* ���K�\�� */
				if( lFlag & 0x04
					&& !CheckRegexpSyntax( Argument[0], NULL, true )
				)
				{
					break;
				}

				/* ���������� */
				if( nLen < _MAX_PATH && bAddHistory ){
					CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );
					GetDllShareData().m_Common.m_sSearch.m_sSearchOption = sSearchOption;
				}
				pcEditView->m_strCurSearchKey = Argument[0];
				pcEditView->m_sCurSearchOption = sSearchOption;
				pcEditView->m_bCurSearchUpdate = true;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
			}
			//	�ݒ�l�o�b�N�A�b�v
			//	�}�N���p�����[�^���ݒ�l�ϊ�
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			//	�R�}���h���s
			pcEditView->GetCommander().HandleCommand( Index, true, 0, 0, 0, 0);
			if( bBackupFlag ){
				GetDllShareData().m_Common.m_sSearch = backupFlags;
				pcEditView->m_sCurSearchOption = backupLocalFlags;
				pcEditView->m_strCurSearchKey = backupStr;
				pcEditView->m_bCurSearchUpdate = true;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
				pcEditView->ChangeCurRegexp( backupKeyMark );
				pcEditView->m_bCurSrchKeyMark = backupKeyMark;
				if( !backupKeyMark ){
					pcEditView->Redraw();
				}
				pcEditView->m_nCurSearchKeySequence = nBackupSearchKeySequence;
			}
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
		//	Argument[2]:�J�����g�f�B���N�g��
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			int nOpt = wtoi_def(Argument[1], 0);
			const wchar_t* pDir = wtow_def(Argument[2], NULL);
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], nOpt, (LPARAM)pDir, 0 );
		}
		break;

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
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], ArgLengths[0], (LPARAM)(Argument[1] != NULL ? _wtoi(Argument[1]) : 0 ), 0);
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
				LS(STR_ERR_DLGMACRO08));
			return false;
		}
		{
			pcEditView->GetCommander().HandleCommand(
				Index,
				true,
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
		//		0x800	(�}�N����p)�����L�[�𗚗��ɓo�^���Ȃ�
		//		0x1000	(�}�N����p)�����I�v�V���������ɖ߂�
		if( Argument[0] == NULL || Argument[0][0] == L'\0' ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO09));
			return false;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO10));
			return false;
		}
		{
			CDlgReplace& cDlgReplace = pcEditView->m_pcEditWnd->m_cDlgReplace;
			LPARAM lFlag = Argument[2] != NULL ? _wtoi(Argument[2]) : 0;
			SSearchOption sSearchOption;
			sSearchOption.bWordOnly			= (0 != (lFlag & 0x01));
			sSearchOption.bLoHiCase			= (0 != (lFlag & 0x02));
			sSearchOption.bRegularExp		= (0 != (lFlag & 0x04));
			bool bAddHistory = (0 == (lFlag & 0x800));
			bool bBackupFlag = (0 != (lFlag & 0x1000));
			CommonSetting_Search backupFlags;
			SSearchOption backupLocalFlags;
			std::wstring backupStr;
			std::wstring backupStrRep;
			int nBackupSearchKeySequence;
			bool backupKeyMark;
			if( bBackupFlag ){
				backupFlags = GetDllShareData().m_Common.m_sSearch;
				backupLocalFlags = pcEditView->m_sCurSearchOption;
				backupStr = pcEditView->m_strCurSearchKey;
				backupStrRep = cDlgReplace.m_strText2;
				backupKeyMark = pcEditView->m_bCurSrchKeyMark;
				nBackupSearchKeySequence = pcEditView->m_nCurSearchKeySequence;
				bAddHistory = false;
			}
			/* ���K�\�� */
			if( lFlag & 0x04
				&& !CheckRegexpSyntax( Argument[0], NULL, true )
			)
			{
				break;
			}

			/* ���������� */
			if( wcslen(Argument[0]) < _MAX_PATH && bAddHistory ){
				CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );
				GetDllShareData().m_Common.m_sSearch.m_sSearchOption = sSearchOption;
			}
			pcEditView->m_strCurSearchKey = Argument[0];
			pcEditView->m_sCurSearchOption = sSearchOption;
			pcEditView->m_bCurSearchUpdate = true;
			pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

			/* �u���㕶���� */
			if( wcslen(Argument[1]) < _MAX_PATH && bAddHistory ){
				CSearchKeywordManager().AddToReplaceKeyArr( Argument[1] );
			}
			cDlgReplace.m_strText2 = Argument[1];

			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			cDlgReplace.m_nPaste			= lFlag & 0x40 ? 1 : 0;	//	CShareData�ɓ���Ȃ��Ă����́H
			cDlgReplace.m_bConsecutiveAll = lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (LOWORD(Index) == F_REPLACE) {	// 2007.07.08 genta �R�}���h�͉��ʃ��[�h
				//	�u�����鎞�͑I�ׂȂ�
				cDlgReplace.m_bSelectedArea = 0;
			}
			else if (LOWORD(Index) == F_REPLACE_ALL) {	// 2007.07.08 genta �R�}���h�͉��ʃ��[�h
				//	�S�u���̎��͑I�ׂ�H
				cDlgReplace.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	//	8bit�V�t�g�i0x100�Ŋ���Z�j	// 2007.01.16 ryoji ���� 2bit�������o��
			if( bAddHistory ){
				GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll = cDlgReplace.m_bConsecutiveAll;
				GetDllShareData().m_Common.m_sSearch.m_bSelectedArea = cDlgReplace.m_bSelectedArea;
			}
			//	�R�}���h���s
			pcEditView->GetCommander().HandleCommand( Index, true, 0, 0, 0, 0);
			if( bBackupFlag ){
				GetDllShareData().m_Common.m_sSearch = backupFlags;
				pcEditView->m_sCurSearchOption = backupLocalFlags;
				pcEditView->m_strCurSearchKey = backupStr;
				pcEditView->m_bCurSearchUpdate = true;
				cDlgReplace.m_strText2 = backupStrRep;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
				pcEditView->ChangeCurRegexp( backupKeyMark );
				pcEditView->m_bCurSrchKeyMark = backupKeyMark;
				if( !backupKeyMark ){
					pcEditView->Redraw();
				}
				pcEditView->m_nCurSearchKeySequence = nBackupSearchKeySequence;
			}
		}
		break;
	case F_GREP_REPLACE:
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
		//		0x80	���ʂ̂� // 2011.11.24
		//		0xC0	(���g�p) // 2011.11.24
		//		**********************************
		//		0x0100 �` 0xff00	�����R�[�h�Z�b�g�ԍ� * 0x100
		//		0x010000	�P��P�ʂŒT��
		//		0x020000	�t�@�C�����ŏ��̂݌���
		//		0x040000	�x�[�X�t�H���_�\��
		//		0x080000	�t�H���_���ɕ\��
		{
			if( Argument[0] == NULL ){
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO11));
				return false;
			}
			int ArgIndex = 0;
			bool bGrepReplace = LOWORD(Index) == F_GREP_REPLACE;
			if( bGrepReplace ){
				if( ArgLengths[0] == 0 ){
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO11));
					return false;
				}
				ArgIndex = 1;
				if( Argument[1] == NULL ){
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO17));
					return false;
				}
			}
			if( Argument[ArgIndex+1] == NULL ){
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO12));
				return false;
			}
			if( Argument[ArgIndex+2] == NULL ){
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO13));
				return false;
			}
			//	��ɊO���E�B���h�E�ɁB
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
			CNativeW cmWork1;	cmWork1.SetString( Argument[0] );	cmWork1.Replace( L"\"", L"\"\"" );	//	����������
			CNativeW cmWork4;
			if( bGrepReplace ){
				cmWork4.SetString( Argument[1] );	cmWork4.Replace( L"\"", L"\"\"" );	//	�u����
			}
			CNativeT cmWork2;	cmWork2.SetStringW( Argument[ArgIndex+1] );	cmWork2.Replace( _T("\""), _T("\"\"") );	//	�t�@�C����
			CNativeT cmWork3;	cmWork3.SetStringW( Argument[ArgIndex+2] );	cmWork3.Replace( _T("\""), _T("\"\"") );	//	�t�H���_��

			LPARAM lFlag = wtoi_def(Argument[ArgIndex+3], 5);

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
				// 2013.06.11 5�Ԗڂ̈������𕶎��R�[�h�ɂ���
				if( ArgIndex + 5 <= ArgSize ){
					nCharSet = (ECodeType)_wtoi(Argument[ArgIndex + 4]);
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			CNativeT cCmdLine;
			TCHAR	szTemp[20];
			TCHAR	pOpt[64];
			cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
			cCmdLine.AppendStringW(cmWork1.GetStringPtr());
			if( bGrepReplace ){
				cCmdLine.AppendString(_T("\" -GREPR=\""));
				cCmdLine.AppendStringW(cmWork4.GetStringPtr());
			}
			cCmdLine.AppendString(_T("\" -GFILE=\""));
			cCmdLine.AppendString(cmWork2.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GFOLDER=\""));
			cCmdLine.AppendString(cmWork3.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GCODE="));
			auto_sprintf( szTemp, _T("%d"), nCharSet );
			cCmdLine.AppendString(szTemp);

			//GOPT�I�v�V����
			pOpt[0] = '\0';
			if( lFlag & 0x01 )_tcscat( pOpt, _T("S") );	/* �T�u�t�H���_������������� */
			if( lFlag & 0x04 )_tcscat( pOpt, _T("L") );	/* �p�啶���Ɖp����������ʂ��� */
			if( lFlag & 0x08 )_tcscat( pOpt, _T("R") );	/* ���K�\�� */
			if( lFlag & 0x20 )_tcscat( pOpt, _T("P") );	/* �s���o�͂��邩�Y�����������o�͂��邩 */
			if(      0x40 == (lFlag & 0xC0) )_tcscat( pOpt, _T("2") );	/* Grep: �o�͌`�� */
			else if( 0x80 == (lFlag & 0xC0) )_tcscat( pOpt, _T("3") );
			else _tcscat( pOpt, _T("1") );
			if( lFlag & 0x10000 )_tcscat( pOpt, _T("W") );
			if( lFlag & 0x20000 )_tcscat( pOpt, _T("F") );
			if( lFlag & 0x40000 )_tcscat( pOpt, _T("B") );
			if( lFlag & 0x80000 )_tcscat( pOpt, _T("D") );
			if( bGrepReplace ){
				if( lFlag & 0x100000 )_tcscat( pOpt, _T("C") );
				if( lFlag & 0x200000 )_tcscat( pOpt, _T("O") );
			}
			if( pOpt[0] != _T('\0') ){
				auto_sprintf( szTemp, _T(" -GOPT=%ts"), pOpt );
				cCmdLine.AppendString(szTemp);
			}

			/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
			SLoadInfo sLoadInfo;
			sLoadInfo.cFilePath = _T("");
			sLoadInfo.eCharCode = CODE_NONE;
			sLoadInfo.bViewMode = false;
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				pcEditView->GetHwnd(),
				sLoadInfo,
				cCmdLine.GetStringPtr()
			);
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
		}
		break;
	case F_FILEOPEN2:
		//	Argument[0]���J���B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO14));
			return false;
		}
		{
			int  nCharCode = wtoi_def( Argument[1], CODE_AUTODETECT );
			BOOL nViewMode = wtoi_def( Argument[2], FALSE );
			const WCHAR* pDefFileName = wtow_def( Argument[3], L"" );
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], (LPARAM)nCharCode, (LPARAM)nViewMode, (LPARAM)pDefFileName );
		}
		break;
	case F_FILESAVEAS_DIALOG:
	case F_FILESAVEAS:
		//	Argument[0]��ʖ��ŕۑ��B
		if( LOWORD(Index) == F_FILESAVEAS && (Argument[0] == NULL ||  L'\0' == Argument[0][0]) ){
			// F_FILESAVEAS_DIALOG�̏ꍇ�͋󕶎�������e
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO15));
			return false;
		}
		{
			// �����R�[�h�Z�b�g
			//	Sep. 11, 2004 genta �����R�[�h�ݒ�͈̔̓`�F�b�N
			ECodeType nCharCode = CODE_NONE;	//�f�t�H���g�l
			if (Argument[1] != NULL){
				nCharCode = (ECodeType)_wtoi( Argument[1] );
			}
			if (LOWORD(Index) == F_FILESAVEAS && IsValidCodeOrCPType(nCharCode) && nCharCode != pcEditView->m_pcEditDoc->GetDocumentEncoding()) {
				//	From Here Jul. 26, 2003 ryoji BOM��Ԃ�������
				pcEditView->m_pcEditDoc->SetDocumentEncoding(nCharCode, CCodeTypeName(pcEditView->m_pcEditDoc->GetDocumentEncoding()).IsBomDefOn() );
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
			
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], (LPARAM)nCharCode, (LPARAM)eEol, 0);
		}
		break;
	/* 2�̈����������� */
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2009.06.14 syat
		pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
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
			pcEditView->GetCommander().HandleCommand( Index, true, noconfirm, 0, 0, 0 );
		}
		break;
	//	To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if( Argument[0] != NULL ){
				lparam1 = _wtoi( Argument[0] );
				pcEditView->GetCommander().HandleCommand( Index, true, lparam1, 0, 0, 0 );
			}
		}
		break;	//	Jan. 29, 2005 genta �����Ă���
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			//������NULL�ł�OK
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		pcEditView->GetDocument()->HandleCommand( Index );	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOW�����삵�Ȃ������̂��C��
		break;
	case F_MESSAGEBOX:	// ���b�Z�[�W�{�b�N�X�̕\��
	case F_ERRORMSG:	// ���b�Z�[�W�{�b�N�X�i�G���[�j�̕\��
	case F_WARNMSG:		// ���b�Z�[�W�{�b�N�X�i�x���j�̕\��
	case F_INFOMSG:		// ���b�Z�[�W�{�b�N�X�i���j�̕\��
	case F_OKCANCELBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�FOK�^�L�����Z���j�̕\��
	case F_YESNOBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�F�͂��^�������j�̕\��
		{
			VARIANT vArg[2];			// HandleFunction�ɓn������
			VARIANT vResult;			// HandleFunction����Ԃ�l
			if( Argument[0] == NULL ){
				break;
			}
			SysString S( Argument[0], wcslen(Argument[0]) );
			Wrap( &vArg[0] )->Receive( S );
			int nArgSize = 1;
			//	2�ڂ̈��������l�B
			if( F_MESSAGEBOX == LOWORD(Index) ){
				vArg[1].vt = VT_I4;
				vArg[1].intVal = (Argument[1] != NULL ? _wtoi(Argument[1]) : 0 );
				nArgSize = 2;
			}
			return HandleFunction( pcEditView, Index, vArg, nArgSize, vResult );
		}
	case F_MOVECURSORLAYOUT:
	case F_MOVECURSOR:
		{
			if( Argument[0] != NULL && Argument[1] != NULL && Argument[2] != NULL ){
				int lparam1 = _wtoi( Argument[0] ) - 1;
				int lparam2 = _wtoi( Argument[1] ) - 1;
				int lparam3 = _wtoi( Argument[2] );
				pcEditView->GetCommander().HandleCommand( Index, true, lparam1, lparam2, lparam3, 0);
			}else{
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO16));
				return false;
			}
		}
		break;
	case F_CHGTABWIDTH:		//  �^�u�T�C�Y���擾�A�ݒ肷��i�L�[�}�N���ł͎擾�͖��Ӗ��j
	case F_CHGWRAPCOLUMN:		//  �܂�Ԃ������擾�A�ݒ肷��i�L�[�}�N���ł͎擾�͖��Ӗ��j
	case F_MACROSLEEP:
	case F_SETDRAWSWITCH:	//  �ĕ`��X�C�b�`���擾�A�ݒ肷��
		{
			VARIANT vArg[1];			// HandleFunction�ɓn������
			VARIANT vResult;			// HandleFunction����Ԃ�l
			//	��ڂ̈��������l�B
			vArg[0].vt = VT_I4;
			vArg[0].intVal = (Argument[0] != NULL ? _wtoi(Argument[0]) : 0 );
			return HandleFunction( pcEditView, Index, vArg, 1, vResult );
		}
	case F_SETFONTSIZE:
		{
			int val0 = Argument[0] != NULL ? _wtoi(Argument[0]) : 0;
			int val1 = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			int val2 = Argument[2] != NULL ? _wtoi(Argument[2]) : 0;
			pcEditView->GetCommander().HandleCommand( Index, true, (LPARAM)val0, (LPARAM)val1, (LPARAM)val2, 0 );
		}
		break;
	case F_STATUSMSG:
		{
			if( Argument[0] == NULL ){
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO07) );
				return false;
			}
			std::tstring val0 = to_tchar(Argument[0]);
			int val1 = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			if( (val1 & 0x03) == 0 ){
				pcEditView->SendStatusMessage( val0.c_str() );
			}else if( (val1 & 0x03) == 1 ){
				if( NULL != pcEditView->m_pcEditWnd->m_cStatusBar.GetStatusHwnd() ){
					pcEditView->SendStatusMessage( val0.c_str() );
				}else{
					InfoMessage( pcEditView->GetHwnd(), _T("%ts"), val0.c_str() );
				}
			}else if( (val1 & 0x03) == 2 ){
				pcEditView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( val0.c_str() );
			}
		}
		break;
	case F_MSGBEEP:
		{
			int val0 = Argument[0] != NULL ? _wtoi(Argument[0]) : 0;
			switch( val0 ){
			case -1: break;
			case 0: val0 = MB_OK; break;
			case 1: val0 = MB_ICONERROR; break;
			case 2: val0 = MB_ICONQUESTION; break;
			case 3: val0 = MB_ICONWARNING; break;
			case 4: val0 = MB_ICONINFORMATION; break;
			default: val0 = MB_OK; break;
			}
			::MessageBeep( val0 );
		}
		break;
	case F_COMMITUNDOBUFFER:
		{
			COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
			if( opeBlk ){
				int nCount = opeBlk->GetRefCount();
				opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
				pcEditView->SetUndoBuffer();
				if( pcEditView->m_cCommander.GetOpeBlk() == NULL && 0 < nCount ){
					pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
					pcEditView->m_cCommander.GetOpeBlk()->SetRefCount( nCount );
				}
			}
		}
		break;
	case F_ADDREFUNDOBUFFER:
		{
			COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
			if( opeBlk == NULL ){
				pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
			}
			pcEditView->m_cCommander.GetOpeBlk()->AddRef();
		}
		break;
	case F_SETUNDOBUFFER:
		{
			pcEditView->SetUndoBuffer();
		}
		break;
	case F_APPENDUNDOBUFFERCURSOR:
		{
			COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
			if( opeBlk == NULL ){
				pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
			}
			opeBlk = pcEditView->m_cCommander.GetOpeBlk();
			opeBlk->AddRef();
			opeBlk->AppendOpe(
				new CMoveCaretOpe(
					pcEditView->GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
			pcEditView->SetUndoBuffer();
		}
		break;
	case F_CLIPBOARDEMPTY:
		{
			CClipboard cClipboard(pcEditView->GetHwnd());
			cClipboard.Empty();
		}
		break;
	default:
		//	�����Ȃ��B
		pcEditView->GetCommander().HandleCommand( Index, true, 0, 0, 0, 0 );	//	�W��
		break;
	}
	return true;
}


inline bool VariantToBStr(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(arg) ), 0, VT_BSTR) == S_OK;
}

inline bool VariantToI4(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(arg) ), 0, VT_I4) == S_OK;
}

/**	�l��Ԃ��֐�����������

	@param View      [in] �ΏۂƂȂ�View
	@param ID        [in] ����16bit: �֐��ԍ�
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
bool CMacro::HandleFunction(CEditView *View, EFunctionCode ID, const VARIANT *Arguments, int ArgSize, VARIANT &Result)
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

	//2003-02-21 �S
	switch(LOWORD(ID))
	{
	case F_GETFILENAME:
		{
			const TCHAR* FileName = View->m_pcEditDoc->m_cDocFile.GetFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSAVEFILENAME:
		//	2006.09.04 ryoji �ۑ����̃t�@�C���̃p�X
		{
			const TCHAR* FileName = View->m_pcEditDoc->m_cDocFile.GetSaveFilePath();
			SysString S(FileName, lstrlen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if(View->GetSelectionInfo().IsTextSelected())
			{
				CNativeW cMem;
				if(!View->GetSelectedDataSimple(cMem)) return false;
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
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
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
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
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
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
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
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			int nTab = (Int)View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
			Wrap( &Result )->Receive( nTab );
			// 2013.04.30 Moca �����ǉ��B�s�v�ȏꍇ��ChangeLayoutParam���Ă΂Ȃ�
			if( 0 < varCopy.Data.iVal && nTab != varCopy.Data.iVal ){
				View->GetDocument()->m_bTabSpaceCurTemp = true;
				View->m_pcEditWnd->ChangeLayoutParam(
					false, 
					CLayoutInt(varCopy.Data.iVal),
					View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()
				);

				// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I������TAB�����ύX���ꂽ��e�L�X�g�ő啝�̍ĎZ�o���K�v
				if( View->m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
					// �ő啝�̍ĎZ�o���Ɋe�s�̃��C�A�E�g���̌v�Z���s��
					View->m_pcEditDoc->m_cLayoutMgr.CalculateTextWidth();
				}
				View->m_pcEditWnd->RedrawAllViews( NULL );		// TAB�����ς�����̂ōĕ`�悪�K�v
			}
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
	case F_GETSELCOLUMNFROM:
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
	case F_GETSELCOLUMNTO:
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
			int n = 0;
			switch( View->m_pcEditDoc->m_cDocEditor.GetNewLineCode() ){
			case EOL_CRLF:
				n = 0;
				break;
			case EOL_CR:
				n = 1;
				break;
			case EOL_LF:
				n = 2;
				break;
			case EOL_NEL:
				n = 3;
				break;
			case EOL_LS:
				n = 4;
				break;
			case EOL_PS:
				n = 5;
				break;
			}
			Wrap( &Result )->Receive( n );
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
	case F_CHGWRAPCOLUMN:
		//	2008.06.19 ryoji �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			Wrap( &Result )->Receive( (Int)View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() );
			if( varCopy.Data.iVal < MINLINEKETAS || varCopy.Data.iVal > MAXLINEKETAS )
				return true;
			View->m_pcEditDoc->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
			View->m_pcEditDoc->m_bTextWrapMethodCurTemp = !( View->m_pcEditDoc->m_nTextWrapMethodCur == View->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
			View->m_pcEditWnd->ChangeLayoutParam(
				false, 
				View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace(),
				CLayoutInt(varCopy.Data.iVal)
			);
		}
		return true;
	case F_ISCURTYPEEXT:
		//	2006.09.04 ryoji �w�肵���g���q�����݂̃^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ�
		{
			if( ArgSize != 1 ) return false;

			TCHAR *Source;
			int SourceLength;

			int nType1 = View->m_pcEditDoc->m_cDocType.GetDocumentType().GetIndex();	// ���݂̃^�C�v

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType2 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// �w��g���q�̃^�C�v
			delete[] Source;

			Wrap( &Result )->Receive( (nType1 == nType2)? 1: 0 );	// �^�C�v�ʐݒ�̈�v�^�s��v
		}
		return true;
	case F_ISSAMETYPEEXT:
		//	2006.09.04 ryoji �Q�̊g���q�������^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ�
		{
			if( ArgSize != 2 ) return false;

			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType1 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// �g���q�P�̃^�C�v
			delete[] Source;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType2 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// �g���q�Q�̃^�C�v
			delete[] Source;

			Wrap( &Result )->Receive( (nType1 == nType2)? 1: 0 );	// �^�C�v�ʐݒ�̈�v�^�s��v
		}
		return true;
	case F_INPUTBOX:
		//	2011.03.18 syat �e�L�X�g���̓_�C�A���O�̕\��
		{
			if( ArgSize < 1 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sMessage = Source;	// �\�����b�Z�[�W
			delete[] Source;

			std::tstring sDefaultValue = _T("");
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefaultValue = Source;	// �f�t�H���g�l
				delete[] Source;
			}

			int nMaxLen = _MAX_PATH;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
				nMaxLen = varCopy.Data.intVal;	// �ő���͒�
				if( nMaxLen <= 0 ){
					nMaxLen = _MAX_PATH;
				}
			}

			TCHAR *Buffer = new TCHAR[ nMaxLen+1 ];
			_tcscpy( Buffer, sDefaultValue.c_str() );
			CDlgInput1 cDlgInput1;
			if( cDlgInput1.DoModal( G_AppInstance(), View->GetHwnd(), _T("sakura macro"), sMessage.c_str(), nMaxLen, Buffer ) ) {
				SysString S( Buffer, _tcslen(Buffer) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
			delete[] Buffer;
		}
		return true;
	case F_MESSAGEBOX:	// ���b�Z�[�W�{�b�N�X�̕\��
	case F_ERRORMSG:	// ���b�Z�[�W�{�b�N�X�i�G���[�j�̕\��
	case F_WARNMSG:		// ���b�Z�[�W�{�b�N�X�i�x���j�̕\��
	case F_INFOMSG:		// ���b�Z�[�W�{�b�N�X�i���j�̕\��
	case F_OKCANCELBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�FOK�^�L�����Z���j�̕\��
	case F_YESNOBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�F�͂��^�������j�̕\��
		//	2011.03.18 syat ���b�Z�[�W�{�b�N�X�̕\��
		{
			if( ArgSize < 1 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sMessage = Source;	// �\��������
			delete[] Source;

			UINT uType = 0;		// ���b�Z�[�W�{�b�N�X���
			switch( LOWORD(ID) ) {
			case F_MESSAGEBOX:
				if( ArgSize >= 2 ){
					if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
					uType = varCopy.Data.uintVal;
				}else{
					uType = MB_OK;
				}
				break;
			case F_ERRORMSG:
				uType |= MB_OK | MB_ICONSTOP;
				break;
			case F_WARNMSG:
				uType |= MB_OK | MB_ICONEXCLAMATION;
				break;
			case F_INFOMSG:
				uType |= MB_OK | MB_ICONINFORMATION;
				break;
			case F_OKCANCELBOX:
				uType |= MB_OKCANCEL | MB_ICONQUESTION;
				break;
			case F_YESNOBOX:
				uType |= MB_YESNO | MB_ICONQUESTION;
				break;
			}
			int ret = ::MessageBox( View->GetHwnd(), sMessage.c_str(), _T("sakura macro"), uType );
			Wrap( &Result )->Receive( ret );
		}
		return true;
	case F_COMPAREVERSION:
		//	2011.03.18 syat �o�[�W�����ԍ��̔�r
		{
			if( ArgSize != 2 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sVerA = Source;	// �o�[�W����A
			delete[] Source;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sVerB = Source;	// �o�[�W����B
			delete[] Source;

			Wrap( &Result )->Receive( CompareVersion( sVerA.c_str(), sVerB.c_str() ) );
		}
		return true;
	case F_MACROSLEEP:
		//	2011.03.18 syat �w�肵�����ԁi�~���b�j��~����
		{
			if( ArgSize != 1 ) return false;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_UI4) != S_OK) return false;	// VT_UI4�Ƃ��ĉ���
			CWaitCursor cWaitCursor( View->GetHwnd() );	// �J�[�\���������v�ɂ���
			::Sleep( varCopy.Data.uintVal );
			Wrap( &Result )->Receive( 0 );	//�߂�l�͍��̂Ƃ���0�Œ�
		}
		return true;
	case F_FILEOPENDIALOG:
	case F_FILESAVEDIALOG:
		//	2011.03.18 syat �t�@�C���_�C�A���O�̕\��
		{
			TCHAR *Source;
			int SourceLength;
			std::tstring sDefault;
			std::tstring sFilter;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefault = Source;	// ����̃t�@�C����
				delete[] Source;
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sFilter = Source;	// �t�B���^������
				delete[] Source;
			}

			CDlgOpenFile cDlgOpenFile;
			cDlgOpenFile.Create(
				G_AppInstance(), View->GetHwnd(),
				sFilter.c_str(),
				sDefault.c_str()
			);
			bool bRet;
			TCHAR szPath[ _MAX_PATH ];
			_tcscpy( szPath, sDefault.c_str() );
			if( LOWORD(ID) == F_FILEOPENDIALOG ){
				bRet = cDlgOpenFile.DoModal_GetOpenFileName( szPath );
			}else{
				bRet = cDlgOpenFile.DoModal_GetSaveFileName( szPath );
			}
			if( bRet ){
				SysString S( szPath, _tcslen(szPath) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_FOLDERDIALOG:
		//	2011.03.18 syat �t�H���_�_�C�A���O�̕\��
		{
			TCHAR *Source;
			int SourceLength;
			std::tstring sMessage;
			std::tstring sDefault;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sMessage = Source;	// �\�����b�Z�[�W
				delete[] Source;
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefault = Source;	// ����̃t�@�C����
				delete[] Source;
			}

			TCHAR szPath[ _MAX_PATH ];
			int nRet = SelectDir( View->GetHwnd(), sMessage.c_str(), sDefault.c_str(), szPath );
			if( nRet == IDOK ){
				SysString S( szPath, _tcslen(szPath) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_GETCLIPBOARD:
		//	2011.03.18 syat �N���b�v�{�[�h�̕�������擾
		{
			int nOpt = 0;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
				nOpt = varCopy.Data.intVal;	// �I�v�V����
			}

			CNativeW memBuff;
			bool bColumnSelect = false;
			bool bLineSelect = false;
			bool bRet = View->MyGetClipboardData( memBuff, &bColumnSelect, &bLineSelect );
			if( bRet ){
				SysString S( memBuff.GetStringPtr(), memBuff.GetStringLength() );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_SETCLIPBOARD:
		//	2011.03.18 syat �N���b�v�{�[�h�ɕ������ݒ�
		{
			std::tstring sValue;
			int nOpt = 0;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
				nOpt = varCopy.Data.intVal;	// �I�v�V����
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.Data.bstrVal)->GetT(&sValue);
			}

			// 2013.06.12 �I�v�V�����ݒ�
			bool bColumnSelect = ((nOpt & 0x01) == 0x01);
			bool bLineSelect = ((nOpt & 0x02) == 0x02);
			bool bRet = View->MySetClipboardData( sValue.c_str(), sValue.size(), bColumnSelect, bLineSelect );
			Wrap( &Result )->Receive( bRet );
		}
		return true;

	case F_LAYOUTTOLOGICLINENUM:
		// ���C�A�E�g�����W�b�N�s
		{
			if( ArgSize < 1 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLayoutInt nLineNum = CLayoutInt(varCopy.Data.lVal - 1);
			int ret = 0;
			if( View->m_pcEditDoc->m_cLayoutMgr.GetLineCount() == nLineNum ){
				ret = (Int)View->m_pcEditDoc->m_cDocLineMgr.GetLineCount() + 1;
			}else{
				const CLayout* pcLayout = View->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(nLineNum);
				if( pcLayout != NULL ){
					ret = pcLayout->GetLogicLineNo() + 1;
				}else{
					return false;
				}
			}
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_LINECOLUMNTOINDEX:
		// ���C�A�E�g�����W�b�N��
		{
			if( ArgSize < 2 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLayoutInt nLineNum = CLayoutInt(varCopy.Data.lVal - 1);
			if( !VariantToI4(varCopy, Arguments[1]) ){
				return false;
			}
			CLayoutInt nLineCol = CLayoutInt(varCopy.Data.lVal - 1);
			if( nLineNum < 0 ){
				return false;
			}

			CLayoutPoint nLayoutPos(nLineCol, nLineNum);
			CLogicPoint nLogicPos( CLogicInt(0), CLogicInt(0) );
			View->m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nLayoutPos, &nLogicPos);
			int ret = nLogicPos.GetX() + 1;
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_LOGICTOLAYOUTLINENUM:
	case F_LINEINDEXTOCOLUMN:
		// ���W�b�N�����C�A�E�g�s/��
		{
			if( ArgSize < 2 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLogicInt nLineNum = CLogicInt(varCopy.Data.lVal - 1);
			if( !VariantToI4(varCopy, Arguments[1]) ){
				return false;
			}
			CLogicInt nLineIdx = CLogicInt(varCopy.Data.lVal - 1);
			if( nLineNum < 0 ){
				return false;
			}

			CLogicPoint nLogicPos(nLineIdx, nLineNum);
			CLayoutPoint nLayoutPos(CLayoutInt(0),CLayoutInt(0));
			View->m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nLogicPos, &nLayoutPos);
			int ret = ((LOWORD(ID) == F_LOGICTOLAYOUTLINENUM) ? (Int)nLayoutPos.GetY2() : (Int)nLayoutPos.GetX2()) + 1;
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_GETCOOKIE:
		{
			Variant varCopy2;
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_GETCOOKIEDEFAULT:
		{
			Variant varCopy2, varCopy3;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy3.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookieDefault(varCopy.Data.bstrVal, varCopy2.Data.bstrVal,
					varCopy3.Data.bstrVal, SysStringLen(varCopy3.Data.bstrVal) );
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_SETCOOKIE:
		{
			Variant varCopy2, varCopy3;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy3.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_BSTR) != S_OK) return false;
				int ret = View->GetDocument()->m_cCookie.SetCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal,
					varCopy3.Data.bstrVal, SysStringLen(varCopy3.Data.bstrVal) );
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_DELETECOOKIE:
		{
			Variant varCopy2;
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				int ret = View->GetDocument()->m_cCookie.DeleteCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_GETCOOKIENAMES:
		{
			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookieNames(varCopy.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_SETDRAWSWITCH:
		{
			if( 1 <= ArgSize ){
				if( !VariantToI4(varCopy, Arguments[0]) ) return false;
				int ret = (View->m_pcEditWnd->SetDrawSwitchOfAllViews(varCopy.Data.iVal != 0) ? 1: 0);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_GETDRAWSWITCH:
		{
			int ret = (View->GetDrawSwitch() ? 1: 0);
			Wrap( &Result )->Receive( ret );
			return true;
		}
	case F_ISSHOWNSTATUS:
		{
			int ret = (NULL != View->m_pcEditWnd->m_cStatusBar.GetStatusHwnd() ? 1: 0);
			Wrap( &Result )->Receive( ret );
			return true;
		}
	case F_GETSTRWIDTH:
		{
			Variant varCopy2;
			if( 1 <= ArgSize ){
				if( !VariantToBStr(varCopy, Arguments[0]) ){ return false; }
				if( 2 <= ArgSize ){
					if( !VariantToI4(varCopy2, Arguments[1]) ){ return false; }
				}else{
					varCopy2.Data.lVal = 1;
				}
				const wchar_t* pLine = varCopy.Data.bstrVal;
				int nLen = ::SysStringLen(varCopy.Data.bstrVal);
				if( 2 <= nLen ){
					if( pLine[nLen-2] == WCODE::CR && pLine[nLen-1] == WCODE::LF ){
						nLen--;
					}
				}
				const int nTabWidth = (Int)View->GetDocument()->m_cLayoutMgr.GetTabSpaceKetas();
				int nPosX = varCopy2.Data.lVal - 1;
				for( int i =0; i < nLen; ){
					if( pLine[i] == WCODE::TAB ){
						nPosX += nTabWidth - (nPosX % nTabWidth);
					}else{
						nPosX += (Int)CNativeW::GetKetaOfChar(pLine, nLen, i);
					}
					i += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(pLine, nLen, i));
				}
				nPosX -=  varCopy2.Data.lVal - 1;
				Wrap( &Result )->Receive( nPosX );
				return true;
			}
			return false;
		}
	case F_GETSTRLAYOUTLENGTH:
		{
			Variant varCopy2;
			if( 1 <= ArgSize ){
				if( !VariantToBStr(varCopy, Arguments[0]) ){ return false; }
				if( 2 <= ArgSize ){
					if( !VariantToI4(varCopy2, Arguments[1]) ){ return false; }
				}else{
					varCopy2.Data.lVal = 1;
				}
				CDocLine tmpDocLine;
				tmpDocLine.SetDocLineString(varCopy.Data.bstrVal, ::SysStringLen(varCopy.Data.bstrVal));
				const int tmpLenWithEol1 = tmpDocLine.GetLengthWithoutEOL() + (0 < tmpDocLine.GetEol().GetLen() ? 1: 0);
				const CLayoutXInt offset(varCopy2.Data.lVal - 1);
				const CLayout tmpLayout(
					&tmpDocLine,
					CLogicPoint(0,0),
					CLogicXInt(tmpLenWithEol1),
					COLORIDX_TEXT,
					offset,
					NULL
				);
				CLayoutXInt width = View->LineIndexToColumn(&tmpLayout, tmpDocLine.GetLengthWithEOL()) - offset;
				Wrap( &Result )->Receive( (Int)width );
				return true;
			}
			return false;
		}
	case F_GETDEFAULTCHARLENGTH:
		{
			CLayoutXInt width = View->GetTextMetrics().GetLayoutXDefault();
			Wrap( &Result )->Receive( (Int)width );
			return true;
		}
	case F_ISINCLUDECLIPBOARDFORMAT:
		{
			if( 1 <= ArgSize ){
				if( !VariantToBStr(varCopy, Arguments[0]) ) return false;
				CClipboard cClipboard(View->GetHwnd());
				bool bret = cClipboard.IsIncludeClipboradFormat(varCopy.Data.bstrVal);
				Wrap( &Result )->Receive( bret ? 1 : 0 );
				return true;
			}
			return false;
		}
	case F_GETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3;
			if( 2 <= ArgSize ){
				if( !VariantToBStr(varCopy, Arguments[0]) ) return false;
				if( !VariantToI4(varCopy2, Arguments[1]) ) return false;
				if( 3 <= ArgSize ){
					if( !VariantToI4(varCopy3, Arguments[2]) ) return false;
				}else{
					varCopy3.Data.lVal = -1;
				}
				CClipboard cClipboard(View->GetHwnd());
				CNativeW mem;
				CEol cEol = View->m_pcEditDoc->m_cDocEditor.GetNewLineCode();
				cClipboard.GetClipboradByFormat(mem, varCopy.Data.bstrVal, varCopy2.Data.lVal, varCopy3.Data.lVal, cEol);
				SysString ret = SysString(mem.GetStringPtr(), mem.GetStringLength());
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_SETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3, varCopy4;
			if( 3 <= ArgSize ){
				if( !VariantToBStr(varCopy, Arguments[0]) ) return false;
				if( !VariantToBStr(varCopy2, Arguments[1]) ) return false;
				if( !VariantToI4(varCopy3, Arguments[2]) ) return false;
				if( 3 <= ArgSize ){
					if( !VariantToI4(varCopy4, Arguments[3]) ) return false;
				}else{
					varCopy4.Data.lVal = -1;
				}
				CClipboard cClipboard(View->GetHwnd());
				CStringRef cstr(varCopy.Data.bstrVal, ::SysStringLen(varCopy.Data.bstrVal));
				bool bret = cClipboard.SetClipboradByFormat(cstr, varCopy2.Data.bstrVal, varCopy3.Data.lVal, varCopy4.Data.lVal);
				Wrap( &Result )->Receive( bret ? 1 : 0 );
				return true;
			}
			return false;
		}
	default:
		return false;
	}
}


