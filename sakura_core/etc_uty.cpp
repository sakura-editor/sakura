/*!	@file
	@brief 共通関数群

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, shoji masami, Misaka, Stonee, MIK, YAZAKI
	Copyright (C) 2002, genta, aroka, hor, MIK, 鬼, Moca, YAZAKI
	Copyright (C) 2003, genta, matsumo, Moca, MIK
	Copyright (C) 2004, genta, novice, Moca, MIK
	Copyright (C) 2005, genta, D.S.Koba, Moca, ryoji, aroka
	Copyright (C) 2006, genta, ryoji, rastiv
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, ryoji, nasukoji, novice
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

// 2006.04.21 ryoji マルチモニタのエミュレーション関数の実体生成のため
#define COMPILE_MULTIMON_STUBS

#include "StdAfx.h"
#include <HtmlHelp.h>
#include <io.h>
#include <memory.h>		// Apr. 03, 2003 genta
#include <CdErr.h> // Nov. 3, 2005 genta	//CDERR_FINDRESFAILURE等
#include "etc_uty.h"
#include "shell.h"
#include "file.h"
#include "os.h"
#include "Debug.h"
#include "CMemory.h"
#include "Funccode.h"	//Stonee, 2001/02/23
#include "mymessage.h"	// 2007.04.03 ryoji

#include "WinNetWk.h"	//Stonee, 2001/12/21
#include "sakura.hh"	//YAZAKI, 2001/12/11
#include "CEol.h"// 2002/2/3 aroka
#include "CBregexp.h"// 2002/2/3 aroka
#include "COsVersionInfo.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加
#include "charcode.h"  // 2006/08/28 rastiv

#include "CShareData.h"
#include "CMRUFile.h"
#include "CMRUFolder.h"
#include "CUxTheme.h"	// 2007.04.01 ryoji

int CDPI::nDpiX = 96;
int CDPI::nDpiY = 96;
bool CDPI::bInitialized = false;

/*! ヘルプファイルのフルパスを返す
 
    @return パスを格納したバッファのポインタ
 
    @note 実行ファイルと同じ位置の sakura.chm ファイルを返す。
        パスが UNC のときは _MAX_PATH に収まらない可能性がある。
 
    @date 2002/01/19 aroka ；nMaxLen 引数追加
	@date 2007/10/23 kobake 引数説明の誤りを修正(in→out)
	@date 2007/10/23 kobake CEditAppのメンバ関数に変更
	@date 2007/10/23 kobake シグニチャ変更。constポインタを返すだけのインターフェースにしました。
*/
static LPCTSTR GetHelpFilePath()
{
	static TCHAR szHelpFile[_MAX_PATH] = _T("");
	if(szHelpFile[0]==_T('\0')){
		GetExedir( szHelpFile, _T("sakura.chm") );
	}
	return szHelpFile;
}

/*!
	空白を含むファイル名を考慮したトークンの分割
	
	先頭にある連続した区切り文字は無視する．
	
	@param pBuffer [in] 文字列バッファ(終端があること)
	@param nLen [in] 文字列の長さ
	@param pnOffset [in/out] オフセット
	@param pDelimiter [in] 区切り文字
	@return トークン

	@date 2004.02.15 みく 最適化
*/
TCHAR* my_strtok( TCHAR* pBuffer, int nLen, int* pnOffset, const TCHAR* pDelimiter )
{
	int i = *pnOffset;
	TCHAR* p;

	do {
		bool bFlag = false;	//ダブルコーテーションの中か？
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == _T('"') ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( _tcschr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//空のトークンなら次を探す
	return p;
}


/** フレームウィンドウをアクティブにする
	@date 2007.11.07 ryoji 対象がdisableのときは最近のポップアップをフォアグラウンド化する．
		（モーダルダイアログやメッセージボックスを表示しているようなとき）
*/
void ActivateFrameWindow( HWND hwnd )
{
	// 編集ウィンドウでタブまとめ表示の場合は表示位置を復元する
	CShareData* pInstance = NULL;
	DLLSHAREDATA* pShareData = NULL;
	if( (pInstance = CShareData::getInstance()) && (pShareData = pInstance->GetShareData()) ){
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
			if( IsSakuraMainWindow( hwnd ) ){
				if( pShareData->m_sFlags.m_bEditWndChanging )
					return;	// 切替の最中(busy)は要求を無視する
				pShareData->m_sFlags.m_bEditWndChanging = TRUE;	// 編集ウィンドウ切替中ON	2007.04.03 ryoji

				// 対象ウィンドウのスレッドに位置合わせを依頼する	// 2007.04.03 ryoji
				DWORD_PTR dwResult;
				::SendMessageTimeout(
					hwnd,
					MYWM_TAB_WINDOW_NOTIFY,
					TWNT_WNDPL_ADJUST,
					(LPARAM)NULL,
					SMTO_ABORTIFHUNG | SMTO_BLOCK,
					10000,
					&dwResult
				);
			}
		}
	}

	// 対象がdisableのときは最近のポップアップをフォアグラウンド化する
	HWND hwndActivate;
	hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

	if( pShareData )
		pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中OFF	2007.04.03 ryoji

	return;
}




//@@@ 2002.01.24 Start by MIK
/*!
	文字列がURLかどうかを検査する。
	
	@retval TRUE URLである
	@retval FALSE URLでない
	
	@note 関数内に定義したテーブルは必ず static const 宣言にすること(性能に影響します)。
		url_char の値は url_table の配列番号+1 になっています。
		新しい URL を追加する場合は #define 値を修正してください。
		url_table は頭文字がアルファベット順になるように並べてください。
*/
BOOL IsURL(
	const char*	pszLine,	//!< [in] 文字列
	int			nLineLen,	//!< [in] 文字列の長さ
	int*		pnMatchLen	//!< [out] URLの長さ
)
{
	struct _url_table_t {
		char	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* アルファベット順 */
		{ "file://",	7,	false }, /* 1 */
		{ "ftp://",		6,	false }, /* 2 */
		{ "gopher://",	9,	false }, /* 3 */
		{ "http://",	7,	false }, /* 4 */
		{ "https://",	8,	false }, /* 5 */
		{ "mailto:",	7,	true  }, /* 6 */
		{ "news:",		5,	false }, /* 7 */
		{ "nntp://",	7,	false }, /* 8 */
		{ "prospero://",11,	false }, /* 9 */
		{ "telnet://",	9,	false }, /* 10 */
		{ "tp://",		5,	false }, /* 11 */	//2004.02.02
		{ "ttp://",		6,	false }, /* 12 */	//2004.02.02
		{ "wais://",	7,	false }, /* 13 */
		{ "{",			0,	false }  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* テーブルの保守性を高めるための定義 */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* あと128バイト犠牲にすればif文を2箇所削除できる */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const unsigned char	*p = (const unsigned char*)pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( *p & 0x80 ) return FALSE;	/* 2バイト文字 */
	if( 0 < url_char[*p] ){	/* URL開始文字 */
		for(urlp = &url_table[url_char[*p]-1]; urlp->name[0] == *p; urlp++){	/* URLテーブルを探索 */
			if( (urlp->length <= nLineLen) && (memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URLヘッダは一致した */
				p += urlp->length;	/* URLヘッダ分をスキップする */
				if( urlp->is_mail ){	/* メール専用の解析へ */
					if( IsMailAddress((const char*)p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* 通常の解析へ */
					if( (*p & 0x80) || (!(url_char[*p])) ) break;	/* 終端に達した */
				}
				if( i == urlp->length ) return FALSE;	/* URLヘッダだけ */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
}

/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
BOOL IsMailAddress( const char* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	int		j;
	int		nDotCount;
	int		nBgn;


	j = 0;
	if( (pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	 || (pszBuf[j] == '.')
	 || (pszBuf[j] == '-')
	 || (pszBuf[j] == '_')
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( '@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	while( 1 ){
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
		 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
		 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
		 || (pszBuf[j] == '-')
		 || (pszBuf[j] == '_')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( '.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}




//@@@ 2001.11.07 Start by MIK
/*
 * 数値なら長さを返す。
 * 10進数の整数または小数。16進数(正数)。
 * 文字列   数値(色分け)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (ただしソース中の#ifを有効にしたとき)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (ただしソース中の#ifを有効にしたら"0.56.1"になる)
 * .5       5        (ただしソース中の#ifを有効にしたら".5"になる)
 * -.5      5        (ただしソース中の#ifを有効にしたら"-.5"になる)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * 半角数値
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10進数, 16進数, LF接尾語, 浮動小数点数, 負符号
 *   IPアドレスのドット連結(本当は数値じゃないんだよね)
 */
int IsNumber(const char *buf, int offset, int length)
{
	register const char *p;
	register const char *q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = &buf[offset];
	q = &buf[length];

	if( *p == '0' )  /* 10進数,Cの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'x' ) )  /* Cの16進数 */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" なら "0" だけが数値 */
			if( i == 2 ) return 1;
			
			/* 接尾語 */
			if( p < q )
			{
				if( *p == 'L' || *p == 'l' || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= '0' && *p <= '9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == '.' )
		{
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == 'E' || *p == 'e' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( ( *p == '+' || *p == '-' ) && ( *(p - 1) == 'E' || *(p - 1) == 'e' ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < '0' || *(p + 1) > '9' )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" なら "0" が数値 */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" だけが数値 */
			/*if( *p == '.' ) return i - 1;*/  /* 最後が "." なら含めない */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= '1' && *p <= '9' )  /* 10進数 */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == '.' ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '-' )  /* マイナス */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." だけなら数値でない */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
		if( i == 1 ) return 0;
		if( *(p - 1) == '.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '.' )  /* 小数点 */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." だけなら数値でない */
		if( i == 1 ) return 0;
		if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == '&' )  /* VBの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'H' ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" だけなら数値でない */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" だけなら数値でない */
		return 0;
	}
#endif

	/* 数値ではない */
	return 0;
}
//@@@ 2001.11.07 End by MIK

void GetLineColumn( const char* pLine, int* pnJumpToLine, int* pnJumpToColumn )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szNumber[32];
	nLineLen = strlen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= '0' &&
			pLine[i] <= '9' ){
			break;
		}
	}
	memset( szNumber, 0, _countof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* 行位置 改行単位行番号(1起点)の抽出 */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= '0' &&
				pLine[i] <= '9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = atoi( szNumber );

		/* 桁位置 改行単位行先頭からのバイト数(1起点)の抽出 */
		if( i < nLineLen && pLine[i] == ',' ){
			memset( szNumber, 0, sizeof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= '0' &&
					pLine[i] <= '9' ){
					continue;
				}
				break;
			}
			*pnJumpToColumn = atoi( szNumber );
		}
	}
	return;
}




/* CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEol*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* 改行コードがあった */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* 行終端子の種類を調べる */
			pcEol->SetTypeByString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}




/*! 指定長以下のテキストに切り分ける

	@param pText [in] 切り分け対象となる文字列へのポインタ
	@param nTextLen [in] 切り分け対象となる文字列全体の長さ
	@param nLimitLen [in] 切り分ける長さ
	@param pnLineLen [out] 実際に取り出された文字列の長さ
	@param pnBgn [i/o] 入力: 切り分け開始位置, 出力: 取り出された文字列の次の位置

	@note 2003.05.25 未使用のようだ
*/
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pText, nTextLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}




/* データを指定バイト数以内に切り詰める */
int LimitStringLengthB( const char* pszData, int nDataLength, int nLimitLengthB, CMemory& cmemDes )
{
	int	i;
	int	nCharChars;
	int	nDesLen;
	nDesLen = 0;
	for( i = 0; i < nDataLength; ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pszData, nDataLength, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( nDesLen + nCharChars > nLimitLengthB ){
			break;
		}
		nDesLen += nCharChars;
		i += nCharChars;
	}
	cmemDes.SetString( pszData, nDesLen );
	return nDesLen;
}




/*!	文字列が指定された文字で終わっていなかった場合には
	末尾にその文字を付加する．

	@param pszPath [i/o]操作する文字列
	@param nMaxLen [in]バッファ長
	@param c [in]追加したい文字
	@retval  0 \が元から付いていた
	@retval  1 \を付加した
	@retval -1 バッファが足りず、\を付加できなかった
	@date 2003.06.24 Moca 新規作成
*/
int AddLastChar( TCHAR* pszPath, int nMaxLen, TCHAR c ){
	int pos = _tcslen( pszPath );
	// 何もないときは\を付加
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = _T('\0');
		return 1;
	}
	// 最後が\でないときも\を付加(日本語を考慮)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = _T('\0');
		return 1;
	}
	return 0;
}

void ResolvePath(TCHAR* pszPath)
{
	// pszPath -> pSrc
	TCHAR* pSrc = pszPath;

	// ショートカット(.lnk)の解決: pSrc -> szBuf -> pSrc
	TCHAR szBuf[_MAX_PATH];
	if( ResolveShortcutLink( NULL, pSrc, szBuf ) ){
		pSrc = szBuf;
	}

	// ロングファイル名を取得する: pSrc -> szBuf2 -> pSrc
	TCHAR szBuf2[_MAX_PATH];
	if( ::GetLongFileName( pSrc, szBuf2 ) ){
		pSrc = szBuf2;
	}

	// pSrc -> pszPath
	if(pSrc != pszPath){
		_tcscpy(pszPath, pSrc);
	}
}




/*!
	処理中のユーザー操作を可能にする
	ブロッキングフック(?)（メッセージ配送

	@date 2003.07.04 genta 一回の呼び出しで複数メッセージを処理するように
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		//	Jun. 04, 2003 genta メッセージをあるだけ処理するように
		while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
			if ( msg.message == WM_QUIT ){
				return FALSE;
			}
			if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
			}else{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		return TRUE/*ret*/;
}

/*
	scanf的安全スキャン

	使用例:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//結果: a[0]=1, a[1]=23, a[2]=4 となる。
*/
int scan_ints(
	const TCHAR*	pszData,	//!< [in]
	const TCHAR*	pszFormat,	//!< [in]
	int*			anBuf		//!< [out]
)
{
	//要素数
	int num = 0;
	const TCHAR* p = pszFormat;
	while(*p){
		if(*p==_T('%'))num++;
		p++;
	}

	//スキャン
	int dummy[32];
	memset(dummy,0,sizeof(dummy));
	int nRet = _stscanf(
		pszData,pszFormat,
		&dummy[ 0],&dummy[ 1],&dummy[ 2],&dummy[ 3],&dummy[ 4],&dummy[ 5],&dummy[ 6],&dummy[ 7],&dummy[ 8],&dummy[ 9],
		&dummy[10],&dummy[11],&dummy[12],&dummy[13],&dummy[14],&dummy[15],&dummy[16],&dummy[17],&dummy[18],&dummy[19],
		&dummy[20],&dummy[21],&dummy[22],&dummy[23],&dummy[24],&dummy[25],&dummy[26],&dummy[27],&dummy[28],&dummy[29]
	);

	//結果コピー
	int i;
	for(i=0;i<num;i++){
		anBuf[i]=dummy[i];
	}

	return nRet;
}

/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 返還後の文字列を入れるバッファ
	@param cesc  [in] エスケープしないといけない文字
	@param cwith [in] エスケープに使う文字
	
	@retval 出力したバイト数 (Unicodeの場合は文字数)

	文字列中にそのまま使うとまずい文字がある場合にその文字の前に
	エスケープキャラクタを挿入するために使う．

	@note 変換後のデータは最大で元の文字列の2倍になる
	@note この関数は2バイト文字の考慮を行っていない

	@author genta
	@date 2002/01/04 新規作成
	@date 2002/01/30 genta &専用(dupamp)から一般の文字を扱えるように拡張．
		dupampはinline関数にした．
	@date 2002/02/01 genta bugfix エスケープする文字とされる文字の出力順序が逆だった
	@date 2004/06/19 genta Generic mapping対応
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}

/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 返還後の文字列を入れるバッファ
	@param cesc  [in] エスケープしないといけない文字
	@param cwith [in] エスケープに使う文字
	
	@retval 出力したバイト数

	文字列中にそのまま使うとまずい文字がある場合にその文字の前に
	エスケープキャラクタを挿入するために使う．

	@note 変換後のデータは最大で元の文字列の2倍になる
	@note この関数は2バイト文字の考慮を行っている
	
	@note 2003.05.25 未使用のようだ
*/
int cescape_j(const char* org, char* buf, char cesc, char cwith)
{
	char *out = buf;
	for( ; *org != '\0'; ++org, ++out ){
		if( _IS_SJIS_1( (unsigned char)*org ) ){
			*out = *org;
			++out; ++org;
		}
		else if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = '\0';
	return out - buf;
}

/*
 * カラー名からインデックス番号に変換する
 */
int GetColorIndexByName( const char *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( strcmp( name, (const char*)g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * インデックス番号からカラー名に変換する
 */
const char* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}

/*! 文字数制限機能付きstrncpy

	コピー先のバッファサイズから溢れないようにstrncpyする。
	バッファが不足する場合には2バイト文字の切断もあり得る。
	末尾の\0は付与されないが、コピーはコピー先バッファサイズ-1までにしておく。

	@param dst [in] コピー先領域へのポインタ
	@param dst_count [in] コピー先領域のサイズ
	@param src [in] コピー元
	@param src_count [in] コピーする文字列の末尾

	@retval 実際にコピーされたコピー先領域の1つ後を指すポインタ

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	memcpy( dst, src, src_count );
	return dst + src_count;
}

/**	指定したウィンドウの祖先のハンドルを取得する

	GetAncestor() APIがWin95で使えないのでそのかわり

	WS_POPUPスタイルを持たないウィンドウ（ex.CDlgFuncListダイアログ）だと、
	GA_ROOTOWNERでは編集ウィンドウまで遡れないみたい。GetAncestor() APIでも同様。
	本関数固有に用意したGA_ROOTOWNER2では遡ることができる。

	@author ryoji
	@date 2007.07.01 ryoji 新規
	@date 2007.10.22 ryoji フラグ値としてGA_ROOTOWNER2（本関数固有）を追加
	@date 2008.04.09 ryoji GA_ROOTOWNER2 は可能な限り祖先を遡るように動作修正
*/
HWND MyGetAncestor( HWND hWnd, UINT gaFlags )
{
	HWND hwndAncestor;
	HWND hwndDesktop = ::GetDesktopWindow();
	HWND hwndWk;

	if( hWnd == hwndDesktop )
		return NULL;

	switch( gaFlags )
	{
	case GA_PARENT:	// 親ウィンドウを返す（オーナーは返さない）
		hwndAncestor = ( (DWORD)::GetWindowLongPtr( hWnd, GWL_STYLE ) & WS_CHILD )? ::GetParent( hWnd ): hwndDesktop;
		break;

	case GA_ROOT:	// 親子関係を遡って直近上位のトップレベルウィンドウを返す
		hwndAncestor = hWnd;
		while( (DWORD)::GetWindowLongPtr( hwndAncestor, GWL_STYLE ) & WS_CHILD )
			hwndAncestor = ::GetParent( hwndAncestor );
		break;

	case GA_ROOTOWNER:	// 親子関係と所有関係をGetParent()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
		}while( hwndWk != NULL );
		break;

	case GA_ROOTOWNER2:	// 所有関係をGetWindow()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
			if( hwndWk == NULL )
				hwndWk = ::GetWindow( hwndAncestor, GW_OWNER );
		}while( hwndWk != NULL );
		break;

	default:
		hwndAncestor = NULL;
		break;
	}

	return hwndAncestor;
}

// novice 2004/10/10 マウスサイドボタン対応
/*!
	Shift,Ctrl,Altキー状態の取得

	@retval nIdx Shift,Ctrl,Altキー状態
	@date 2004.10.10 関数化
*/
int getCtrlKeyState()
{
	int nIdx = 0;

	/* Shiftキーが押されているなら */
	if(GetKeyState_Shift()){
		nIdx |= _SHIFT;
	}
	/* Ctrlキーが押されているなら */
	if( GetKeyState_Control() ){
		nIdx |= _CTRL;
	}
	/* Altキーが押されているなら */
	if( GetKeyState_Alt() ){
		nIdx |= _ALT;
	}

	return nIdx;
}

/*[EOF]*/
