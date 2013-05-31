#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ���[�U�ɑ���
//Jul. 08, 2001 JEPRO �ǉ�
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Perl") );
	_tcscpy( pType->m_szTypeExts, _T("cgi,pl,pm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_eDefaultOutline = OUTLINE_PERL;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 12;								/* �L�[���[�h�Z�b�g2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* ���p���l��F�����\�� */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//�Ί��ʂ̋������f�t�H���gON	//Sep. 21, 2002 genta
}



//	From Here Sep 8, 2000 genta
//
//!	Perl�p�A�E�g���C����͋@�\�i�ȈՔŁj
/*!
	�P���� /^\\s*sub\\s+(\\w+)/ �Ɉ�v������ $1�����o��������s���B
	�l�X�g�Ƃ��͖ʓ|�������̂ōl���Ȃ��B
	package{ }���g��Ȃ���΂���ŏ\���D�������͂܂��B

	@par nMode�̈Ӗ�
	@li 0: �͂���
	@li 2: sub����������
	@li 1: �P��ǂݏo����

	@date 2005.06.18 genta �p�b�P�[�W��؂��\�� ::��'���l������悤��
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* �P��ǂݍ��ݒ� */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i])
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub �̈ꕶ���ڂ�������Ȃ�
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, L"sub" ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == L' ' || c == L'\t' ){
					nMode = 2;	//	����
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i])
				){
					continue;
				}
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
				){
					//	�֐����̎n�܂�
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = L'\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					//	Jun. 18, 2005 genta �p�b�P�[�W�C���q���l��
					//	�R������2�A�����Ȃ��Ƃ����Ȃ��̂����C�����͎蔲��
					L':' == pLine[i] || L'\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	�֐����擾
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  ��
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
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




const wchar_t* g_ppszKeywordsPERL[] = {
	//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
	L"break",
	L"continue",
	L"do",
	L"elsif",
	L"else",
	L"for",
	L"foreach",
	L"goto",
	L"if",
	L"last",
	L"next",
	L"return",
	L"sub",
	L"undef",
	L"unless",
	L"until",
	L"while",
	L"abs",
	L"accept",
	L"alarm",
	L"atan2",
	L"bind",
	L"binmode",
	L"bless",
	L"caller",
	L"chdir",
	L"chmod",
	L"chomp",
	L"chop",
	L"chown",
	L"chr",
	L"chroot",
	L"close",
	L"closedir",
	L"connect",
	L"continue",
	L"cos",
	L"crypt",
	L"dbmclose",
	L"dbmopen",
	L"defined",
	L"delete",
	L"die",
	L"do",
	L"dump",
	L"each",
	L"eof",
	L"eval",
	L"exec",
	L"exists",
	L"exit",
	L"exp",
	L"fcntl",
	L"fileno",
	L"flock",
	L"fork",
	L"format",
	L"formline",
	L"getc",
	L"getlogin",
	L"getpeername",
	L"getpgrp",
	L"getppid",
	L"getpriority",
	L"getpwnam",
	L"getgrnam",
	L"gethostbyname",
	L"getnetbyname",
	L"getprotobyname",
	L"getpwuid",
	L"getgrgid",
	L"getservbyname",
	L"gethostbyaddr",
	L"getnetbyaddr",
	L"getprotobynumber",
	L"getservbyport",
	L"getpwent",
	L"getgrent",
	L"gethostent",
	L"getnetent",
	L"getprotoent",
	L"getservent",
	L"setpwent",
	L"setgrent",
	L"sethostent",
	L"setnetent",
	L"setprotoent",
	L"setservent",
	L"endpwent",
	L"endgrent",
	L"endhostent",
	L"endnetent",
	L"endprotoent",
	L"endservent",
	L"getsockname",
	L"getsockopt",
	L"glob",
	L"gmtime",
	L"goto",
	L"grep",
	L"hex",
	L"import",
	L"index",
	L"int",
	L"ioctl",
	L"join",
	L"keys",
	L"kill",
	L"last",
	L"lc",
	L"lcfirst",
	L"length",
	L"link",
	L"listen",
	L"local",
	L"localtime",
	L"log",
	L"lstat",
//			"//m",
	L"map",
	L"mkdir",
	L"msgctl",
	L"msgget",
	L"msgsnd",
	L"msgrcv",
	L"my",
	L"next",
	L"no",
	L"oct",
	L"open",
	L"opendir",
	L"ord",
	L"our",	// 2006.04.20 genta
	L"pack",
	L"package",
	L"pipe",
	L"pop",
	L"pos",
	L"print",
	L"printf",
	L"prototype",
	L"push",
//			"//q",
	L"qq",
	L"qr",
	L"qx",
	L"qw",
	L"quotemeta",
	L"rand",
	L"read",
	L"readdir",
	L"readline",
	L"readlink",
	L"readpipe",
	L"recv",
	L"redo",
	L"ref",
	L"rename",
	L"require",
	L"reset",
	L"return",
	L"reverse",
	L"rewinddir",
	L"rindex",
	L"rmdir",
//			"//s",
	L"scalar",
	L"seek",
	L"seekdir",
	L"select",
	L"semctl",
	L"semget",
	L"semop",
	L"send",
	L"setpgrp",
	L"setpriority",
	L"setsockopt",
	L"shift",
	L"shmctl",
	L"shmget",
	L"shmread",
	L"shmwrite",
	L"shutdown",
	L"sin",
	L"sleep",
	L"socket",
	L"socketpair",
	L"sort",
	L"splice",
	L"split",
	L"sprintf",
	L"sqrt",
	L"srand",
	L"stat",
	L"study",
	L"sub",
	L"substr",
	L"symlink",
	L"syscall",
	L"sysopen",
	L"sysread",
	L"sysseek",
	L"system",
	L"syswrite",
	L"tell",
	L"telldir",
	L"tie",
	L"tied",
	L"time",
	L"times",
	L"tr",
	L"truncate",
	L"uc",
	L"ucfirst",
	L"umask",
	L"undef",
	L"unlink",
	L"unpack",
	L"untie",
	L"unshift",
	L"use",
	L"utime",
	L"values",
	L"vec",
	L"wait",
	L"waitpid",
	L"wantarray",
	L"warn",
	L"write"
};
int g_nKeywordsPERL = _countof(g_ppszKeywordsPERL);



//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
// 2008/05/05 novice �d��������폜
const wchar_t* g_ppszKeywordsPERL2[] = {
	L"$ARGV",
	L"$_",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$0",
	L"$MATCH",
	L"$&",
	L"$PREMATCH",
	L"$`",
	L"$POSTMATCH",
	L"$'",
	L"$LAST_PAREN_MATCH",
	L"$+",
	L"$MULTILINE_MATCHING",
	L"$*",
	L"$INPUT_LINE_NUMBER",
	L"$NR",
	L"$.",
	L"$INPUT_RECORD_SEPARATOR",
	L"$RS",
	L"$/",
	L"$OUTPUT_AUTOFLUSH",
	L"$|",
	L"$OUTPUT_FIELD_SEPARATOR",
	L"$OFS",
	L"$,",
	L"$OUTPUT_RECORD_SEPARATOR",
	L"$ORS",
	L"$\\",
	L"$LIST_SEPARATOR",
	L"$\"",
	L"$SUBSCRIPT_SEPARATOR",
	L"$SUBSEP",
	L"$;",
	L"$OFMT",
	L"$#",
	L"$FORMAT_PAGE_NUMBER",
	L"$%",
	L"$FORMAT_LINES_PER_PAGE",
	L"$=",
	L"$FORMAT_LINES_LEFT",
	L"$-",
	L"$FORMAT_NAME",
	L"$~",
	L"$FORMAT_TOP_NAME",
	L"$^",
	L"$FORMAT_LINE_BREAK_CHARACTERS",
	L"$:",
	L"$FORMAT_FORMFEED",
	L"$^L",
	L"$ACCUMULATOR",
	L"$^A",
	L"$CHILD_ERROR",
	L"$?",
	L"$OS_ERROR",
	L"$ERRNO",
	L"$!",
	L"$EVAL_ERROR",
	L"$@",
	L"$PROCESS_ID",
	L"$PID",
	L"$$",
	L"$REAL_USER_ID",
	L"$UID",
	L"$<",
	L"$EFFECTIVE_USER_ID",
	L"$EUID",
	L"$>",
	L"$REAL_GROUP_ID",
	L"$GID",
	L"$(",
	L"$EFFECTIVE_GROUP_ID",
	L"$EGID",
	L"$)",
	L"$PROGRAM_NAME",
	L"$[",
	L"$PERL_VERSION",
	L"$]",
	L"$DEBUGGING",
	L"$^D",
	L"$SYSTEM_FD_MAX",
	L"$^F",
	L"$INPLACE_EDIT",
	L"$^I",
	L"$PERLDB",
	L"$^P",
	L"$BASETIME",
	L"$^T",
	L"$WARNING",
	L"$^W",
	L"$EXECUTABLE_NAME",
	L"$^X",
	L"$ENV",
	L"$SIG"
};
int g_nKeywordsPERL2 = _countof(g_ppszKeywordsPERL2);
