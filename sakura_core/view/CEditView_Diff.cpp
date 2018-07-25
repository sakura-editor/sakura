/*!	@file
	@brief DIFF差分表示

	@author MIK
	@date	2002/05/25 ExecCmd を参考にDIFF実行結果を取り込む処理作成
 	@date	2005/10/29	maru Diff差分表示処理を分離し、ダイアログあり版・ダイアログなし版の両方からコール
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
#include "_main/CMutex.h"
#include "dlg/CDlgDiff.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "uiparts/CWaitCursor.h"
#include "_os/COsVersionInfo.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
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
	bool	bDiffInfo;	//DIFF情報か
	int		nDiffLen;		//DIFF情報長
	char	szDiffData[100];	//DIFF情報
protected:
	CEditView* m_view;
	bool	bLineHead;	//行頭か
	bool	bFirst;	//先頭か？	//@@@ 2003.05.31 MIK
	int		nFlgFile12;
};


/*!	差分表示
	@param	pszFile1	[in]	自ファイル名
	@param	pszFile2	[in]	相手ファイル名
    @param  nFlgOpt     [in]    0b000000000
                                    ||||||+--- -i ignore-case         大文字小文字同一視
                                    |||||+---- -w ignore-all-space    空白無視
                                    ||||+----- -b ignore-space-change 空白変更無視
                                    |||+------ -B ignore-blank-lines  空行無視
                                    ||+------- -t expand-tabs         TAB-SPACE変換
                                    |+--------    (編集中のファイルが旧ファイル)
                                    +---------    (DIFF差分がないときにメッセージ表示)
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
	@author	MIK
	@date	2002/05/25
	@date	2005/10/28	旧Command_Diffから関数名の変更。
						GetCommander().Command_Diff_Dialogだけでなく新Command_Diff
						からも呼ばれる関数。maru
	@date	2013/06/21	ExecCmdを利用するように
*/
void CEditView::ViewDiffInfo( 
	const TCHAR*	pszFile1,
	const TCHAR*	pszFile2,
	int				nFlgOpt,
	bool 			bUTF8
)
/*
	bool	bFlgCase,		//大文字小文字同一視
	bool	bFlgBlank,		//空白無視
	bool	bFlgWhite,		//空白変更無視
	bool	bFlgBLine,		//空行無視
	bool	bFlgTabSpc,		//TAB-SPACE変換
	bool	bFlgFile12,		//編集中のファイルが旧ファイル
*/
{
	CWaitCursor	cWaitCursor( this->GetHwnd() );
	int		nFlgFile12 = 1;

	/* exeのあるフォルダ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	TCHAR	cmdline[1024];
	GetExedir( cmdline, _T("diff.exe") );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//	From Here Dec. 28, 2002 MIK
	//	diff.exeの存在チェック
	if( !IsFileExists( cmdline, true ) )
	{
		WarningMessage( GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF2) );
		return;
	}
	cmdline[0] = _T('\0');

	//今あるDIFF差分を消去する。
	if( CDiffManager::getInstance()->IsDiffUse() )
		GetCommander().Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	//オプションを作成する
	TCHAR	szOption[16];	// "-cwbBt"
	_tcscpy( szOption, _T("-") );
	if( nFlgOpt & 0x0001 ) _tcscat( szOption, _T("i") );	//-i ignore-case         大文字小文字同一視
	if( nFlgOpt & 0x0002 ) _tcscat( szOption, _T("w") );	//-w ignore-all-space    空白無視
	if( nFlgOpt & 0x0004 ) _tcscat( szOption, _T("b") );	//-b ignore-space-change 空白変更無視
	if( nFlgOpt & 0x0008 ) _tcscat( szOption, _T("B") );	//-B ignore-blank-lines  空行無視
	if( nFlgOpt & 0x0010 ) _tcscat( szOption, _T("t") );	//-t expand-tabs         TAB-SPACE変換
	if( _tcscmp( szOption, _T("-") ) == 0 ) _tcscpy( szOption, _T("") );	//オプションなし
	if( nFlgOpt & 0x0020 ) nFlgFile12 = 0;
	else                   nFlgFile12 = 1;

	//	To Here Dec. 28, 2002 MIK

	{
		//コマンドライン文字列作成(MAX:1024)
		auto_sprintf(
			cmdline,
			_T("\"%ts\\%ts\" %ts \"%ts\" \"%ts\""),
			szExeFolder,	//sakura.exeパス
			_T("diff.exe"),		//diff.exe
			szOption,		//diffオプション
			( nFlgFile12 ? pszFile2 : pszFile1 ),
			( nFlgFile12 ? pszFile1 : pszFile2 )
		);
	}

	{
		int nFlgOpt = 0;
		nFlgOpt |= 0x01;  // GetStdOut
		if( bUTF8 ){
			nFlgOpt |= 0x80;  // UTF-8 out (SJISと違ってASCIIセーフなので)
			nFlgOpt |= 0x100; // UTF-8 in
		}
		nFlgOpt |= 0x40;  // 拡張情報出力無効
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

	//DIFF差分が見つからなかったときにメッセージ表示
	if( nFlgOpt & 0x0040 )
	{
		if( !CDiffManager::getInstance()->IsDiffUse() )
		{
			InfoMessage( this->GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF5) );
		}
	}


	//分割したビューも更新
	m_pcEditWnd->Views_Redraw();

	return;
}

bool COutputAdapterDiff::OutputA(const ACHAR* pBuf, int size)
{
	if( size == -1 ){
		size = auto_strlen(pBuf);
	}
	//@@@ 2003.05.31 MIK
	//	先頭がBinary filesならバイナリファイルのため意味のある差分が取られなかった
	if( bFirst )
	{
		bFirst = false;
		if( strncmp( pBuf, "Binary files ", strlen( "Binary files " ) ) == 0 )
		{
			WarningMessage( NULL, LS(STR_ERR_DLGEDITVWDIFF4) );
			return false;
		}
	}

	//読み出した文字列をチェックする
	int j;
	for( j = 0; j < (int)size/*-1*/; j++ )
	{
		if( bLineHead )
		{
			if( pBuf[j] != '\n' && pBuf[j] != '\r' )
			{
				bLineHead = false;
			
				//DIFF情報の始まりか？
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
			//行末に達したか？
			if( pBuf[j] == '\n' || pBuf[j] == '\r' )
			{
				//DIFF情報があれば解析する
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
				//DIFF情報に追加する
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

/*!	DIFF差分情報を解析しマーク登録
	@param	pszDiffInfo	[in]	新ファイル名
	@param	nFlgFile12	[in]	編集中ファイルは...
									0	ファイル1(旧ファイル)
									1	ファイル2(新ファイル)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	const char*	pszDiffInfo,
	int			nFlgFile12
)
{
	/*
	 * 99a99		旧ファイル99行の次行に新ファイル99行が追加された。
	 * 99a99,99		旧ファイル99行の次行に新ファイル99～99行が追加された。
	 * 99c99		旧ファイル99行が新ファイル99行に変更された。
	 * 99,99c99,99	旧ファイル99～99行が新ファイル99～99行に変更された。
	 * 99d99		旧ファイル99行が新ファイル99行の次行から削除された。
	 * 99,99d99		旧ファイル99～99行が新ファイル99行の次行から削除された。
	 * s1,e1 mode s2,e2
	 * 先頭の場合0の次行となることもある
	 */
	const char	*q;
	int		s1, e1, s2, e2;
	char	mode;

	//前半ファイルの開始行
	s1 = 0;
	for( q = pszDiffInfo; *q; q++ )
	{
		if( *q == ',' ) break;
		if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
		//行番号を抽出
		if( *q >= '0' && *q <= '9' ) s1 = s1 * 10 + (*q - '0');
		else return;
	}
	if( ! *q ) return;

	//前半ファイルの終了行
	if( *q != ',' )
	{
		//開始・終了行番号は同じ
		e1 = s1;
	}
	else
	{
		e1 = 0;
		for( q++; *q; q++ )
		{
			if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
			//行番号を抽出
			if( *q >= '0' && *q <= '9' ) e1 = e1 * 10 + (*q - '0');
			else return;
		}
	}
	if( ! *q ) return;

	//DIFFモードを取得
	mode = *q;

	//後半ファイルの開始行
	s2 = 0;
	for( q++; *q; q++ )
	{
		if( *q == ',' ) break;
		//行番号を抽出
		if( *q >= '0' && *q <= '9' ) s2 = s2 * 10 + (*q - '0');
		else return;
	}

	//後半ファイルの終了行
	if( *q != ',' )
	{
		//開始・終了行番号は同じ
		e2 = s2;
	}
	else
	{
		e2 = 0;
		for( q++; *q; q++ )
		{
			//行番号を抽出
			if( *q >= '0' && *q <= '9' ) e2 = e2 * 10 + (*q - '0');
			else return;
		}
	}

	//行末に達してなければエラー
	if( *q ) return;

	//抽出したDIFF情報から行番号に差分マークを付ける
	if( 0 == nFlgFile12 )	//編集中ファイルは旧ファイル
	{
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s1    ), CLogicInt(e1    ) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
	}
	else	//編集中ファイルは新ファイル
	{
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s2    ), CLogicInt(e2    ) );
	}

	return;
}

static bool MakeDiffTmpFile_core(CTextOutputStream& out, HWND hwnd, CEditView& view, bool bBom)
{
	CLogicInt y = CLogicInt(0);
	const wchar_t*	pLineData;
	if( !hwnd ){
		const CDocLineMgr& docMgr = view.m_pcEditDoc->m_cDocLineMgr;
		for(;;){
			CLogicInt		nLineLen;
			pLineData = docMgr.GetLine(y)->GetDocLineStrWithEOL(&nLineLen);
			// 正常終了
			if( 0 == nLineLen || NULL == pLineData ) break;
			if( bBom ){
				CNativeW cLine2(L"\ufeff");
				cLine2.AppendString(pLineData, nLineLen);
				out.WriteString(cLine2.GetStringPtr(), cLine2.GetStringLength());
				bBom = false;
			}else{
				out.WriteString(pLineData,nLineLen);
			}
			y++;
		}
	}else if( IsSakuraMainWindow(hwnd) ) {
		const int max_size = (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<const EDIT_CHAR>();
		pLineData = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<const EDIT_CHAR>();
		for(;;){
			int nLineOffset = 0;
			int nLineLen = 0; //初回用仮値
			do{
				// m_sWorkBuffer#m_Workの排他制御。外部コマンド出力/TraceOut/Diffが対象
				LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
				{
					nLineLen = ::SendMessageAny( hwnd, MYWM_GETLINEDATA, y, nLineOffset );
					if( nLineLen == 0 ){ return true; } // EOF => 正常終了
					if( nLineLen < 0 ){ return false; } // 何かエラー
					if( bBom ){
						CNativeW cLine2(L"\ufeff");
						cLine2.AppendString(pLineData, t_min(nLineLen, max_size));
						out.WriteString(cLine2.GetStringPtr(), cLine2.GetStringLength());
						bBom = false;
					}else{
						out.WriteString(pLineData, t_min(nLineLen, max_size));
					}
				}
				nLineOffset += max_size;
			}while(max_size < nLineLen);
			y++;
		}
	}else{
		return false;
	}
	if( bBom ){
		out.WriteString(L"\ufeff", 1);
	}
	return true;
}

/*!	一時ファイルを作成する
	@author	MIK
	@date	2002/05/26
	@date	2005/10/29	引数変更const char* → char*
						一時ファイル名の取得処理もここでおこなう。maru
	@date	2007/08/??	kobake 機械的にUNICODE化
	@date	2008/01/26	kobake 出力形式修正
	@date	2013/06/21 エンコードをASCII系にする(SJIS固定をやめる)
*/
BOOL CEditView::MakeDiffTmpFile( TCHAR* filename, HWND hWnd, ECodeType code, bool bBom )
{
	//一時
	TCHAR* pszTmpName = _ttempnam( NULL, SAKURA_DIFF_TEMP_PREFIX );
	if( NULL == pszTmpName ){
		WarningMessage( NULL, LS(STR_DIFF_FAILED) );
		return FALSE;
	}

	_tcscpy( filename, pszTmpName );
	free( pszTmpName );

	//自分か？
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

	bool bError = false;
	try{
		if( ! MakeDiffTmpFile_core(out, hWnd, *this, bBom) ){
			bError = true;
		}
	}
	catch(...){
		bError = true;
	}
	if( bError ){
		out.Close();
		_tunlink( filename );	//関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。2005.10.29
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
	}

	return TRUE;
}



/*!	外部ファイルを指定でのファイルを表示
*/
BOOL CEditView::MakeDiffTmpFile2( TCHAR* tmpName, const TCHAR* orgName, ECodeType code, ECodeType saveCode )
{
	//一時
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
		_tunlink( tmpName );	//関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
		return FALSE;
	}

	return TRUE;
}

