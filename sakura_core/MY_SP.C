//$Id$
/*!	@file
	@brief 引数を正しく無視するsplitpath

	@author SUI
	@date 2002.07.15 Initial release
	$Revision$
*/
/*
	Copyright (C) 2002, SUI

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
/* #define	_USE_MS_DOS_FUNC_	/* */
/* ↑ファイルの属性取得に _dos_getfileattr (Dos function) を使用する場合に定義
　 しておく。未定義の場合は GetFileAttributes (Windows API) を使用する。
　 主に LSI-C 試食版で動作確認していたもので。 */

#ifdef	LSI_C	/* LSI-C なら絶対 _dos_getfileattr を使う */
#ifndef	_USE_MS_DOS_FUNC_
#define	_USE_MS_DOS_FUNC_
#endif
#endif	/* LSI_C */

#include	<io.h>		/* access */
#include	<string.h>	/* strncpy strlen */
#include	<stdlib.h>	/* _MAX_PATH _MAX_DRIVE _MAX_DIR _MAX_FNAME _MAX_EXT */

/* LSI-C 試食版では↓この辺が無いので．．． */
#ifndef		_MAX_PATH
#define		_MAX_PATH	260		/* */
/*#define		_MAX_PATH	28		/* for DEBUG */
/*#define		_MAX_PATH	27		/* for DEBUG */
#endif
#ifndef		_MAX_DRIVE
#define		_MAX_DRIVE	3
#endif
#ifndef		_MAX_DIR
#define		_MAX_DIR	256
#endif
#ifndef		_MAX_FNAME
#define		_MAX_FNAME	256
#endif
#ifndef		_MAX_EXT
#define		_MAX_EXT	256
#endif

#ifdef	_USE_MS_DOS_FUNC_
	#include	<dos.h>		/* _dos_getfileattr _A_SUBDIR _dos_getvect _dos_setvect */
#else
	#include	<windows.h>	/* GetFileAttributes FILE_ATTRIBUTE_DIRECTORY */
#endif

#include "MY_SP.h" // Feb. 12, 2003 MIK 抜けていた

/* ============================================================================
my_splitpath( const char *CommandLine, char *drive, char *dir, char *fname, char *ext );

★ 概要
CommandLine に与えられたコマンドライン文字列の先頭から、実在するファイル・ディ
レクトリの部分のみを抽出し、その抽出部分に対して _splitpath() と同等の処理をお
こないます。
先頭部分に実在するファイル・ディレクトリ名が無い場合は空文字列が返ります。
文字列中の日本語(Shift_JISコードのみ)に対応しています。

★ プログラム記述内容について(言い訳あれこれ)
文字列の split 処理部だけにでも _splitpath() を使えばもっと短くなったのですが、
そうやらずに全て自前で処理している理由は、
・コンパイラによっては _splitpath() が日本語に対応していない可能性もある。
・_splitpath() って、コンパイラによって、詳細動作が微妙に異なるかもしれないから
　仕様をハッキリさせるためにもコンパイラに添付されている _splitpath() にあまり
　頼りたくなかった。
・というか、主に動作確認に使用していた LSI-C試食版にはそもそも _splitpath() が
　存在しないから、やらざるをえなかった。 :-(
という事によります。

あらかじめ _USE_MS_DOS_FUNC_ が #define されていると MS-DOS ファンクションの関
数（_dos_getfileattr）が使用されます。( LSI-C試食版なんか向け)
#define定義されていない場合、Windows の GetFileAttributes が使用されます。
（ Windows ソフトウェア向け）

動作確認は主に LSI-C 試食版(+ LFN library Ver.1.1)にておこなっています。

注意点としては、DOS版コンパイラでコンパイルしたMS-DOS用.exe の場合、フロッピー
ディスクなどのリムーバブルディスクのドライブのパスが渡されて来た時、そのドライ
ブのディスクが用意できていない場合、
 |準備ができていません 読み取り中 ドライブ X:
 |中止(A), 再試行(R), 失敗(F)?
という、お馴染みのメッセージが登場してしまうという事です。
# Windows版の.exe では Windows API が使用されるので出ないはずです。
MS-DOSにて、このうっとうしいメッセージを出さないようにするには、割り込みベクタ
の設定をおこなわなくてはなりません。
（ _dos_getvect (DOS func.35H) , _dos_setvect (DOS func.25H) で 24H の割り込み
　ベクタをいじる）
が、これはかなりコンパイラに依存する記述になってしまいます。
ので、結局、今この部分(メッセージを出さない処理)は LSI-C のみ対応のプログラムに
なっています。
「 LSI-C でコンパイルした場合」または「 Windows版.exe を作成した場合」*以外* の
場合は上記メッセージが出てしまいます。

※ "LFN library" -> http://webs.to/ken/

★ 詳細動作
my_splitpath( CommandLine, drive, dir, fname, ext );
CommandLine に文字列として D:\Test.ext が与えられた場合、
├・D:\Test.ext というファイルが存在する場合
│　drive = "D:"
│　dir   = "\"
│　fname = "Test"
│　ext   = ".ext"
├・D:\Test.ext というディレクトリが存在する場合
│　drive = "D:"
│　dir   = "\Test.ext\"
│　fname = ""
│　ext   = ""
└・D:\Test.ext というファイル・ディレクトリが存在しない場合、
　　├・D:ドライブは有効
　　│　drive = "D:"
　　│　dir   = "\"
　　│　fname = ""
　　│　ext   = ""
　　└・D:ドライブは無効
　　　　drive = ""
　　　　dir   = ""
　　　　fname = ""
　　　　ext   = ""
)=========================================================================== */

/* ↓ Shift_JIS の漢字の1バイト目の判定 */
#define	_IS_SJIS_1(ch)	( ( ( ch >=0x081 )&&( ch <=0x09F ) ) || ( ( ch >=0x0E0 )&&( ch <=0x0FC ) ) )

/* Shift_JIS 対応で検索対象文字を２個指定できる strrchr() みたいなもの。
/ 指定された２つの文字のうち、見つかった方(より後方の方)の位置を返す。
/ # strrchr( char *s , char c ) とは、文字列 s 中の最後尾の c を探し出す関数。
/ # 文字 c が見つかったら、その位置を返す。
/ # 文字 c が見つからない場合は NULL を返す。 */
char	*sjis_strrchr2( unsigned char *pt , const unsigned char ch1 , const unsigned char ch2 ){
	unsigned char	*pf = NULL;
	while( *pt != '\0' ){	/* 文字列の終端まで調べる。 */
		if( ( *pt == ch1 ) || ( *pt == ch2 ) )	pf = pt;	/* pf = 検索文字の位置 */
		if( _IS_SJIS_1(*pt) )	pt++;	/* Shift_JIS の1文字目なら、次の1文字をスキップ */
		if( *pt != '\0' )		pt++;	/* 次の文字へ */
	}
	return	pf;
}

/* 文字列 pi 中の先頭にある有効なパス名（実在するファイル・ディレクトリ名）部分
　 のみを po に書き出す。po は char po[_MAX_PATH]; 確保しておく事。 */
#ifdef	LSI_C	/* LSI-C の場合。 */
void far	_asm_void_( char * );
void far	int_nop(){	_asm_void_( "IRET\n" );	}
#endif	/* LSI_C */

#define		GetExistPath_NO_DriveLetter	0	/* ドライブレターが無い */
#define		GetExistPath_IV_Drive		1	/* ドライブが無効 */
#define		GetExistPath_AV_Drive		2	/* ドライブが有効 */

void	GetExistPath( char *po , const char *pi ) {
	char	*pw,*ps;
	int		cnt;
	char	drv[4] = "_:\\";
	int		dl;		/* ドライブの状態 */
	#ifdef	LSI_C
	void	far		*int24h;
	#endif	/* LSI_C */

	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 ( _MAX_PATH -1 ) 文字まで
	/ po にコピーする。 */
	for( pw=po,cnt=0 ; ( *pi != '\0' ) && ( cnt < _MAX_PATH -1 ) ; pi++ ){
		/* /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。 */
		if( *pi == '\"' )	continue;		/*  " なら何もしない。次の文字へ */
		if( *pi == '/' )	*pw++ = '\\';	/*  / なら \ に変換してコピー    */
		else				*pw++ = *pi;	/* その他の文字はそのままコピー  */
		cnt++;	/* コピーした文字数 ++ */
	}
	*pw = '\0';		/* 文字列終端 */

	#ifdef	LSI_C	/* LSI-C の場合のみ */
	/* 準備ができていません 読み取り中 ドライブ X:
	　 中止(A), 再試行(R), 失敗(F)?
	　 を出さないようにする。 */
	int24h = _dos_getvect(0x24);	/* 割込みベクタ待避 */
	_dos_setvect(0x24,int_nop);		/* 何もしない処理に入れ替える */
	#endif	/* LSI_C */

	dl = GetExistPath_NO_DriveLetter;	/*「ドライブレターが無い」にしておく*/
	if(
		( *(po+1) == ':' )&&
		(
			( ( *po >='A' )&&( *po <='Z' ) ) ||
			( ( *po >='a' )&&( *po <='z' ) )
		)
	){	/* 先頭にドライブレターがある。そのドライブが有効かどうか判定する */
		drv[0] = *po;
/*		printf("chk>{%s}\n",drv);	/* for DEBUG */
		if( access(drv,0) == 0 )	dl = GetExistPath_AV_Drive;		/* 有効 */
		else						dl = GetExistPath_IV_Drive;		/* 無効 */
	}

	#ifdef	LSI_C	/* LSI-C の場合のみ */
	_dos_setvect(0x24,int24h);	/* 割込みベクタ復活 */
	#endif	/* LSI_C */

	if( dl == GetExistPath_IV_Drive ){	/* ドライブ自体が無効 */
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = '\0';	/* 返値文字列 = "";(空文字列) */
		return;		/* これ以上何もしない */
	}

	/* ps = 検索開始位置 */
	ps = po;	/* ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す */
	if( ( *po == '\\' )&&( *(po+1) == '\\' ) )	ps +=2;

	if( *ps == '\0' ){	/* 検索対象が空文字列なら */
		*po = '\0';		/* 返値文字列 = "";(空文字列) */
		return;			/*これ以上何もしない */
	}

	for(;;){
/*		printf("chk>{%s}\n",po);		/* for DEBUG */
		if( access(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		/* ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。*/

		pw = sjis_strrchr2(ps,'\\',' ');	/* 最末尾の \ か ' ' を探す。 */
		if ( pw == NULL ){	/* 文字列中に '\\' も ' ' も無かった */
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if( dl == GetExistPath_AV_Drive ){
				/* 先頭に有効なドライブのドライブレターがある。 */
				*(po+2) = '\0';		/* ドライブレター部の文字列のみ返す */
			}
			else{	/* 有効なパス部分が全く見つからなかった */
				*po = '\0';	/* 返値文字列 = "";(空文字列) */
			}
			break;		/* ループを抜ける */
		}
		/* ↓ルートディレクトリを引っかけるための処理 */
		if( ( *pw == '\\' )&&( *(pw-1) == ':' ) ){	/* C:\ とかの \ っぽい */
			* (pw+1) = '\0';		/* \ の後ろの位置を文字列の終端にする。 */
/*			printf("chk>{%s}\n",po);		/* for DEBUG */
			if( access(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		}
		*pw = '\0';		/* \ か ' ' の位置を文字列の終端にする。 */
		/* ↓末尾がスペースなら、スペースを全て削除する */
		while( ( pw != ps ) && ( *(pw-1) == ' ' ) )	* --pw = '\0';
	}

	return;
}

/* 与えられたコマンドライン文字列の先頭部分から実在するファイル・ディレクトリ
　 のパス文字列を抽出し、そのパスを分解して drv dir fnm ext に書き込む。
　 先頭部分に有効なパス名が存在しない場合、全てに空文字列が返る。 */
void	my_splitpath ( const char *comln , char *drv,char *dir,char *fnm,char *ext ){
	char	ppp[_MAX_PATH];		/* パス格納（作業用） */
	unsigned char	*pd;
	unsigned char	*pf;
	unsigned char	*pe;
	unsigned char	ch;
	#ifdef	_USE_MS_DOS_FUNC_
		unsigned	attr;
	#else
		DWORD	attr;
	#endif
	int		a_dir;

	if( drv != NULL )	*drv = '\0';
	if( dir != NULL )	*dir = '\0';
	if( fnm != NULL )	*fnm = '\0';
	if( ext != NULL )	*ext = '\0';
	if( *comln == '\0' )	return;

	/* コマンドライン先頭部分の実在するパス名を ppp に書き出す。 */
	GetExistPath( ppp , comln );

	if( *ppp != '\0' ) {	/* ファイル・ディレクトリが存在する場合 */
		/* 先頭文字がドライブレターかどうか判定し、
		　 pd = ディレクトリ名の先頭位置に設定する。 */
		pd = ppp;
		if(
			( *(pd+1)==':' )&&
			( ( ( *pd >='A' )&&( *pd <='Z' ) ) || ( ( *pd >='a' )&&( *pd <='z' ) ) )
		){	/* 先頭にドライブレターがある。 */
			pd += 2;	/* pd = ドライブレター部の後ろ         */
		}				/*      ( = ディレクトリ名の先頭位置 ) */
		/* ここまでで、pd = ディレクトリ名の先頭位置 */

		#ifdef	_USE_MS_DOS_FUNC_
			_dos_getfileattr(ppp,&attr);
			a_dir = ( attr & _A_SUBDIR ) ? 1 : 0;
		#else
			attr =  GetFileAttributes(ppp);
			a_dir = ( attr & FILE_ATTRIBUTE_DIRECTORY ) ?  1 : 0;
		#endif
/*		if( a_dir )	printf("find>{%s} <DIR>\n",ppp);		/* for DEBUG */
/*		else		printf("find>{%s} <FILE>\n",ppp);		/* for DEBUG */
		if( ! a_dir ){	/* 見つけた物がファイルだった場合。 */
			pf = sjis_strrchr2(ppp,'\\','\\');	/* 最末尾の \ を探す。 */
			if(pf != NULL)	pf++;		/* 見つかった→  pf=\の次の文字の位置*/
			else			pf = pd;	/* 見つからない→pf=パス名の先頭位置 */
			/* ここまでで pf = ファイル名の先頭位置 */
			pe = sjis_strrchr2(pf,'.','.');		/* 最末尾の '.' を探す。 */
			if( pe != NULL ){					/* 見つかった(pe = '.'の位置)*/
				if( ext != NULL ){	/* 拡張子を返値として書き込む。 */
					strncpy(ext,pe,_MAX_EXT -1);
					ext[_MAX_EXT -1] = '\0';
				}
				*pe = '\0';	/* 区切り位置を文字列終端にする。pe = 拡張子名の先頭位置。 */
			}
			if( fnm != NULL ){	/* ファイル名を返値として書き込む。 */
				strncpy(fnm,pf,_MAX_FNAME -1);
				fnm[_MAX_FNAME -1] = '\0';
			}
			*pf = '\0';	/* ファイル名の先頭位置を文字列終端にする。 */
		}
		/* ここまでで文字列 ppp はドライブレター＋ディレクトリ名のみになっている */
		if( dir != NULL ){
			/* ディレクトリ名の最後の文字が \ ではない場合、\ にする。 */

			/* ↓最後の文字を ch に得る。(ディレクトリ文字列が空の場合 ch='\\' となる) */
			for( ch = '\\' , pf = pd ; *pf != '\0' ; pf++ ){
				ch = *pf;
				if( _IS_SJIS_1(*pf) )	pf++;	/* Shift_JIS の1文字目なら次の1文字をスキップ */
			}
			/* 文字列が空でなく、かつ、最後の文字が \ でなかったならば \ を追加。 */
			if( ( ch != '\\' ) && ( strlen(ppp) < _MAX_PATH -1 ) ){
				*pf++ = '\\';	*pf = '\0';
			}
/*			printf("%d:{%s}\n",strlen(ppp),ppp);	/* for DEBUG */

			/* ディレクトリ名を返値として書き込む。 */
			strncpy(dir,pd,_MAX_DIR -1);
			dir[_MAX_DIR -1] = '\0';
		}
		*pd = '\0';		/* ディレクトリ名の先頭位置を文字列終端にする。 */
		if( drv != NULL ){	/* ドライブレターを返値として書き込む。 */
			strncpy(drv,ppp,_MAX_DRIVE -1);
			drv[_MAX_DRIVE -1] = '\0';
		}
	}
	return;
}

/* file end */
