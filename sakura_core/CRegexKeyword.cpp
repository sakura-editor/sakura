//	$Id$
/*!	@file
	CRegexKeyword Library

	正規表現キーワードを扱う。
	BREGEXP.DLLを利用する。

	@author MIK
	@date Nov. 17, 2001
	$Revision$
*/
/*
	Copyright (C) 2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "global.h"
#include "CRegexKeyword.h"
#include "CBregexp.h"

#if 0
#include <stdio.h>
#define	MYDBGMSG(s) \
{\
	FILE	*fp;\
	fp = fopen("debug.log", "a");\
	fprintf(fp, "%08x: %s  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex);\
	fclose(fp);\
}
#else
#define	MYDBGMSG(a)
#endif

//!	コンストラクタ
/*!	@brief コンストラクタ

	BREGEXP.DLL 初期化、正規表現キーワード初期化を行う。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CRegexKeyword::CRegexKeyword()
{
	Init();
	MYDBGMSG("CRegexKeyword")

	m_pTypes    = NULL;
	m_nTypeIndex = -1;

	RegexKeyInit();
}

//!	デストラクタ
/*!	@brief デストラクタ

	コンパイル済みデータの破棄を行う。
*/
CRegexKeyword::~CRegexKeyword()
{
	int	i;

	MYDBGMSG("~CRegexKeyword")
	//コンパイル済みのバッファを解放する。
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_sInfo[i].pBregexp && IsAvailable() )
			BRegfree(m_sInfo[i].pBregexp);
		m_sInfo[i].pBregexp = NULL;
	}

	RegexKeyInit();

	m_nTypeIndex = -1;
	m_pTypes     = NULL;
//	m_pShareData = NULL;
}

//!	正規表現キーワード初期化処理
/*!	@brief 正規表現キーワード初期化

	 正規表現キーワードに関する変数類を初期化する。

	@retval TRUE 成功
*/
BOOL CRegexKeyword::RegexKeyInit( void )
{
	int	i;

	MYDBGMSG("RegexKeyInit")
	m_nTypeIndex = -1;
	m_nCompiledMagicNumber = 0;
	m_bUseRegexKeyword = FALSE;
	m_nRegexKeyCount = 0;
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		m_sInfo[i].pBregexp = NULL;
#ifdef USE_PARENT
#else
		m_sInfo[i].sRegexKey.m_szKeyword[0] = '\0';
		m_sInfo[i].sRegexKey.m_nColorIndex = COLORIDX_REGEX1;
#endif
	}

	return TRUE;
}

//!	現在タイプ設定処理
/*!	@brief 現在タイプ設定

	現在のタイプ設定を設定する。

	@param pTypesPtr [in] タイプ設定構造体へのポインタ

	@retval TRUE 成功
	@retval FALSE 失敗

	@note タイプ設定が変わったら再ロードしコンパイルする。
*/
BOOL CRegexKeyword::RegexKeySetTypes( Types *pTypesPtr )
{
	MYDBGMSG("RegexKeySetTypes")
	if( pTypesPtr == NULL ) 
	{
		m_pTypes = NULL;
		m_bUseRegexKeyword = FALSE;
		return FALSE;
	}

	if( pTypesPtr->m_bUseRegexKeyword == FALSE )
	{
		//OFFになったのにまだONならOFFにする。
		if( m_bUseRegexKeyword )
		{
			m_pTypes = NULL;
			m_bUseRegexKeyword = FALSE;
		}
		return FALSE;
	}

	if( m_pTypes               == pTypesPtr
	 && m_nCompiledMagicNumber == pTypesPtr->m_nRegexKeyMagicNumber
	/* && m_bUseRegexKeyword     == pTypesPtr->m_bUseRegexKeyword */ )
	{
		return TRUE;
	}

	m_pTypes = pTypesPtr;

	RegexKeyCompile();
	
	return TRUE;
}

//!	正規表現キーワードコンパイル処理
/*!	@brief 正規表現キーワードコンパイル

	正規表現キーワードをコンパイルする。

	@retval TRUE 成功
	@retval FALSE 失敗

	@note すでにコンパイル済みの場合はそれを破棄する。
	キーワードはコンパイルデータとして内部変数にコピーする。
	先頭指定、色指定側の使用・未使用をチェックする。
*/
BOOL CRegexKeyword::RegexKeyCompile( void )
{
	int	i;
	int	matched;
	static const char dummy[2] = "\0";
	struct RegexKeywordInfo	*rp;

	MYDBGMSG("RegexKeyCompile")
	//コンパイル済みのバッファを解放する。
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_sInfo[i].pBregexp && IsAvailable() )
			BRegfree(m_sInfo[i].pBregexp);
		m_sInfo[i].pBregexp = NULL;
	}

	//コンパイルパターンを内部変数に移す。
	m_nRegexKeyCount = 0;
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_pTypes->m_RegexKeywordArr[i].m_szKeyword[0] == '\0' ) break;
#ifdef USE_PARENT
#else
		strcpy(m_sInfo[i].sRegexKey.m_szKeyword, m_pTypes->m_RegexKeywordArr[i].m_szKeyword);
		m_sInfo[i].sRegexKey.m_nColorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#endif
		m_nRegexKeyCount++;
	}

	m_nTypeIndex = m_pTypes->m_nIdx;
	m_nCompiledMagicNumber = m_pTypes->m_nRegexKeyMagicNumber - 1;	//Not Compiled.
	m_bUseRegexKeyword  = m_pTypes->m_bUseRegexKeyword;
	if( m_bUseRegexKeyword == 0 ) return FALSE;

	if( ! IsAvailable() )
	{
		m_bUseRegexKeyword = 0;
		return FALSE;
	}

	//パターンをコンパイルする。
	for(i = 0; i < m_nRegexKeyCount; i++)
	{
#ifdef USE_PARENT
		rp = &m_pTypes->m_RegexKeywordArr[i];
#else
		rp = &m_sInfo[i].sRegexKey;
#endif

		if( RegexKeyCheckSyntax( rp->m_szKeyword ) == TRUE )
		{
			m_szMsg[0] = '\0';
			matched = BMatch(rp->m_szKeyword, dummy, dummy+1, &m_sInfo[i].pBregexp, m_szMsg);

			if( m_szMsg[0] == '\0' )	//エラーがないかチェックする
			{
				//先頭以外は検索しなくてよい
				if( strncmp( RK_HEAD_STR1, rp->m_szKeyword, RK_HEAD_STR1_LEN ) == 0
				 || strncmp( RK_HEAD_STR2, rp->m_szKeyword, RK_HEAD_STR2_LEN ) == 0
				 || strncmp( RK_HEAD_STR3, rp->m_szKeyword, RK_HEAD_STR3_LEN ) == 0
				/* || strncmp( RK_HEAD_STR4, rp->m_szKeyword, RK_HEAD_STR4_LEN ) == 0 */
				)
				{
					m_sInfo[i].nHead = 1;
				}
				else
				{
					m_sInfo[i].nHead = 0;
				}

				if( COLORIDX_REGEX1  <= rp->m_nColorIndex
				 && COLORIDX_REGEX10 >= rp->m_nColorIndex )
				{
					//色指定でチェックが入ってなければ検索しなくてもよい
					if( m_pTypes->m_ColorInfoArr[rp->m_nColorIndex].m_bDisp )
					{
						m_sInfo[i].nFlag = RK_EMPTY;
					}
					else
					{
						//正規表現では色指定のチェックを見る。
						m_sInfo[i].nFlag = RK_NOMATCH;
					}
				}
				else
				{
					//正規表現以外では、色指定チェックは見ない。
					//例えば、半角数値は正規表現を使い、基本機能を使わないという指定もあり得るため
					m_sInfo[i].nFlag = RK_EMPTY;
				}
			}
			else
			{
				//コンパイルエラーなので検索対象からはずす
				m_sInfo[i].nFlag = RK_NOMATCH;
			}
		}
		else
		{
			//書式エラーなので検索対象からはずす
			m_sInfo[i].nFlag = RK_NOMATCH;
		}

	}

	m_nCompiledMagicNumber = m_pTypes->m_nRegexKeyMagicNumber;	//Compiled.

	return TRUE;
}

//!	行検索開始処理
/*!	@brief 行検索開始

	行検索を開始する。

	@retval TRUE 成功
	@retval FALSE 失敗または検索しない指定あり

	@note それぞれの行検索の最初に実行する。
	タイプ設定等が変更されている場合はリロードする。
*/
BOOL CRegexKeyword::RegexKeyLineStart( void )
{
	int	i;

	MYDBGMSG("RegexKeyLineStart")

	//動作に必要なチェックをする。
	if( ( m_bUseRegexKeyword == FALSE )
	 || ( ! IsAvailable() )
	 || ( m_pTypes == NULL ) )
	{
		return FALSE;
	}

#if 0	//RegexKeySetTypesで設定されているはずなので廃止
	//情報不一致ならマスタから取得してコンパイルする。
	if( m_nCompiledMagicNumber != m_pTypes->m_nRegexKeyMagicNumber
	 || m_nTypeIndex           != m_pTypes->m_nIdx )
	{
		RegexKeyCompile();
	}
#endif

	//検索開始のためにオフセット情報等をクリアする。
	for(i = 0; i < m_nRegexKeyCount; i++)
	{
		m_sInfo[i].nOffset = -1;
		//m_sInfo[i].nMatch  = RK_EMPTY;
		m_sInfo[i].nMatch  = m_sInfo[i].nFlag;
		m_sInfo[i].nStatus = RK_EMPTY;
	}

	return TRUE;
}

//!	正規表現検索処理
/*!	@brief 正規表現検索

	正規表現キーワードを検索する。

	@param pLine [in] １行のデータ
	@param nPos [in] 検索開始オフセット
	@param nLineLen [in] １行の長さ
	@param nMatchLen [out] マッチした長さ
	@param nMatchColor [out] マッチした色番号

	@retval TRUE 一致
	@retval FALSE 不一致

	@note RegexKeyLineStart関数によって初期化されていること。
*/
BOOL CRegexKeyword::RegexIsKeyword( const char *pLine, int nPos, int nLineLen, int *nMatchLen, int *nMatchColor )
{
	int	i, matched;

	MYDBGMSG("RegexIsKeyword")

	//動作に必要なチェックをする。
	if( ( m_bUseRegexKeyword == FALSE )
	 || ( ! IsAvailable() )
#ifdef USE_PARENT
	 || ( m_pTypes == NULL )
#endif
	 /* || ( pLine == NULL ) */ )
	{
		return FALSE;
	}

	for(i = 0; i < m_nRegexKeyCount; i++)
	{
		if( m_sInfo[i].nMatch != RK_NOMATCH )  /* この行にキーワードがないと分かっていない */
		{
			if( m_sInfo[i].nOffset == nPos )  /* 以前検索した結果に一致する */
			{
				*nMatchLen   = m_sInfo[i].nLength;
#ifdef USE_PARENT
				*nMatchColor = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#else
				*nMatchColor = m_sInfo[i].sRegexKey.m_nColorIndex;
#endif
				return TRUE;  /* マッチした */
			}

			/* 以前の結果はもう古いので再検索する */
			if( m_sInfo[i].nOffset < nPos )
			{
#ifdef USE_PARENT
				matched = BMatch(m_pTypes->m_RegexKeywordArr[i].m_szKeyword, pLine+nPos, pLine+nLineLen,
					&m_sInfo[i].pBregexp, m_szMsg);
#else
				matched = BMatch(m_sInfo[i].sRegexKey.m_szKeyword, pLine+nPos, pLine+nLineLen,
					&m_sInfo[i].pBregexp, m_szMsg);
#endif
				if( matched )
				{
					m_sInfo[i].nOffset = m_sInfo[i].pBregexp->startp[0] - pLine;
					m_sInfo[i].nLength = m_sInfo[i].pBregexp->endp[0] - m_sInfo[i].pBregexp->startp[0];
					m_sInfo[i].nMatch  = RK_MATCH;
				
					/* 指定の開始位置でマッチした */
					if( m_sInfo[i].nOffset == nPos )
					{
						if( m_sInfo[i].nHead != 1 || nPos == 0 )
						{
							*nMatchLen   = m_sInfo[i].nLength;
#ifdef USE_PARENT
							*nMatchColor = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#else
							*nMatchColor = m_sInfo[i].sRegexKey.m_nColorIndex;
#endif
							return TRUE;  /* マッチした */
						}
					}

					/* 行先頭を要求する正規表現では次回から無視する */
					if( m_sInfo[i].nHead == 1 )
					{
						m_sInfo[i].nMatch = RK_NOMATCH;
					}
				}
				else
				{
					/* この行にこのキーワードはない */
					m_sInfo[i].nMatch = RK_NOMATCH;
				}
			}
		}
	}  /* for */

	return FALSE;
}

BOOL CRegexKeyword::RegexKeyCheckSyntax(const char *s)
{
	const char	*p;
	int	length, i;
	static const char *kakomi[7 * 2] = {
		"/",  "/k",
		"m/", "/k",
		"m#", "#k",
		"/",  "/ki",
		"m/", "/ki",
		"m#", "#ki",
		NULL, NULL,
	};

	length = strlen(s);

	for(i = 0; kakomi[i] != NULL; i += 2)
	{
		//文字長を確かめる
		if( length > (int)strlen(kakomi[i]) + (int)strlen(kakomi[i+1]) )
		{
			//始まりを確かめる
			if( strncmp(kakomi[i], s, strlen(kakomi[i])) == 0 )
			{
				//終わりを確かめる
				p = &s[length - strlen(kakomi[i+1])];
				if( strcmp(p, kakomi[i+1]) == 0 )
				{
					//正常
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//@@@ 2001.11.17 add end MIK

/*[EOF]*/
