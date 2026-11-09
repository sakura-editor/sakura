/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ユーザーに贈る
//Jul. 08, 2001 JEPRO 追加
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("Perl") );
	wcscpy( pType->m_szTypeExts, LTEXT("cgi,pl,pm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("#"), -1 );					/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_PERL;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 12;								/* キーワードセット2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトON	//Sep. 21, 2002 genta
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

//	From Here Sep 8, 2000 genta
//
//!	Perl用アウトライン解析機能（簡易版）
/*!
	単純に /^\\s*sub\\s+(\\w+)/ に一致したら $1を取り出す動作を行う。
	ネストとかは面倒くさいので考えない。
	package{ }を使わなければこれで十分．無いよりはまし。

	@par nModeの意味
	@li 0: はじめ
	@li 2: subを見つけた後
	@li 1: 単語読み出し中

	@date 2005.06.18 genta パッケージ区切りを表す ::と'を考慮するように
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const WCHAR*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	WCHAR		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* 単語読み込み中 */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub の一文字目かもしれない
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, LTEXT("sub") ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == LCHAR(' ') || c == LCHAR('\t') ){
					nMode = 2;	//	発見
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					continue;
				}
				if( LCHAR('_') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )
				){
					//	関数名の始まり
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = LCHAR('\0');
					nMode = 1;
					continue;
				}
				else
					break;
			}
			else if( 1 == nMode ){
				if( LCHAR('_') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
					//	Jun. 18, 2005 genta パッケージ修飾子を考慮
					//	コロンは2つ連続しないといけないのだが，そこは手抜き
					LCHAR(':') == pLine[i] || LCHAR('\'') == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = LCHAR('\0');
					}
				}else{
					//	関数名取得
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					CLayoutPoint ptPosXY;
					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(CLogicInt(0), nLineCount),
						&ptPosXY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPosXY.GetY2() + CLayoutInt(1), szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta

const WCHAR* g_ppszKeywordsPERL[] = {
	//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
	LTEXT("break"),
	LTEXT("continue"),
	LTEXT("do"),
	LTEXT("elsif"),
	LTEXT("else"),
	LTEXT("for"),
	LTEXT("foreach"),
	LTEXT("goto"),
	LTEXT("if"),
	LTEXT("last"),
	LTEXT("next"),
	LTEXT("return"),
	LTEXT("sub"),
	LTEXT("undef"),
	LTEXT("unless"),
	LTEXT("until"),
	LTEXT("while"),
	LTEXT("abs"),
	LTEXT("accept"),
	LTEXT("alarm"),
	LTEXT("atan2"),
	LTEXT("bind"),
	LTEXT("binmode"),
	LTEXT("bless"),
	LTEXT("caller"),
	LTEXT("chdir"),
	LTEXT("chmod"),
	LTEXT("chomp"),
	LTEXT("chop"),
	LTEXT("chown"),
	LTEXT("chr"),
	LTEXT("chroot"),
	LTEXT("close"),
	LTEXT("closedir"),
	LTEXT("connect"),
	LTEXT("continue"),
	LTEXT("cos"),
	LTEXT("crypt"),
	LTEXT("dbmclose"),
	LTEXT("dbmopen"),
	LTEXT("defined"),
	LTEXT("delete"),
	LTEXT("die"),
	LTEXT("do"),
	LTEXT("dump"),
	LTEXT("each"),
	LTEXT("eof"),
	LTEXT("eval"),
	LTEXT("exec"),
	LTEXT("exists"),
	LTEXT("exit"),
	LTEXT("exp"),
	LTEXT("fcntl"),
	LTEXT("fileno"),
	LTEXT("flock"),
	LTEXT("fork"),
	LTEXT("format"),
	LTEXT("formline"),
	LTEXT("getc"),
	LTEXT("getlogin"),
	LTEXT("getpeername"),
	LTEXT("getpgrp"),
	LTEXT("getppid"),
	LTEXT("getpriority"),
	LTEXT("getpwnam"),
	LTEXT("getgrnam"),
	LTEXT("gethostbyname"),
	LTEXT("getnetbyname"),
	LTEXT("getprotobyname"),
	LTEXT("getpwuid"),
	LTEXT("getgrgid"),
	LTEXT("getservbyname"),
	LTEXT("gethostbyaddr"),
	LTEXT("getnetbyaddr"),
	LTEXT("getprotobynumber"),
	LTEXT("getservbyport"),
	LTEXT("getpwent"),
	LTEXT("getgrent"),
	LTEXT("gethostent"),
	LTEXT("getnetent"),
	LTEXT("getprotoent"),
	LTEXT("getservent"),
	LTEXT("setpwent"),
	LTEXT("setgrent"),
	LTEXT("sethostent"),
	LTEXT("setnetent"),
	LTEXT("setprotoent"),
	LTEXT("setservent"),
	LTEXT("endpwent"),
	LTEXT("endgrent"),
	LTEXT("endhostent"),
	LTEXT("endnetent"),
	LTEXT("endprotoent"),
	LTEXT("endservent"),
	LTEXT("getsockname"),
	LTEXT("getsockopt"),
	LTEXT("glob"),
	LTEXT("gmtime"),
	LTEXT("goto"),
	LTEXT("grep"),
	LTEXT("hex"),
	LTEXT("import"),
	LTEXT("index"),
	LTEXT("int"),
	LTEXT("ioctl"),
	LTEXT("join"),
	LTEXT("keys"),
	LTEXT("kill"),
	LTEXT("last"),
	LTEXT("lc"),
	LTEXT("lcfirst"),
	LTEXT("length"),
	LTEXT("link"),
	LTEXT("listen"),
	LTEXT("local"),
	LTEXT("localtime"),
	LTEXT("log"),
	LTEXT("lstat"),
//			"//m",
	LTEXT("map"),
	LTEXT("mkdir"),
	LTEXT("msgctl"),
	LTEXT("msgget"),
	LTEXT("msgsnd"),
	LTEXT("msgrcv"),
	LTEXT("my"),
	LTEXT("next"),
	LTEXT("no"),
	LTEXT("oct"),
	LTEXT("open"),
	LTEXT("opendir"),
	LTEXT("ord"),
	LTEXT("our"),	// 2006.04.20 genta
	LTEXT("pack"),
	LTEXT("package"),
	LTEXT("pipe"),
	LTEXT("pop"),
	LTEXT("pos"),
	LTEXT("print"),
	LTEXT("printf"),
	LTEXT("prototype"),
	LTEXT("push"),
//			"//q",
	LTEXT("qq"),
	LTEXT("qr"),
	LTEXT("qx"),
	LTEXT("qw"),
	LTEXT("quotemeta"),
	LTEXT("rand"),
	LTEXT("read"),
	LTEXT("readdir"),
	LTEXT("readline"),
	LTEXT("readlink"),
	LTEXT("readpipe"),
	LTEXT("recv"),
	LTEXT("redo"),
	LTEXT("ref"),
	LTEXT("rename"),
	LTEXT("require"),
	LTEXT("reset"),
	LTEXT("return"),
	LTEXT("reverse"),
	LTEXT("rewinddir"),
	LTEXT("rindex"),
	LTEXT("rmdir"),
//			"//s",
	LTEXT("scalar"),
	LTEXT("seek"),
	LTEXT("seekdir"),
	LTEXT("select"),
	LTEXT("semctl"),
	LTEXT("semget"),
	LTEXT("semop"),
	LTEXT("send"),
	LTEXT("setpgrp"),
	LTEXT("setpriority"),
	LTEXT("setsockopt"),
	LTEXT("shift"),
	LTEXT("shmctl"),
	LTEXT("shmget"),
	LTEXT("shmread"),
	LTEXT("shmwrite"),
	LTEXT("shutdown"),
	LTEXT("sin"),
	LTEXT("sleep"),
	LTEXT("socket"),
	LTEXT("socketpair"),
	LTEXT("sort"),
	LTEXT("splice"),
	LTEXT("split"),
	LTEXT("sprintf"),
	LTEXT("sqrt"),
	LTEXT("srand"),
	LTEXT("stat"),
	LTEXT("study"),
	LTEXT("sub"),
	LTEXT("substr"),
	LTEXT("symlink"),
	LTEXT("syscall"),
	LTEXT("sysopen"),
	LTEXT("sysread"),
	LTEXT("sysseek"),
	LTEXT("system"),
	LTEXT("syswrite"),
	LTEXT("tell"),
	LTEXT("telldir"),
	LTEXT("tie"),
	LTEXT("tied"),
	LTEXT("time"),
	LTEXT("times"),
	LTEXT("tr"),
	LTEXT("truncate"),
	LTEXT("uc"),
	LTEXT("ucfirst"),
	LTEXT("umask"),
	LTEXT("undef"),
	LTEXT("unlink"),
	LTEXT("unpack"),
	LTEXT("untie"),
	LTEXT("unshift"),
	LTEXT("use"),
	LTEXT("utime"),
	LTEXT("values"),
	LTEXT("vec"),
	LTEXT("wait"),
	LTEXT("waitpid"),
	LTEXT("wantarray"),
	LTEXT("warn"),
	LTEXT("write")
};
int g_nKeywordsPERL = int(std::size(g_ppszKeywordsPERL));

//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
// 2008/05/05 novice 重複文字列削除
const WCHAR* g_ppszKeywordsPERL2[] = {
	LTEXT("$ARGV"),
	LTEXT("$_"),
	LTEXT("$1"),
	LTEXT("$2"),
	LTEXT("$3"),
	LTEXT("$4"),
	LTEXT("$5"),
	LTEXT("$6"),
	LTEXT("$7"),
	LTEXT("$8"),
	LTEXT("$9"),
	LTEXT("$0"),
	LTEXT("$MATCH"),
	LTEXT("$&"),
	LTEXT("$PREMATCH"),
	LTEXT("$`"),
	LTEXT("$POSTMATCH"),
	LTEXT("$'"),
	LTEXT("$LAST_PAREN_MATCH"),
	LTEXT("$+"),
	LTEXT("$MULTILINE_MATCHING"),
	LTEXT("$*"),
	LTEXT("$INPUT_LINE_NUMBER"),
	LTEXT("$NR"),
	LTEXT("$."),
	LTEXT("$INPUT_RECORD_SEPARATOR"),
	LTEXT("$RS"),
	LTEXT("$/"),
	LTEXT("$OUTPUT_AUTOFLUSH"),
	LTEXT("$|"),
	LTEXT("$OUTPUT_FIELD_SEPARATOR"),
	LTEXT("$OFS"),
	LTEXT("$,"),
	LTEXT("$OUTPUT_RECORD_SEPARATOR"),
	LTEXT("$ORS"),
	LTEXT("$\\"),
	LTEXT("$LIST_SEPARATOR"),
	LTEXT("$\""),
	LTEXT("$SUBSCRIPT_SEPARATOR"),
	LTEXT("$SUBSEP"),
	LTEXT("$;"),
	LTEXT("$OFMT"),
	LTEXT("$#"),
	LTEXT("$FORMAT_PAGE_NUMBER"),
	LTEXT("$%"),
	LTEXT("$FORMAT_LINES_PER_PAGE"),
	LTEXT("$="),
	LTEXT("$FORMAT_LINES_LEFT"),
	LTEXT("$-"),
	LTEXT("$FORMAT_NAME"),
	LTEXT("$~"),
	LTEXT("$FORMAT_TOP_NAME"),
	LTEXT("$^"),
	LTEXT("$FORMAT_LINE_BREAK_CHARACTERS"),
	LTEXT("$:"),
	LTEXT("$FORMAT_FORMFEED"),
	LTEXT("$^L"),
	LTEXT("$ACCUMULATOR"),
	LTEXT("$^A"),
	LTEXT("$CHILD_ERROR"),
	LTEXT("$?"),
	LTEXT("$OS_ERROR"),
	LTEXT("$ERRNO"),
	LTEXT("$!"),
	LTEXT("$EVAL_ERROR"),
	LTEXT("$@"),
	LTEXT("$PROCESS_ID"),
	LTEXT("$PID"),
	LTEXT("$$"),
	LTEXT("$REAL_USER_ID"),
	LTEXT("$UID"),
	LTEXT("$<"),
	LTEXT("$EFFECTIVE_USER_ID"),
	LTEXT("$EUID"),
	LTEXT("$>"),
	LTEXT("$REAL_GROUP_ID"),
	LTEXT("$GID"),
	LTEXT("$("),
	LTEXT("$EFFECTIVE_GROUP_ID"),
	LTEXT("$EGID"),
	LTEXT("$)"),
	LTEXT("$PROGRAM_NAME"),
	LTEXT("$["),
	LTEXT("$PERL_VERSION"),
	LTEXT("$]"),
	LTEXT("$DEBUGGING"),
	LTEXT("$^D"),
	LTEXT("$SYSTEM_FD_MAX"),
	LTEXT("$^F"),
	LTEXT("$INPLACE_EDIT"),
	LTEXT("$^I"),
	LTEXT("$PERLDB"),
	LTEXT("$^P"),
	LTEXT("$BASETIME"),
	LTEXT("$^T"),
	LTEXT("$WARNING"),
	LTEXT("$^W"),
	LTEXT("$EXECUTABLE_NAME"),
	LTEXT("$^X"),
	LTEXT("$ENV"),
	LTEXT("$SIG")
};
int g_nKeywordsPERL2 = int(std::size(g_ppszKeywordsPERL2));
