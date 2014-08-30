/*
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
#include "CEditDoc.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "my_icmp.h" // 2002/11/30 Moca 追加

/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.19 zenryaku 空要素を判定
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
	@date 2008.08.15 aroka 見出しと段落の深さ制御を追加 2008.09.07修正
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const unsigned char*	pLineBuf;	//	pLineBuf は行全体を指し、
	const unsigned char*	pLine;		//	pLine は処理中の文字以降の部分を指します。
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;
	bool					bEndTag;
	bool					bCommentTag = false;
	bool					bParaTag = false;	//	2008.08.15 aroka

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int				nMaxStack = 32;	//	ネストの最深
	int						nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char					pszStack[nMaxStack][32];
	char					szTitle[32];			//	一時領域
	char					szTag[32];				//	一時領域  小文字で保持して高速化しています。

	enum ELabelType {						//	列挙体：ラベルの種別
		LT_DEFAULT,		LT_INLINE,		LT_IGNORE,		LT_EMPTY,
		LT_BLOCK,		LT_PARAGRAPH,	LT_HEADING
	};
	enum ELabelType	nLabelType;				// default, inlined, ignore, empty, block, p, heading
	/*	同じ見出し要素（hy）を次に上位レベルの見出し(hx)が現れるまで同じ深さにそろえます。
		このため、見出しの深さを記憶しておきます。
		下位レベルの見出しの深さは現れるまで不定で、前の章節での深さは影響しません。 2008.08.15 aroka
	*/
	int						nHeadDepth[6+1];		// [0]は 空けておく
	for(k=0;k<=6;k++){
		nHeadDepth[k] = -1;
	}
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLineBuf = (const unsigned char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if( !pLineBuf )
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			pLine = &pLineBuf[i];
			// 2004.04.20 Moca コメントを処理する
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == memcmp( "-->", pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!='<' || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine=='/')
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<sizeof(szTitle)-1; )
			{
				// タグ名を切り出す
				// スペース、タブ、「_:-.英数」以外の半角文字、１文字目の「-.数字」は認めない。
				if( (pLine[j]==' ' || pLine[j]=='\t') ||
					(pLine[j]<0x80 && !strchr("_:-.",pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>='0' && pLine[j]<='9') || pLine[j]=='-' || pLine[j]=='.' )) )
				{
					break;
				}
				int nCharSize = CMemory::GetSizeOfChar((char*)pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize);
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == memcmp( "!--", pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j] = '\0';
			/*	タグの種類ごとに処理を変える必要があるが、
				都度比較するのはコストが高いので、最初に分類しておく。 2008.08.15 aroka
				比較の回数が多いため、小文字に変換しておいてstrcmpを使う。
			*/
			strcpy( szTag, szTitle );
			_strlwr( szTag );
			
			nLabelType = LT_DEFAULT;
			// 物理要素（見た目を変えるためのタグ）は構造解析しない。
			if( !strcmp(szTag,"b") || !strcmp(szTag,"big") || !strcmp(szTag,"blink")
			 || !strcmp(szTag,"font") || !strcmp(szTag,"i") || !strcmp(szTag,"marquee")
			 || !strcmp(szTag,"nobr") || !strcmp(szTag,"s") || !strcmp(szTag,"small")
			 || !strcmp(szTag,"strike") || !strcmp(szTag,"tt") || !strcmp(szTag,"u")
			 || !strcmp(szTag,"bdo") || !strcmp(szTag,"sub") || !strcmp(szTag,"sup") )
			{
				nLabelType = LT_INLINE;
			}
			// インラインテキスト要素（テキストを修飾するタグ）は構造解析しない?
//			if( !strcmp(szTag,"abbr") || !strcmp(szTag,"acronym") || !strcmp(szTag,"dfn")
//			 || !strcmp(szTag,"em") || !strcmp(szTag,"strong") || !strcmp(szTag,"span")
//			 || !strcmp(szTag,"code") || !strcmp(szTag,"samp") || !strcmp(szTag,"kbd")
//			 || !strcmp(szTag,"var") || !strcmp(szTag,"cite") || !strcmp(szTag,"q") )
//			{
//				nLabelType = LT_INLINE;
//			}
			// ルビ要素（XHTML1.1）は構造解析しない。
			if( !strcmp(szTag,"rbc") || !strcmp(szTag,"rtc") || !strcmp(szTag,"ruby")
			 || !strcmp(szTag,"rb") || !strcmp(szTag,"rt") || !strcmp(szTag,"rp") )
			{
				nLabelType = LT_INLINE;
			}
			// 空要素（内容を持たないタグ）のうち構造に関係ないものは構造解析しない。
			if( !strcmp(szTag,"br") || !strcmp(szTag,"base") || !strcmp(szTag,"basefont")
			 || !strcmp(szTag,"frame") )
			{
				nLabelType = LT_IGNORE;
			}
			// 空要素（内容を持たないタグ）のうち構造に関係するもの。
			if( !strcmp(szTag,"area") || !strcmp(szTag,"hr") || !strcmp(szTag,"img")
			 || !strcmp(szTag,"input") || !strcmp(szTag,"link") || !strcmp(szTag,"meta")
			 || !strcmp(szTag,"param") )
			{
				nLabelType = LT_EMPTY;
			}
			if( !strcmp(szTag,"div") || !strcmp(szTag,"center")
			 || !strcmp(szTag,"address") || !strcmp(szTag,"blockquote")
			 || !strcmp(szTag,"noscript") || !strcmp(szTag,"noframes")
			 || !strcmp(szTag,"ol") || !strcmp(szTag,"ul") || !strcmp(szTag,"dl")
			 || !strcmp(szTag,"dir") || !strcmp(szTag,"menu")
			 || !strcmp(szTag,"pre") || !strcmp(szTag,"table")
			 || !strcmp(szTag,"form") || !strcmp(szTag,"fieldset") || !strcmp(szTag,"isindex") )
			{
				nLabelType = LT_BLOCK;
			}
			if( !strcmp(szTag,"p") )
			{
				nLabelType = LT_PARAGRAPH;
			}
			if( (szTag[0]=='h') && ('1'<=szTitle[1]&&szTitle[1]<='6') ){
				nLabelType = LT_HEADING;
			}

			// 2009.08.08 syat 「/>」で終わるタグの判定のため、終了タグ処理を開始タグ処理の後にした。
			//                  （開始タグ処理の中で、bEndTagをtrueにしている所がある。）

			if( ! bEndTag ) // 開始タグ
			{
				if( nLabelType!=LT_INLINE && nLabelType!=LT_IGNORE ){
					// pの中でブロック要素がきたら、自動的にpを閉じる。 2008.09.07 aroka
					if( bParaTag ){
						if( nLabelType==LT_HEADING || nLabelType==LT_PARAGRAPH || nLabelType==LT_BLOCK ){
							nDepth--;
						}
					}
					if( nLabelType==LT_HEADING ){
						if( nHeadDepth[szTitle[1]-'0'] != -1 ) // 小見出し:既出
						{
							nDepth = nHeadDepth[szTitle[1]-'0'];
							for(k=szTitle[1]-'0';k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-'0'] = nDepth;
							bParaTag = false;
						}
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = true;
					}
					if( nLabelType==LT_BLOCK ){
						bParaTag = false;
					}

					int		nPosX;
					int		nPosY;

					m_cLayoutMgr.LogicToLayout(
						i,
						nLineCount,
						&nPosX,
						&nPosY
					);

					if( nLabelType!=LT_EMPTY ){
						// 終了タグなしを除く全てのタグらしきものを判定
						strcpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<sizeof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if( pLine[j]=='/' && pLine[j+1]=='>' )
								{
									bEndTag = true;
									break;
								}
								else if( pLine[j]=='>' )
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	' ';
								for(j-=k-1;i+j+k<nLineLen && k<sizeof(szTitle)-1;k++)
								{
									if( pLine[j+k]=='<' || pLine[j+k]=='\r' || pLine[j+k]=='\n' )
									{
										break;
									}
									szTitle[k] = pLine[j+k];
								}
								j += k-1;
							}
						}
						szTitle[k]	=	'\0';
						pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1, szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<sizeof(szTitle)-1;j++)
						{
							if( pLine[j]=='>' )
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	'\0';
						pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle,0,nDepth);
					}
				}
			}
			if( bEndTag ) // 終了タグ
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca 追加
				while(nDepth>0)
				{
					nDepth--;
					if(!my_stricmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if(my_stricmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}else{
					if( nLabelType==LT_HEADING ){	//	見出しの終わり
						nHeadDepth[szTitle[1]-'0'] = nDepth;
						nDepth++;
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = false;
					}
				}
			}
			i	+=	j;
		}
	}
}

/*[EOF]*/
