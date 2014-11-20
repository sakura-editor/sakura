/*!	@file
	@brief DIFF�����\��

	@author MIK
	@date	2002/05/25 ExecCmd ���Q�l��DIFF���s���ʂ���荞�ޏ����쐬
 	@date	2005/10/29	maru Diff�����\�������𕪗����A�_�C�A���O����ŁE�_�C�A���O�Ȃ��ł̗�������R�[��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, GAE, YAZAKI, hor
	Copyright (C) 2002, hor, MIK
	Copyright (C) 2003, MIK, ryoji, genta
	Copyright (C) 2004, genta
	Copyright (C) 2005, maru
	Copyright (C) 2007, ryoji, kobake
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
#include <stdio.h>
#include <stdlib.h>
#include "view/CEditView.h"
#include "_main/global.h"
#include "dlg/CDlgDiff.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "uiparts/CWaitCursor.h"
#include "_os/COsVersionInfo.h"
#include "util/module.h"
#include "util/file.h"
#include "window/CEditWnd.h"
#include "io/CTextStream.h"
#include "io/CFileLoad.h"
#include "CWriteManager.h"
#include "sakura_rc.h"

#define	SAKURA_DIFF_TEMP_PREFIX	_T("sakura_diff_")

class COutputAdapterDiff: public COutputAdapter
{
public:
	COutputAdapterDiff(CEditView* view, int nFlgFile12_){
		m_view = view;
		bLineHead = true;
		bDiffInfo = false;
		nDiffLen = 0;
		bFirst = true;
		nFlgFile12 = nFlgFile12_;
		szDiffData[0] = 0;
	}
	~COutputAdapterDiff(){};

	bool OutputW(const WCHAR* pBuf, int size = -1){ return true; };
	bool OutputA(const ACHAR* pBuf, int size = -1);
	bool IsEnableRunningDlg(){ return false; }
	bool IsActiveDebugWindow(){ return false; }

public:
	bool	bDiffInfo;	//DIFF���
	int		nDiffLen;		//DIFF���
	char	szDiffData[100];	//DIFF���
protected:
	CEditView* m_view;
	bool	bLineHead;	//�s����
	bool	bFirst;	//�擪���H	//@@@ 2003.05.31 MIK
	int		nFlgFile12;
};


/*!	�����\��
	@param	pszFile1	[in]	���t�@�C����
	@param	pszFile2	[in]	����t�@�C����
    @param  nFlgOpt     [in]    0b000000000
                                    ||||||+--- -i ignore-case         �啶�����������ꎋ
                                    |||||+---- -w ignore-all-space    �󔒖���
                                    ||||+----- -b ignore-space-change �󔒕ύX����
                                    |||+------ -B ignore-blank-lines  ��s����
                                    ||+------- -t expand-tabs         TAB-SPACE�ϊ�
                                    |+--------    (�ҏW���̃t�@�C�������t�@�C��)
                                    +---------    (DIFF�������Ȃ��Ƃ��Ƀ��b�Z�[�W�\��)
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	MIK
	@date	2002/05/25
	@date	2005/10/28	��Command_Diff����֐����̕ύX�B
						GetCommander().Command_Diff_Dialog�����łȂ��VCommand_Diff
						������Ă΂��֐��Bmaru
	@date	2013/06/21	ExecCmd�𗘗p����悤��
*/
void CEditView::ViewDiffInfo( 
	const TCHAR*	pszFile1,
	const TCHAR*	pszFile2,
	int				nFlgOpt,
	bool 			bUTF8
)
/*
	bool	bFlgCase,		//�啶�����������ꎋ
	bool	bFlgBlank,		//�󔒖���
	bool	bFlgWhite,		//�󔒕ύX����
	bool	bFlgBLine,		//��s����
	bool	bFlgTabSpc,		//TAB-SPACE�ϊ�
	bool	bFlgFile12,		//�ҏW���̃t�@�C�������t�@�C��
*/
{
	CWaitCursor	cWaitCursor( this->GetHwnd() );
	int		nFlgFile12 = 1;

	/* exe�̂���t�H���_ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	TCHAR	cmdline[1024];
	GetExedir( cmdline, _T("diff.exe") );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//	From Here Dec. 28, 2002 MIK
	//	diff.exe�̑��݃`�F�b�N
	if( !IsFileExists( cmdline, true ) )
	{
		WarningMessage( GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF2) );
		return;
	}
	cmdline[0] = _T('\0');

	//������DIFF��������������B
	if( CDiffManager::getInstance()->IsDiffUse() )
		GetCommander().Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	//�I�v�V�������쐬����
	TCHAR	szOption[16];	// "-cwbBt"
	_tcscpy( szOption, _T("-") );
	if( nFlgOpt & 0x0001 ) _tcscat( szOption, _T("i") );	//-i ignore-case         �啶�����������ꎋ
	if( nFlgOpt & 0x0002 ) _tcscat( szOption, _T("w") );	//-w ignore-all-space    �󔒖���
	if( nFlgOpt & 0x0004 ) _tcscat( szOption, _T("b") );	//-b ignore-space-change �󔒕ύX����
	if( nFlgOpt & 0x0008 ) _tcscat( szOption, _T("B") );	//-B ignore-blank-lines  ��s����
	if( nFlgOpt & 0x0010 ) _tcscat( szOption, _T("t") );	//-t expand-tabs         TAB-SPACE�ϊ�
	if( _tcscmp( szOption, _T("-") ) == 0 ) _tcscpy( szOption, _T("") );	//�I�v�V�����Ȃ�
	if( nFlgOpt & 0x0020 ) nFlgFile12 = 0;
	else                   nFlgFile12 = 1;

	//	To Here Dec. 28, 2002 MIK

	{
		//�R�}���h���C��������쐬(MAX:1024)
		auto_sprintf(
			cmdline,
			_T("\"%ts\\%ts\" %ts \"%ts\" \"%ts\""),
			szExeFolder,	//sakura.exe�p�X
			_T("diff.exe"),		//diff.exe
			szOption,		//diff�I�v�V����
			( nFlgFile12 ? pszFile2 : pszFile1 ),
			( nFlgFile12 ? pszFile1 : pszFile2 )
		);
	}

	{
		int nFlgOpt = 0;
		nFlgOpt |= 0x01;  // GetStdOut
		if( bUTF8 ){
			nFlgOpt |= 0x80;  // UTF-8 out (SJIS�ƈ����ASCII�Z�[�t�Ȃ̂�)
			nFlgOpt |= 0x100; // UTF-8 in
		}
		nFlgOpt |= 0x40;  // �g�����o�͖���
		COutputAdapterDiff oa(this, nFlgFile12);
		bool ret = ExecCmd( cmdline, nFlgOpt, NULL, &oa );

		if( ret ){
			if( oa.bDiffInfo == true && oa.nDiffLen > 0 )
			{
				oa.szDiffData[oa.nDiffLen] = '\0';
				AnalyzeDiffInfo( oa.szDiffData, nFlgFile12 );
			}
		}
	}

	//DIFF������������Ȃ������Ƃ��Ƀ��b�Z�[�W�\��
	if( nFlgOpt & 0x0040 )
	{
		if( !CDiffManager::getInstance()->IsDiffUse() )
		{
			InfoMessage( this->GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF5) );
		}
	}


	//���������r���[���X�V
	m_pcEditWnd->Views_Redraw();

	return;
}

bool COutputAdapterDiff::OutputA(const ACHAR* pBuf, int size)
{
	if( size == -1 ){
		size = auto_strlen(pBuf);
	}
	//@@@ 2003.05.31 MIK
	//	�擪��Binary files�Ȃ�o�C�i���t�@�C���̂��߈Ӗ��̂��鍷��������Ȃ�����
	if( bFirst )
	{
		bFirst = false;
		if( strncmp( pBuf, "Binary files ", strlen( "Binary files " ) ) == 0 )
		{
			WarningMessage( NULL, LS(STR_ERR_DLGEDITVWDIFF4) );
			return false;
		}
	}

	//�ǂݏo������������`�F�b�N����
	int j;
	for( j = 0; j < (int)size/*-1*/; j++ )
	{
		if( bLineHead )
		{
			if( pBuf[j] != '\n' && pBuf[j] != '\r' )
			{
				bLineHead = false;
			
				//DIFF���̎n�܂肩�H
				if( pBuf[j] >= '0' && pBuf[j] <= '9' )
				{
					bDiffInfo = true;
					nDiffLen = 0;
					szDiffData[nDiffLen++] = pBuf[j];
				}
				/*
				else if( pBuf[j] == '<' || pBuf[j] == '>' || pBuf[j] == '-' )
				{
					bDiffInfo = false;
					nDiffLen = 0;
				}
				*/
			}
		}
		else
		{
			//�s���ɒB�������H
			if( pBuf[j] == '\n' || pBuf[j] == '\r' )
			{
				//DIFF��񂪂���Ή�͂���
				if( bDiffInfo == true && nDiffLen > 0 )
				{
					szDiffData[nDiffLen] = '\0';
					m_view->AnalyzeDiffInfo( szDiffData, nFlgFile12 );
					nDiffLen = 0;
				}
				
				bDiffInfo = false;
				bLineHead = true;
			}
			else if( bDiffInfo == true )
			{
				//DIFF���ɒǉ�����
				szDiffData[nDiffLen++] = pBuf[j];
				if( nDiffLen >= 99 )
				{
					nDiffLen = 0;
					bDiffInfo = false;
				}
			}
		}
	}
	return true;
}

/*!	DIFF����������͂��}�[�N�o�^
	@param	pszDiffInfo	[in]	�V�t�@�C����
	@param	nFlgFile12	[in]	�ҏW���t�@�C����...
									0	�t�@�C��1(���t�@�C��)
									1	�t�@�C��2(�V�t�@�C��)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	const char*	pszDiffInfo,
	int			nFlgFile12
)
{
	/*
	 * 99a99		���t�@�C��99�s�̎��s�ɐV�t�@�C��99�s���ǉ����ꂽ�B
	 * 99a99,99		���t�@�C��99�s�̎��s�ɐV�t�@�C��99�`99�s���ǉ����ꂽ�B
	 * 99c99		���t�@�C��99�s���V�t�@�C��99�s�ɕύX���ꂽ�B
	 * 99,99c99,99	���t�@�C��99�`99�s���V�t�@�C��99�`99�s�ɕύX���ꂽ�B
	 * 99d99		���t�@�C��99�s���V�t�@�C��99�s�̎��s����폜���ꂽ�B
	 * 99,99d99		���t�@�C��99�`99�s���V�t�@�C��99�s�̎��s����폜���ꂽ�B
	 * s1,e1 mode s2,e2
	 * �擪�̏ꍇ0�̎��s�ƂȂ邱�Ƃ�����
	 */
	const char	*q;
	int		s1, e1, s2, e2;
	char	mode;

	//�O���t�@�C���̊J�n�s
	s1 = 0;
	for( q = pszDiffInfo; *q; q++ )
	{
		if( *q == ',' ) break;
		if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
		//�s�ԍ��𒊏o
		if( *q >= '0' && *q <= '9' ) s1 = s1 * 10 + (*q - '0');
		else return;
	}
	if( ! *q ) return;

	//�O���t�@�C���̏I���s
	if( *q != ',' )
	{
		//�J�n�E�I���s�ԍ��͓���
		e1 = s1;
	}
	else
	{
		e1 = 0;
		for( q++; *q; q++ )
		{
			if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
			//�s�ԍ��𒊏o
			if( *q >= '0' && *q <= '9' ) e1 = e1 * 10 + (*q - '0');
			else return;
		}
	}
	if( ! *q ) return;

	//DIFF���[�h���擾
	mode = *q;

	//�㔼�t�@�C���̊J�n�s
	s2 = 0;
	for( q++; *q; q++ )
	{
		if( *q == ',' ) break;
		//�s�ԍ��𒊏o
		if( *q >= '0' && *q <= '9' ) s2 = s2 * 10 + (*q - '0');
		else return;
	}

	//�㔼�t�@�C���̏I���s
	if( *q != ',' )
	{
		//�J�n�E�I���s�ԍ��͓���
		e2 = s2;
	}
	else
	{
		e2 = 0;
		for( q++; *q; q++ )
		{
			//�s�ԍ��𒊏o
			if( *q >= '0' && *q <= '9' ) e2 = e2 * 10 + (*q - '0');
			else return;
		}
	}

	//�s���ɒB���ĂȂ���΃G���[
	if( *q ) return;

	//���o����DIFF��񂩂�s�ԍ��ɍ����}�[�N��t����
	if( 0 == nFlgFile12 )	//�ҏW���t�@�C���͋��t�@�C��
	{
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s1    ), CLogicInt(e1    ) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
	}
	else	//�ҏW���t�@�C���͐V�t�@�C��
	{
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s2    ), CLogicInt(e2    ) );
	}

	return;
}

/*!	�ꎞ�t�@�C�����쐬����
	@author	MIK
	@date	2002/05/26
	@date	2005/10/29	�����ύXconst char* �� char*
						�ꎞ�t�@�C�����̎擾�����������ł����Ȃ��Bmaru
	@date	2007/08/??	kobake �@�B�I��UNICODE��
	@date	2008/01/26	kobake �o�͌`���C��
	@date	2013/06/21 �G���R�[�h��ASCII�n�ɂ���(SJIS�Œ����߂�)
*/
BOOL CEditView::MakeDiffTmpFile( TCHAR* filename, HWND hWnd, ECodeType code, bool bBom )
{
	//�ꎞ
	TCHAR* pszTmpName = _ttempnam( NULL, SAKURA_DIFF_TEMP_PREFIX );
	if( NULL == pszTmpName ){
		WarningMessage( NULL, LS(STR_DIFF_FAILED) );
		return FALSE;
	}

	_tcscpy( filename, pszTmpName );
	free( pszTmpName );

	//�������H
	if( NULL == hWnd )
	{
		EConvertResult eWriteResult = CWriteManager().WriteFile_From_CDocLineMgr(
			m_pcEditDoc->m_cDocLineMgr,
			SSaveInfo(
				filename,
				code,
				EOL_NONE,
				bBom
			)
		);
		return RESULT_FAILURE != eWriteResult;
	}

	CTextOutputStream out(filename, code, true, false);
	if(!out){
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
		return FALSE;
	}

	CLogicInt y = CLogicInt(0);

	for (;;) {
		// �s(���s�P��)�f�[�^�̗v�� 
		const wchar_t*	pLineData;
		CLogicInt		nLineLen;
		if( hWnd ){
			pLineData = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
			nLineLen = CLogicInt(::SendMessageAny( hWnd, MYWM_GETLINEDATA, y, 0 ));

			// �ꎞ�o�b�t�@�𒴂���ꍇ�̓G���[�I��
			if( nLineLen > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
				out.Close();
				_tunlink( filename );	//�֐��̎��s�Ɏ��s�����Ƃ��A�ꎞ�t�@�C���̍폜�͊֐����ōs���B2005.10.29
				WarningMessage( NULL, LS(STR_DIFF_FAILED_LONG) );
				return FALSE;
			}
		}
		else{
			pLineData = m_pcEditDoc->m_cDocLineMgr.GetLine(y)->GetDocLineStrWithEOL(&nLineLen);
		}

		if( 0 == nLineLen || NULL == pLineData ) break;

		try{
			if( bBom ){
				CNativeW cLine2(L"\ufeff");
				cLine2.AppendString(pLineData, nLineLen);
				out.WriteString(cLine2.GetStringPtr(), cLine2.GetStringLength());
				bBom = false;
			}else{
				out.WriteString(pLineData,nLineLen);
			}
		}
		catch(...){
			out.Close();
			_tunlink( filename );	//�֐��̎��s�Ɏ��s�����Ƃ��A�ꎞ�t�@�C���̍폜�͊֐����ōs���B2005.10.29
			WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
			return FALSE;
		}

		y++;
	}
	if( bBom ){
		out.WriteString(L"\ufeff", 1);
	}

	//fclose( fp );

	return TRUE;
}



/*!	�O���t�@�C�����w��ł̃t�@�C����\��
*/
BOOL CEditView::MakeDiffTmpFile2( TCHAR* tmpName, const TCHAR* orgName, ECodeType code, ECodeType saveCode )
{
	//�ꎞ
	TCHAR* pszTmpName = _ttempnam( NULL, SAKURA_DIFF_TEMP_PREFIX );
	if( NULL == pszTmpName ){
		WarningMessage( NULL, LS(STR_DIFF_FAILED) );
		return FALSE;
	}

	_tcscpy( tmpName, pszTmpName );
	free( pszTmpName );

	bool bBom = false;
	const STypeConfigMini* typeMini;
	CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( orgName ), &typeMini );
	CFileLoad	cfl( typeMini->m_encoding );
	CTextOutputStream out(tmpName, saveCode, true, false);
	if(!out){
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
		return FALSE;
	}
	try{
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		cfl.FileOpen( orgName, bBigFile, code, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &bBom );
		CNativeW cLine;
		CEol cEol;
		while( RESULT_FAILURE != cfl.ReadLine( &cLine, &cEol ) ) {
			const wchar_t*	pLineData;
			CLogicInt		nLineLen;
			pLineData= cLine.GetStringPtr(&nLineLen);
			if( 0 == nLineLen || NULL == pLineData ) break;
			if( bBom ){
				CNativeW cLine2(L"\ufeff");
				cLine2.AppendString(pLineData, nLineLen);
				out.WriteString(cLine2.GetStringPtr(), cLine2.GetStringLength());
				bBom = false;
			}else{
				out.WriteString(pLineData,nLineLen);
			}
		}
		if( bBom ){
			out.WriteString(L"\ufeff", 1);
		}
	}
	catch(...){
		out.Close();
		_tunlink( tmpName );	//�֐��̎��s�Ɏ��s�����Ƃ��A�ꎞ�t�@�C���̍폜�͊֐����ōs���B
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
		return FALSE;
	}

	return TRUE;
}

