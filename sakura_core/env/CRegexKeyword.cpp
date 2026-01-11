/*!	@file
	@brief CRegexKeyword Library

	正規表現キーワードを扱う。
	BREGEXP.DLLを利用する。

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "env/CRegexKeyword.h"
#include "extmodule/CBregexp.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"
#include "mem/CNativeW.h"

#if 0
#define	MYDBGMSG(s) \
{\
	FILE	*fp;\
	fp = fopen("debug.log", "a");\
	fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex);\
	fclose(fp);\
}
#else
#define	MYDBGMSG(a)
#endif

constexpr auto& RK_HEAD_STR1 = L"/^";
constexpr auto& RK_HEAD_STR2 = L"m#^";
constexpr auto& RK_HEAD_STR3 = L"m/^";

//!	コンストラクタ
/*!	@brief コンストラクタ

	BREGEXP.DLL 初期化、正規表現キーワード初期化を行う。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.08.12 genta 正規表現DLL指定のため引数追加
*/
CRegexKeyword::CRegexKeyword(const std::filesystem::path& bregonigPath)
{
	InitDll(bregonigPath.c_str());	// 2007.08.12 genta 引数追加
	MYDBGMSG("CRegexKeyword")

	RegexKeyInit();
}

//!	デストラクタ
/*!	@brief デストラクタ

	コンパイル済みデータの破棄を行う。
*/
CRegexKeyword::~CRegexKeyword()
{
	MYDBGMSG("~CRegexKeyword")

	RegexKeyInit();

	m_nTypeIndex = -1;
	m_pTypes     = nullptr;
}

//!	正規表現キーワード初期化処理
/*!	@brief 正規表現キーワード初期化

	 正規表現キーワードに関する変数類を初期化する。

	@retval TRUE 成功
*/
BOOL CRegexKeyword::RegexKeyInit( void )
{
	MYDBGMSG("RegexKeyInit")
	m_nTypeIndex = -1;
	m_nTypeId = -1;
	m_nCompiledMagicNumber = 1;
	m_bUseRegexKeyword = false;
	m_nRegexKeyCount = 0;

	for (int i = 0; i < MAX_REGEX_KEYWORD; ++i) {
		m_sInfo[i].pPattern = nullptr;
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
BOOL CRegexKeyword::RegexKeySetTypes( const STypeConfig *pTypesPtr )
{
	MYDBGMSG("RegexKeySetTypes")
	if( pTypesPtr == nullptr ) 
	{
		m_pTypes = nullptr;
		m_bUseRegexKeyword = false;
		return FALSE;
	}

	if( !pTypesPtr->m_bUseRegexKeyword )
	{
		//OFFになったのにまだONならOFFにする。
		if( m_bUseRegexKeyword )
		{
			m_pTypes = nullptr;
			m_bUseRegexKeyword = false;
		}
		return FALSE;
	}

	if( m_nTypeId              == pTypesPtr->m_id
	 && m_nCompiledMagicNumber == pTypesPtr->m_nRegexKeyMagicNumber
	 && m_pTypes != nullptr  // 2014.07.02 条件追加
	){
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
	using enum ERkMStatus;

	int	i;
	const struct RegexKeywordInfo	*rp;

	MYDBGMSG("RegexKeyCompile")
	//コンパイル済みのバッファを解放する。
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		m_sInfo[i].pPattern = nullptr;
	}

	//コンパイルパターンを内部変数に移す。
	m_nRegexKeyCount = 0;
	const wchar_t * pKeyword = &m_pTypes->m_RegexKeywordList[0];
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( pKeyword[0] == L'\0' ) break;
		m_nRegexKeyCount++;
		for(; *pKeyword != '\0'; pKeyword++ ){}
		pKeyword++;
	}

	m_nTypeIndex = m_pTypes->m_nIdx;
	m_nTypeId = m_pTypes->m_id;
	m_nCompiledMagicNumber = 1;	//Not Compiled.
	m_bUseRegexKeyword  = m_pTypes->m_bUseRegexKeyword;
	if( !m_bUseRegexKeyword ) return FALSE;

	if( ! IsAvailable() )
	{
		m_bUseRegexKeyword = false;
		return FALSE;
	}

	pKeyword = &m_pTypes->m_RegexKeywordList[0];
	//パターンをコンパイルする。
	for(i = 0; i < m_nRegexKeyCount; i++)
	{
		rp = &m_pTypes->m_RegexKeywordArr[i];

		if( RegexKeyCheckSyntax( pKeyword ) != FALSE )
		{
			Compile(pKeyword);
			m_sInfo[i].pPattern = GetPattern();

			if (m_sInfo[i].pPattern)	//エラーがないかチェックする
			{
				//先頭以外は検索しなくてよい
				if( wcsncmp_literal( pKeyword, RK_HEAD_STR1 ) == 0
				 || wcsncmp_literal( pKeyword, RK_HEAD_STR2 ) == 0
				 || wcsncmp_literal( pKeyword, RK_HEAD_STR3 ) == 0
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
		for(; *pKeyword != '\0'; pKeyword++ ){}
		pKeyword++;
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
	MYDBGMSG("RegexKeyLineStart")

	//動作に必要なチェックをする。
	if( !m_bUseRegexKeyword || !IsAvailable() || m_pTypes==nullptr )
	{
		return FALSE;
	}

	//検索開始のためにオフセット情報等をクリアする。
	for(int i = 0; i < m_nRegexKeyCount; ++i)
	{
		m_sInfo[i].nOffset = -1;
		m_sInfo[i].nMatch  = m_sInfo[i].nFlag;
		m_sInfo[i].nStatus = ERkStatus::RK_EMPTY;
	}

	return TRUE;
}

//!	正規表現検索処理
/*!	@brief 正規表現検索

	正規表現キーワードを検索する。

	@retval TRUE 一致
	@retval FALSE 不一致

	@note RegexKeyLineStart関数によって初期化されていること。
*/
BOOL CRegexKeyword::RegexIsKeyword(
	std::wstring_view	text,		//!< [in] 検索対象文字列
	int					nPos,		//!< [in] 検索開始オフセット
	int*				nMatchLen,	//!< [out] マッチした長さ
	int*				nMatchColor	//!< [out] マッチした色番号
)
{
	using enum ERkMStatus;

	MYDBGMSG("RegexIsKeyword")

	//動作に必要なチェックをする。
	if( !m_bUseRegexKeyword
		|| !IsAvailable()
		|| m_pTypes == nullptr )
	{
		return FALSE;
	}

	for( int i = 0; i < m_nRegexKeyCount; i++ )
	{
		const auto colorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
		auto &info = m_sInfo[i];
		if (info.nMatch == RK_NOMATCH) { // この行にキーワードはない
			continue;
		}

		if (info.nOffset == nPos)  /* 以前検索した結果に一致する */
		{
			*nMatchLen   = info.nLength;
			*nMatchColor = colorIndex;
			return TRUE;  /* マッチした */
		}

		assert(info.nOffset < nPos);

		const auto &pPattern = info.pPattern;
		if (!pPattern) {
			continue;
		}

		/* 以前の結果はもう古いので再検索する */
		if (!pPattern->Match(text, nPos) && !pPattern->matched().empty())
		{
			info.nMatch = RK_NOMATCH;	//この行にこのキーワードはない
			break;
		}

		info.nOffset = int(pPattern->starti());
		info.nLength = int(pPattern->matched().length());
		info.nMatch  = RK_MATCH;
				
		/* 指定の開始位置でマッチした */
		if (info.nOffset == nPos && (info.nHead != 1 || nPos == 0))
		{
			*nMatchLen   = info.nLength;
			*nMatchColor = colorIndex;
			return TRUE;  /* マッチした */
		}

		/* 行先頭を要求する正規表現では次回から無視する */
		if (info.nHead == 1)
		{
			info.nMatch = RK_NOMATCH;
		}
	}  /* for */

	return FALSE;
}

BOOL CRegexKeyword::RegexKeyCheckSyntax(const wchar_t *s)
{
	const wchar_t	*p;
	int	length, i;
	static const wchar_t *kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		nullptr, nullptr,
	};

	length = (int)wcslen(s);

	for(i = 0; kakomi[i] != nullptr; i += 2)
	{
		//文字長を確かめる
		if( length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i+1]) )
		{
			//始まりを確かめる
			if( wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0 )
			{
				//終わりを確かめる
				p = &s[length - wcslen(kakomi[i+1])];
				if( wcscmp(p, kakomi[i+1]) == 0 )
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

/*static*/
DWORD CRegexKeyword::GetNewMagicNumber()
{
	return ::GetTickCount();
}
