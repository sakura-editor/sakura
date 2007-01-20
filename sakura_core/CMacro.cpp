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
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "funccode.h"
#include "CMacro.h"
#include "CEditApp.h"
#include "CEditView.h" //2002/2/10 aroka
#include "CSMacroMgr.h" //2002/2/10 aroka
#include "etc_uty.h" //2002/2/10 aroka
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "debug.h"
#include "OleTypes.h" //2003-02-21 �S

CMacro::CMacro( int nFuncID )
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
		delete del_p->m_pData;
		delete del_p;
	}
	return;
}

/*	�����̌^�U�蕪��
	�@�\ID�ɂ���āA���҂���^�͈قȂ�܂��B
	�����ŁA�����̌^���@�\ID�ɂ���ĐU�蕪���āAAddParam���܂��傤�B
	���Ƃ��΁AF_INSTEXT��1�߁A2�߂̈����͕�����A3�߂̈�����int�������肷��̂��A�����ł��܂��U�蕪�����邱�Ƃ����҂��Ă��܂��B

	lParam�́AHandleCommand��param�ɒl��n���Ă���R�}���h�̏ꍇ�ɂ̂ݎg���܂��B
*/
void CMacro::AddLParam( LPARAM lParam, CEditView* pcEditView )
{
	switch( m_nFuncID ){
	/*	������p�����[�^��ǉ� */
	case F_INSTEXT:
	case F_FILEOPEN:
	case F_EXECCOMMAND:
		{
			AddParam( (const char *)lParam );	//	lParam��ǉ��B
			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_bGetStdout ? 0x01 : 0x00;
			AddParam( lFlag );
		}
		break;

	case F_JUMP:	//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		{
			AddParam( pcEditView->m_pcEditDoc->m_cDlgJump.m_nLineNum );
			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_bLineNumIsCRLF		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddParam( lFlag );
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bWordOnly			? 0x01 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bLoHiCase			? 0x02 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bRegularExp		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bNOTIFYNOTFOUND	? 0x08 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bAutoCloseDlgFind	? 0x10 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bSearchAll		? 0x20 : 0x00;
			AddParam( lFlag );
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParam��ǉ��B
			AddParam( pcEditView->m_pShareData->m_szREPLACEKEYArr[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bWordOnly			? 0x01 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bLoHiCase			? 0x02 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bRegularExp		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bNOTIFYNOTFOUND	? 0x08 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bAutoCloseDlgFind	? 0x10 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bSearchAll		? 0x20 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgReplace.m_nPaste		? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= pcEditView->m_pShareData->m_Common.m_bSelectedArea		? 0x80 : 0x00;	//	�u�����鎞�͑I�ׂȂ�
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget << 8;	//	8bit�V�t�g�i0x100�Ŋ|���Z�j
			lFlag |= pcEditView->m_pShareData->m_Common.m_bConsecutiveAll	? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddParam( lFlag );
		}
		break;
	case F_GREP:
		{
			AddParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParam��ǉ��B
			AddParam( pcEditView->m_pShareData->m_szGREPFILEArr[0] );	//	lParam��ǉ��B
			AddParam( pcEditView->m_pShareData->m_szGREPFOLDERArr[0] );	//	lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bGrepSubFolder				? 0x01 : 0x00;
			//			���̕ҏW���̃e�L�X�g���猟������(0x02.������)
			lFlag |= pcEditView->m_pShareData->m_Common.m_bLoHiCase						? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_bRegularExp					? 0x08 : 0x00;
			lFlag |= (pcEditView->m_pShareData->m_Common.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca ���ʌ݊����̂��߂̏���
			lFlag |= pcEditView->m_pShareData->m_Common.m_bGrepOutputLine				? 0x20 : 0x00;
			lFlag |= (pcEditView->m_pShareData->m_Common.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	//	CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= pcEditView->m_pShareData->m_Common.m_nGrepCharSet << 8;
			AddParam( lFlag );
		}
		break;
	/*	���l�p�����[�^��ǉ� */
	case F_CHAR:
		AddParam( lParam );
		break;

	/*	�W�����p�����[�^��ǉ� */
	default:
		AddParam( lParam );
		break;
	}
}

/*	�����ɕ������ǉ��B
*/
void CMacro::AddParam( const char* szParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	�K�v�ȗ̈���m�ۂ��ăR�s�[�B
	int nLen = lstrlen( szParam );
	param->m_pData = new char[nLen + 1];
	memcpy((char*)param->m_pData, szParam, nLen );
	param->m_pData[nLen] = '\0';

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
void CMacro::AddParam( const int nParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	�K�v�ȗ̈���m�ۂ��ăR�s�[�B
	param->m_pData = new char[16];	//	���l�i�[�i�ő�16���j�p
	itoa(nParam, param->m_pData, 10);

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

/*	�R�}���h�����s����ipcEditView->HandleCommand�𔭍s����j
	m_nFuncID�ɂ���āA�����̌^�𐳊m�ɓn���Ă����܂��傤�B
	
	paramArr�͉����̃|�C���^�i�A�h���X�j��LONG�ł���킵���l�ɂȂ�܂��B
	������char*�̂Ƃ��́AparamArr[i]�����̂܂�HandleCommand�ɓn���Ă��܂��܂���B
	������int�̂Ƃ��́A*((int*)paramArr[i])�Ƃ��ēn���܂��傤�B
	
	���Ƃ��΁AF_INSTEXT��1�߁A2�߂̈����͕�����A3�߂̈�����int�A4�߂̈����������B�������肷��ꍇ�́A���̂悤�ɂ��܂��傤�B
	pcEditView->HandleCommand( m_nFuncID, TRUE, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
*/
void CMacro::Exec( CEditView* pcEditView )
{
	const char* paramArr[4] = {NULL, NULL, NULL, NULL};	//	4�Ɍ���B
	
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
void CMacro::Save( HINSTANCE hInstance, HFILE hFile )
{
	char	szFuncName[1024];
	char	szFuncNameJapanese[500];
	int		nTextLen;
	const char*	pText;
//	int		i;
	char		szLine[1024];
//	const char*	szFuncName;
	CMemory		cmemWork;

	/* 2002.2.2 YAZAKI CSMacroMgr�ɗ��� */
	if (CSMacroMgr::GetFuncInfoByID( hInstance, m_nFuncID, szFuncName, szFuncNameJapanese)){
#if 0
	for( i = 0; i < m_nMacroFuncInfoArrNum; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == m_nFuncID ){
			szFuncName = m_MacroFuncInfoArr[i].m_pszFuncName;
			::LoadString( hInstance, m_nFuncID, szFuncNameJapanese, 255 );
#endif
		switch ( m_nFuncID ){
		case F_INSTEXT:
		case F_FILEOPEN:
			//	�����ЂƂ������ۑ�
			pText = m_pParamTop->m_pData;
			nTextLen = strlen(pText);
			cmemWork.SetData( pText, nTextLen );
			cmemWork.Replace_j( "\\", "\\\\" );
			cmemWork.Replace_j( "\'", "\\\'" );
			wsprintf( szLine, "S_%s(\'%s\');\t// %s\r\n", szFuncName, cmemWork.GetPtr(), szFuncNameJapanese );
			_lwrite( hFile, szLine, strlen( szLine ) );
			break;
		case F_JUMP:		//	�w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
			wsprintf( szLine, "S_%s(%d, %d);\t// %s\r\n", szFuncName, (m_pParamTop->m_pData ? atoi(m_pParamTop->m_pData) : 1), m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0, szFuncNameJapanese );
			_lwrite( hFile, szLine, strlen( szLine ) );
			break;
		case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		case F_SEARCH_NEXT:
		case F_SEARCH_PREV:
			pText = m_pParamTop->m_pData;
			nTextLen = strlen(pText);
			cmemWork.SetData( pText, nTextLen );
			cmemWork.Replace_j( "\\", "\\\\" );
			cmemWork.Replace_j( "\'", "\\\'" );
			wsprintf( szLine, "S_%s(\'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetPtr(), m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0, szFuncNameJapanese );
			_lwrite( hFile, szLine, strlen( szLine ) );
			break;
		case F_EXECCOMMAND:
			//	�����ЂƂ������ۑ�
			pText = m_pParamTop->m_pData;
			nTextLen = strlen(pText);
			cmemWork.SetData( pText, nTextLen );
			cmemWork.Replace_j( "\\", "\\\\" );
			cmemWork.Replace_j( "\'", "\\\'" );
			wsprintf( szLine, "S_%s(\'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetPtr(), m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0, szFuncNameJapanese );
			_lwrite( hFile, szLine, strlen( szLine ) );
			break;
		case F_REPLACE:
		case F_REPLACE_ALL:
			pText = m_pParamTop->m_pData;
			nTextLen = strlen(pText);
			cmemWork.SetData( pText, nTextLen );
			cmemWork.Replace_j( "\\", "\\\\" );
			cmemWork.Replace_j( "\'", "\\\'" );
			{
				CMemory cmemWork2(m_pParamTop->m_pNext->m_pData, strlen(m_pParamTop->m_pNext->m_pData));
				cmemWork2.Replace_j( "\\", "\\\\" );
				cmemWork2.Replace_j( "\'", "\\\'" );
				wsprintf( szLine, "S_%s(\'%s\', \'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetPtr(), cmemWork2.GetPtr(), m_pParamTop->m_pNext->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pNext->m_pData) : 0, szFuncNameJapanese );
				_lwrite( hFile, szLine, strlen( szLine ) );
			}
			break;
		case F_GREP:
			pText = m_pParamTop->m_pData;
			nTextLen = strlen(pText);
			cmemWork.SetData( pText, nTextLen );
			cmemWork.Replace_j( "\\", "\\\\" );
			cmemWork.Replace_j( "\'", "\\\'" );
			{
				CMemory cmemWork2(m_pParamTop->m_pNext->m_pData, strlen(m_pParamTop->m_pNext->m_pData));
				cmemWork2.Replace_j( "\\", "\\\\" );
				cmemWork2.Replace_j( "\'", "\\\'" );

				CMemory cmemWork3(m_pParamTop->m_pNext->m_pNext->m_pData, strlen(m_pParamTop->m_pNext->m_pNext->m_pData));
				cmemWork3.Replace_j( "\\", "\\\\" );
				cmemWork3.Replace_j( "\'", "\\\'" );
				wsprintf( szLine, "S_%s(\'%s\', \'%s\', \'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetPtr(), cmemWork2.GetPtr(), cmemWork3.GetPtr(), m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData) : 0, szFuncNameJapanese );
				_lwrite( hFile, szLine, strlen( szLine ) );
			}
			break;
		default:
			if( 0 == m_pParamTop ){
				wsprintf( szLine, "S_%s();\t// %s\r\n", szFuncName, szFuncNameJapanese );
			}else{
				wsprintf( szLine, "S_%s(%d);\t// %s\r\n", szFuncName, m_pParamTop->m_pData ? atoi(m_pParamTop->m_pData) : 0, szFuncNameJapanese );
			}
			_lwrite( hFile, szLine, strlen( szLine ) );
			break;
		}
		return;
#if 0
		}
	}
#endif
	}
	wsprintf( szLine, "CMacro::GetFuncInfoByID()�ɁA�o�O������̂ŃG���[���o�܂���������������������\r\n" );
	_lwrite( hFile, szLine, strlen( szLine ) );
}

/*!	Macro�R�}���h��CEditView::HandleCommand�Ɉ����n���B

	�������Ȃ��}�N���������C�}�N����HandleCommand�ł̑Ή��������Œ�`����K�v������D

	@param pcEditView	[in]	����Ώ�EditView
	@param Index	[in] �@�\ ID
	@param Argument [in] ����
	@param ArgSize	[in] �����̐�
	
*/
void CMacro::HandleCommand( CEditView* pcEditView, const int Index,	const char* Argument[], const int ArgSize )
{
	const char EXEC_ERROR_TITLE[] = "Macro���s�G���[";
	switch (Index) 
	{
	case F_CHAR:		//	�������́B���l�͕����R�[�h
	case F_IME_CHAR:	//	���{�����
	case F_CHGMOD_EOL:	//	���͉��s�R�[�h�w��BenumEOLType�̐��l���w��B2003.06.23 Moca
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�}�����ׂ������R�[�h���w�肳��Ă��܂���D" ));
			break;
		}
		/* NO BREAK */
	case F_GOLINETOP:	//	�s���Ɉړ��B���l�́A0x0�i�f�t�H���g�j�A0x1�i�󔒂𖳎����Đ擪�Ɉړ��j�A0x2�i����`�j�A0x4�i�I�����Ĉړ��j�A0x8�i���s�P�ʂŐ擪�Ɉړ��F�������j
		//	��ڂ̈��������l�B
		pcEditView->HandleCommand( Index, FALSE, (Argument[0] != NULL ? atoi(Argument[0]) : 0 ), 0, 0, 0 );
		break;
	case F_INSTEXT:		//	�e�L�X�g�}��
	case F_ADDTAIL:		//	���̑���̓L�[�{�[�h����ł͑��݂��Ȃ��̂ŕۑ����邱�Ƃ��ł��Ȃ��H
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta �ǉ� �e�L�X�g����1�����}�N���͂����ɓ������Ă������D
		//	��ڂ̈�����������B
		//	������2�ڂ̈����͕������B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"����(������)���w�肳��Ă��܂���D" ));
			break;
		}
		{
			int len = strlen(Argument[0]);
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], len, 0, 0 );	//	�W��
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
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�W�����v��s�ԍ����w�肳��Ă��܂���D" ));
			break;
		}
		{
			pcEditView->m_pcEditDoc->m_cDlgJump.m_nLineNum = atoi(Argument[0]);	//�W�����v��
			LPARAM lFlag = Argument[1] != NULL ? atoi(Argument[1]) : 1; // �f�t�H���g1
			pcEditView->m_pShareData->m_bLineNumIsCRLF = lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pcEditDoc->m_cDlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	�W��
		}
		break;
	/*	��ڂ̈����͕�����A��ڂ̈����͐��l	*/
	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�}�[�N�s�̃p�^�[�����w�肳��Ă��܂���D" ));
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
			if( 0 < lstrlen( Argument[0] ) ){
				/* ���K�\�� */
				if( pcEditView->m_pShareData->m_Common.m_bRegularExp && !CheckRegexpSyntax( Argument[0], NULL, true ) ){
					break;
				}

				/* ���������� */
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)Argument[0] );
			}
			//	�ݒ�l�o�b�N�A�b�v
			//	�}�N���p�����[�^���ݒ�l�ϊ�
			LPARAM lFlag = Argument[1] != NULL ? atoi(Argument[1]) : 0;
			pcEditView->m_pShareData->m_Common.m_bWordOnly			= lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bLoHiCase			= lFlag & 0x02 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bRegularExp		= lFlag & 0x04 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			//	�R�}���h���s
		//	pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0);
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
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"��r����t�@�C�����w�肳��Ă��܂���D" ));
			break;
		}
		pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
		break;
	case F_EXECCOMMAND:
		//	Argument[0]�����s�B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		���̐��l�̘a�B
		//		0x01	�W���o�͂𓾂�
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�O���R�}���h���w�肳��Ă��܂���D" ));
			break;
		}
		{
			LPARAM lFlag = Argument[1] != NULL ? atoi(Argument[1]) : 0;
			pcEditView->m_pShareData->m_bGetStdout = lFlag & 0x01 ? 1 : 0;
		//	pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)atoi(Argument[1]), 0, 0);
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_TRACEOUT:		// 2006.05.01 �}�N���p�A�E�g�v�b�g�E�C���h�E�ɏo��
		//	Argument[0]���o�́B�I�v�V������Argument[1]�ɁB
		//	Argument[1]:
		//		���̐��l�̘a�B
		//		0x01	ExpandParameter�ɂ�镶����W�J���s��
		//		0x02	�e�L�X�g�����ɉ��s�R�[�h��t�����Ȃ�
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�o�̓e�L�X�g���w�肳��Ă��܂���D" ));
			break;
		}
		{
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)(Argument[1] != NULL ? atoi(Argument[1]) : 0 ), 0, 0);
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
				_T(	"�u�����p�^�[�����w�肳��Ă��܂���D" ));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�u����p�^�[�����w�肳��Ă��܂���D" ));
			break;
		}
		{
//			if( 0 < lstrlen( Argument[0] ) ){
				/* ���K�\�� */
				if( pcEditView->m_pShareData->m_Common.m_bRegularExp && !CheckRegexpSyntax( Argument[0], NULL, true ) ){
					break;
				}

				/* ���������� */
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)Argument[0] );
//			}
//			if( 0 < lstrlen( Argument[1] ) ){
				/* ���������� */
				CShareData::getInstance()->AddToReplaceKeyArr( (const char*)Argument[1] );
//			}
			LPARAM lFlag = Argument[2] != NULL ? atoi(Argument[2]) : 0;
			pcEditView->m_pShareData->m_Common.m_bWordOnly			= lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bLoHiCase			= lFlag & 0x02 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bRegularExp		= lFlag & 0x04 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			pcEditView->m_pcEditDoc->m_cDlgReplace.m_nPaste			= lFlag & 0x40 ? 1 : 0;	//	CShareData�ɓ���Ȃ��Ă����́H
//			pcEditView->m_pShareData->m_Common.m_bSelectedArea		= 0;	//	lFlag & 0x80 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_bConsecutiveAll	= lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (Index == F_REPLACE) {
				//	�u�����鎞�͑I�ׂȂ�
				pcEditView->m_pShareData->m_Common.m_bSelectedArea	= 0;
			}
			else if (Index == F_REPLACE_ALL) {
				//	�S�u���̎��͑I�ׂ�H
				pcEditView->m_pShareData->m_Common.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			pcEditView->m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	//	8bit�V�t�g�i0x100�Ŋ���Z�j	// 2007.01.16 ryoji ���� 2bit�������o��
			//	�R�}���h���s
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0);
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
				_T(	"GREP�p�^�[�����w�肳��Ă��܂���D" ));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�t�@�C����ʂ��w�肳��Ă��܂���D" ));
			break;
		}
		if( Argument[2] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"������t�H���_���w�肳��Ă��܂���D" ));
			break;
		}
		{
			//	��ɊO���E�B���h�E�ɁB
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
			char	pCmdLine[1024];
			char	pOpt[64];
//			int		nDataLen;
			int		nCharSet;

			CMemory cmWork1;	cmWork1.SetDataSz( Argument[0] );	cmWork1.Replace_j( "\"", "\"\"" );	//	����������
			CMemory cmWork2;	cmWork2.SetDataSz( Argument[1] );	cmWork2.Replace_j( "\"", "\"\"" );	//	�t�@�C����
			CMemory cmWork3;	cmWork3.SetDataSz( Argument[2] );	cmWork3.Replace_j( "\"", "\"\"" );	//	�t�H���_��

			LPARAM lFlag = Argument[3] != NULL ? atoi(Argument[3]) : 5;

			// 2002/09/21 Moca �����R�[�h�Z�b�g
			{
				nCharSet = CODE_SJIS;
				if( lFlag & 0x10 ){	// �����R�[�h��������(���ʌ݊��p)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // ������ 7-15 �r�b�g��(0�J�n)���g��
				if( ( 0 < nCode && nCode < CODE_CODEMAX ) || CODE_AUTODETECT == nCode ){
					nCharSet = nCode;
				}
			}
			/*
			|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			*/
			wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d",
				cmWork1.GetPtr(),
				cmWork2.GetPtr(),
				cmWork3.GetPtr(),
				nCharSet
			);

			pOpt[0] = '\0';
			if( lFlag & 0x01 ){	/* �T�u�t�H���_������������� */
				strcat( pOpt, "S" );
			}
		//	if( lFlag & 0x02 ){	/* ���̕ҏW���̃e�L�X�g���猟������ */
		//
		//	}
			if( lFlag & 0x04 ){	/* �p�啶���Ɖp����������ʂ��� */
				strcat( pOpt, "L" );
			}
			if( lFlag & 0x08 ){	/* ���K�\�� */
				strcat( pOpt, "R" );
			}
//			2002/09/21 Moca -GCODE �ɓ���
//			if( lFlag & 0x10 ){	/* �����R�[�h�������� */
//				strcat( pOpt, "K" );
//			}
			if( lFlag & 0x20 ){	/* �s���o�͂��邩�Y�����������o�͂��邩 */
				strcat( pOpt, "P" );
			}
			if( lFlag & 0x40 ){	/* Grep: �o�͌`�� */
				strcat( pOpt, "2" );
			}
			else {
				strcat( pOpt, "1" );
			}
			if( 0 < lstrlen( pOpt ) ){
				strcat( pCmdLine, " -GOPT=" );
				strcat( pCmdLine, pOpt );
			}
			/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
			CEditApp::OpenNewEditor( pcEditView->m_hInstance, pcEditView->m_pShareData->m_hwndTray, pCmdLine, 0, FALSE );
			/*======= Grep�̎��s =============*/
			/* Grep���ʃE�B���h�E�̕\�� */
		}
		break;
	case F_FILEOPEN:
		//	Argument[0]���J���B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�ǂݍ��݃t�@�C�������w�肳��Ă��܂���D" ));
			break;
		}
		{
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_FILESAVEAS:
		//	Argument[0]��ʖ��ŕۑ��B
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"�ۑ��t�@�C�������w�肳��Ă��܂���D" ));
			break;
		}
		{
			/* �f�t�H���g�l */
			//	Sep. 11, 2004 genta �����l���u�ύX���Ȃ��v��
			//	0����SJIS�w��ƂȂ��Ă��܂�����
			int nCharCode = CODE_AUTODETECT;
			int nSaveLineCode = 0;
			
			if (Argument[1] != NULL){
				nCharCode = atoi( Argument[1] );
			}
			if (Argument[2] != NULL){
				nSaveLineCode = atoi( Argument[2] );
			}
			// �����R�[�h�Z�b�g
			//	Sep. 11, 2004 genta �����R�[�h�ݒ�͈̔̓`�F�b�N
			if(	0 <= nCharCode && nCharCode < CODE_CODEMAX &&
				nCharCode != pcEditView->m_pcEditDoc->m_nCharCode ){
				pcEditView->m_pcEditDoc->m_nCharCode = nCharCode;
				//	From Here Jul. 26, 2003 ryoji BOM��Ԃ�������
				switch( pcEditView->m_pcEditDoc->m_nCharCode ){
				case CODE_UNICODE:
				case CODE_UNICODEBE:
					pcEditView->m_pcEditDoc->m_bBomExist = TRUE;
					break;
				case CODE_UTF8:
				default:
					pcEditView->m_pcEditDoc->m_bBomExist = FALSE;
					break;
				}
				//	To Here Jul. 26, 2003 ryoji BOM��Ԃ�������
			}
			// ���s�R�[�h
			switch (nSaveLineCode){
			case 0:
				pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_NONE;
				break;
			case 1:
				pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_CRLF;
				break;
			case 2:
				pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_LF;
				break;
			case 3:
				pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_CR;
				break;
			default:
				pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_NONE;
				break;
			}
			
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
		pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
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
				noconfirm = ( atoi( Argument[0] ) != 0 );
			}
			pcEditView->HandleCommand( Index, FALSE, noconfirm, 0, 0, 0 );
		}
		break;
	//	To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if( Argument[0] != NULL ){
				lparam1 = atoi( Argument[0] );
				pcEditView->HandleCommand( Index, FALSE, lparam1, 0, 0, 0 );
			}
		}
		break;	//	Jan. 29, 2005 genta �����Ă���
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			//������NULL�ł�OK
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	default:
		//	�����Ȃ��B
		pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	�W��
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
			char const * FileName = View->m_pcEditDoc->GetFilePath();
			SysString S(FileName, lstrlen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if(View->IsTextSelected())
			{
				CMemory cMem;
				if(!View->GetSelectedData(cMem, FALSE, NULL, FALSE, FALSE)) return false;
				SysString S(cMem.GetPtr(), cMem.GetLength());
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
			char *Source;
			int SourceLength;
			Wrap(&varCopy.Data.bstrVal)->Get(&Source, &SourceLength);
			char Buffer[2048];
			View->m_pcEditDoc->ExpandParameter(Source, Buffer, 2047);
			delete[] Source;
			SysString S(Buffer, lstrlen(Buffer));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETLINESTR:
		//	2003.06.01 Moca �}�N���ǉ�
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			if( -1 < varCopy.Data.lVal ){
				const char *Buffer;
				int nLength, nLine;
				if( 0 == varCopy.Data.lVal ){
					nLine = View->m_nCaretPosY_PHY;
				}else{
					nLine = varCopy.Data.lVal - 1;
				}
				Buffer = View->m_pcEditDoc->m_cDocLineMgr.GetLineStr( nLine, &nLength );
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
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			View->m_pcEditDoc->ChangeLayoutParam( false, 
				varCopy.Data.iVal, View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() );
		}
		return true;
	case F_ISTEXTSELECTED:
		//	2005.07.30 maru �}�N���ǉ�
		{
			if(View->IsTextSelected()) {
				if(View->m_bBeginBoxSelect) {
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
			Wrap( &Result )->Receive( View->m_nSelectLineFrom + 1 );
		}
		return true;
	case F_GETSELCOLMFROM:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_nSelectColmFrom + 1 );
		}
		return true;
	case F_GETSELLINETO:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_nSelectLineTo + 1 );
		}
		return true;
	case F_GETSELCOLMTO:
		//	2005.07.30 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_nSelectColmTo + 1);
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
			Wrap( &Result )->Receive(View->m_pcEditDoc->m_nCharCode);
		}
		return true;
	case F_GETLINECODE:
		//	2005.08.04 maru �}�N���ǉ�
		{
			switch( View->m_pcEditDoc->GetNewLineCode() ){
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
			Wrap( &Result )->Receive( View->m_pcEditDoc->IsEnableUndo() );
		}
		return true;
	case F_ISPOSSIBLEREDO:
		//	2005.08.04 maru �}�N���ǉ�
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->IsEnableRedo() );
		}
		return true;
	default:
		return false;
	}
}

/*[EOF]*/
