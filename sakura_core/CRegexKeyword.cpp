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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "CRegexKeyword.h"
#include "extmodule/CBregexp.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

#if 0
#include <stdio.h>
#define MYDBGMSG(s)                                                                                                                 \
    {                                                                                                                               \
        FILE *fp;                                                                                                                   \
        fp = fopen("debug.log", "a");                                                                                               \
        fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex); \
        fclose(fp);                                                                                                                 \
    }
#else
#define MYDBGMSG(a)
#endif

/*
 * パラメータ宣言
 */
#define RK_EMPTY 0 //初期状態
#define RK_CLOSE 1 //BREGEXPクローズ
#define RK_OPEN 2 //BREGEXPオープン
#define RK_ACTIVE 3 //コンパイル済み
#define RK_ERROR 9 //コンパイルエラー

#define RK_MATCH 4 //マッチする
#define RK_NOMATCH 5 //この行ではマッチしない

#define RK_SIZE 100 //最大登録可能数

//#define RK_HEAD_CHAR      '^'    //行先頭の正規表現
#define RK_HEAD_STR1 L"/^" //BREGEXP
#define RK_HEAD_STR2 L"m#^" //BREGEXP
#define RK_HEAD_STR3 L"m/^" //BREGEXP
//#define RK_HEAD_STR4      "#^"   //BREGEXP

#define RK_KAKOMI_1_START "/"
#define RK_KAKOMI_1_END "/k"
#define RK_KAKOMI_2_START "m#"
#define RK_KAKOMI_2_END "#k"
#define RK_KAKOMI_3_START "m/"
#define RK_KAKOMI_3_END "/k"
//#define RK_KAKOMI_4_START "#"
//#define RK_KAKOMI_4_END   "#k"

//!	コンストラクタ
/*!	@brief コンストラクタ

	BREGEXP.DLL 初期化、正規表現キーワード初期化を行う。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.08.12 genta 正規表現DLL指定のため引数追加
*/
CRegexKeyword::CRegexKeyword(LPCWSTR regexp_dll)
{
    InitDll(regexp_dll); // 2007.08.12 genta 引数追加
    MYDBGMSG("CRegexKeyword")

    m_pTypes     = NULL;
    m_nTypeIndex = -1;
    m_nTypeId    = -1;

    RegexKeyInit();
}

//!	デストラクタ
/*!	@brief デストラクタ

	コンパイル済みデータの破棄を行う。
*/
CRegexKeyword::~CRegexKeyword()
{
    int i;

    MYDBGMSG("~CRegexKeyword")
    //コンパイル済みのバッファを解放する。
    for (i = 0; i < MAX_REGEX_KEYWORD; i++)
    {
        if (m_sInfo[i].pBregexp && IsAvailable())
            BRegfree(m_sInfo[i].pBregexp);
        m_sInfo[i].pBregexp = NULL;
    }

    RegexKeyInit();

    m_nTypeIndex = -1;
    m_pTypes     = NULL;
}

//!	正規表現キーワード初期化処理
/*!	@brief 正規表現キーワード初期化

	 正規表現キーワードに関する変数類を初期化する。

	@retval TRUE 成功
*/
BOOL CRegexKeyword::RegexKeyInit(void)
{
    int i;

    MYDBGMSG("RegexKeyInit")
    m_nTypeIndex           = -1;
    m_nTypeId              = -1;
    m_nCompiledMagicNumber = 1;
    m_bUseRegexKeyword     = false;
    m_nRegexKeyCount       = 0;
    for (i = 0; i < MAX_REGEX_KEYWORD; i++)
    {
        m_sInfo[i].pBregexp = NULL;
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
BOOL CRegexKeyword::RegexKeySetTypes(const STypeConfig *pTypesPtr)
{
    MYDBGMSG("RegexKeySetTypes")
    if (pTypesPtr == NULL)
    {
        m_pTypes           = NULL;
        m_bUseRegexKeyword = false;
        return FALSE;
    }

    if (!pTypesPtr->m_bUseRegexKeyword)
    {
        //OFFになったのにまだONならOFFにする。
        if (m_bUseRegexKeyword)
        {
            m_pTypes           = NULL;
            m_bUseRegexKeyword = false;
        }
        return FALSE;
    }

    if (m_nTypeId == pTypesPtr->m_id && m_nCompiledMagicNumber == pTypesPtr->m_nRegexKeyMagicNumber && m_pTypes != NULL // 2014.07.02 条件追加
    )
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
BOOL CRegexKeyword::RegexKeyCompile(void)
{
    int i;
    static const wchar_t dummy[2] = L"\0";
    const struct RegexKeywordInfo *rp;

    MYDBGMSG("RegexKeyCompile")
    //コンパイル済みのバッファを解放する。
    for (i = 0; i < MAX_REGEX_KEYWORD; i++)
    {
        if (m_sInfo[i].pBregexp && IsAvailable())
            BRegfree(m_sInfo[i].pBregexp);
        m_sInfo[i].pBregexp = NULL;
    }

    //コンパイルパターンを内部変数に移す。
    m_nRegexKeyCount        = 0;
    const wchar_t *pKeyword = &m_pTypes->m_RegexKeywordList[0];
    for (i = 0; i < MAX_REGEX_KEYWORD; i++)
    {
        if (pKeyword[0] == L'\0')
            break;
        m_nRegexKeyCount++;
        for (; *pKeyword != '\0'; pKeyword++)
        {
        }
        pKeyword++;
    }

    m_nTypeIndex           = m_pTypes->m_nIdx;
    m_nTypeId              = m_pTypes->m_id;
    m_nCompiledMagicNumber = 1; //Not Compiled.
    m_bUseRegexKeyword     = m_pTypes->m_bUseRegexKeyword;
    if (!m_bUseRegexKeyword)
        return FALSE;

    if (!IsAvailable())
    {
        m_bUseRegexKeyword = false;
        return FALSE;
    }

    pKeyword = &m_pTypes->m_RegexKeywordList[0];
    //パターンをコンパイルする。
    for (i = 0; i < m_nRegexKeyCount; i++)
    {
        rp = &m_pTypes->m_RegexKeywordArr[i];

        if (RegexKeyCheckSyntax(pKeyword) != FALSE)
        {
            m_szMsg[0] = '\0';
            BMatch(pKeyword, dummy, dummy + 1, &m_sInfo[i].pBregexp, m_szMsg);

            if (m_szMsg[0] == '\0') //エラーがないかチェックする
            {
                //先頭以外は検索しなくてよい
                if (wcsncmp_literal(pKeyword, RK_HEAD_STR1) == 0 || wcsncmp_literal(pKeyword, RK_HEAD_STR2) == 0 || wcsncmp_literal(pKeyword, RK_HEAD_STR3) == 0)
                {
                    m_sInfo[i].nHead = 1;
                }
                else
                {
                    m_sInfo[i].nHead = 0;
                }

                if (COLORIDX_REGEX1 <= rp->m_nColorIndex && COLORIDX_REGEX10 >= rp->m_nColorIndex)
                {
                    //色指定でチェックが入ってなければ検索しなくてもよい
                    if (m_pTypes->m_ColorInfoArr[rp->m_nColorIndex].m_bDisp)
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
        for (; *pKeyword != '\0'; pKeyword++)
        {
        }
        pKeyword++;
    }

    m_nCompiledMagicNumber = m_pTypes->m_nRegexKeyMagicNumber; //Compiled.

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
BOOL CRegexKeyword::RegexKeyLineStart(void)
{
    int i;

    MYDBGMSG("RegexKeyLineStart")

    //動作に必要なチェックをする。
    if (!m_bUseRegexKeyword || !IsAvailable() || m_pTypes == NULL)
    {
        return FALSE;
    }

    //検索開始のためにオフセット情報等をクリアする。
    for (i = 0; i < m_nRegexKeyCount; i++)
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

	@retval TRUE 一致
	@retval FALSE 不一致

	@note RegexKeyLineStart関数によって初期化されていること。
*/
BOOL CRegexKeyword::RegexIsKeyword(
    const CStringRef &cStr, //!< [in] 検索対象文字列
    //	const wchar_t*		pLine,		//!< [in] １行のデータ
    int nPos, //!< [in] 検索開始オフセット
    //	int					nLineLen,	//!< [in] １行の長さ
    int *nMatchLen, //!< [out] マッチした長さ
    int *nMatchColor //!< [out] マッチした色番号
)
{
    MYDBGMSG("RegexIsKeyword")

    //動作に必要なチェックをする。
    if (!m_bUseRegexKeyword || !IsAvailable() || m_pTypes == NULL)
    {
        return FALSE;
    }

    for (int i = 0; i < m_nRegexKeyCount; i++)
    {
        const auto colorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
        auto &info            = m_sInfo[i];
        auto *pBregexp        = info.pBregexp;
        if (info.nMatch != RK_NOMATCH) /* この行にキーワードがないと分かっていない */
        {
            if (info.nOffset == nPos) /* 以前検索した結果に一致する */
            {
                *nMatchLen   = info.nLength;
                *nMatchColor = colorIndex;
                return TRUE; /* マッチした */
            }

            /* 以前の結果はもう古いので再検索する */
            if (info.nOffset < nPos)
            {
                const auto begp   = cStr.GetPtr(); //!< 行頭位置
                const auto endp   = begp + cStr.GetLength(); //!< 行末位置
                const auto startp = begp + nPos; //!< 検索開始位置
                int matched       = ExistBMatchEx()
                                  ? BMatchEx(NULL, begp, startp, endp, &pBregexp, m_szMsg)
                                  : BMatch(NULL, startp, endp, &pBregexp, m_szMsg);
                if (0 < matched && pBregexp->endp[0] - pBregexp->startp[0] > 0)
                {
                    info.nOffset = pBregexp->startp[0] - begp;
                    info.nLength = pBregexp->endp[0] - pBregexp->startp[0];
                    info.nMatch  = RK_MATCH;

                    /* 指定の開始位置でマッチした */
                    if (info.nOffset == nPos)
                    {
                        if (info.nHead != 1 || nPos == 0)
                        {
                            *nMatchLen   = info.nLength;
                            *nMatchColor = colorIndex;
                            return TRUE; /* マッチした */
                        }
                    }

                    /* 行先頭を要求する正規表現では次回から無視する */
                    if (info.nHead == 1)
                    {
                        info.nMatch = RK_NOMATCH;
                    }
                }
                else
                {
                    /* この行にこのキーワードはない */
                    info.nMatch = RK_NOMATCH;
                }
            }
        }
    } /* for */

    return FALSE;
}

BOOL CRegexKeyword::RegexKeyCheckSyntax(const wchar_t *s)
{
    const wchar_t *p;
    int length, i;
    static const wchar_t *kakomi[7 * 2] = {
        L"/",
        L"/k",
        L"m/",
        L"/k",
        L"m#",
        L"#k",
        L"/",
        L"/ki",
        L"m/",
        L"/ki",
        L"m#",
        L"#ki",
        NULL,
        NULL,
    };

    length = wcslen(s);

    for (i = 0; kakomi[i] != NULL; i += 2)
    {
        //文字長を確かめる
        if (length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i + 1]))
        {
            //始まりを確かめる
            if (wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0)
            {
                //終わりを確かめる
                p = &s[length - wcslen(kakomi[i + 1])];
                if (wcscmp(p, kakomi[i + 1]) == 0)
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
