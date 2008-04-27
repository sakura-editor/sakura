#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* HTML */
//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
//Feb. 2, 2005 genta 苦情が多いのでシングルクォートの色分けはHTMLでは行わない
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	_tcscpy( pType->m_szTypeName, _T("HTML") );
	_tcscpy( pType->m_szTypeExts, _T("html,htm,shtml,plg") );

	//設定
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"<!--", L"-->" );	/* ブロックコメントデリミタ */
	pType->m_nStringType = 0;											/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pType->m_nKeyWordSetIdx[0] = 1;										/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;			//シングルクォートの色分けOFF
}




/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.19 zenryaku 空要素を判定
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
*/
void CDocOutline::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	BOOL			bEndTag;
	BOOL			bCommentTag = FALSE;

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt			nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca コメントを処理する
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!=L'<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]==L'/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1;j++)
			{
				if((pLine[i+j]<L'a' || pLine[i+j]>L'z') &&
					(pLine[i+j]<L'A' || pLine[i+j]>L'Z') &&
					!(j!=0 && pLine[i+j]>=L'0' && pLine[i+j]<=L'9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine + i, 3 ) )
				{
					bCommentTag = TRUE;
					i += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j]	=	'\0';
			if(bEndTag)
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca 追加
				// 終了タグ
				while(nDepth>0)
				{
					nDepth--;
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(wcsicmp(szTitle,L"br") && wcsicmp(szTitle,L"area") &&
					wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
				{
					CLayoutPoint ptPos;

					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if(wcsicmp(szTitle,L"hr") && wcsicmp(szTitle,L"meta") && wcsicmp(szTitle,L"link") &&
						wcsicmp(szTitle,L"input") && wcsicmp(szTitle,L"img") && wcsicmp(szTitle,L"area") &&
						wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
					{
						// 終了タグなしを除く全てのタグらしきものを判定
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]==L'/' && pLine[i+j+1]==L'>')
								{
									bEndTag	=	TRUE;
									break;
								}
								else if(pLine[i+j]==L'>')
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if(pLine[i+j+k]==L'<' || pLine[i+j+k]==L'\r' || pLine[i+j+k]==L'\n')
									{
										break;
									}
									szTitle[k]	=	pLine[i+j+k];
								}
							j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,(bEndTag ? nDepth : nDepth++));
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}
