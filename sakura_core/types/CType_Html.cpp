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
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

/* HTML */
//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
//Feb. 2, 2005 genta 苦情が多いのでシングルクォートの色分けはHTMLでは行わない
//2012.01.03 シングルクォートの色分けをする
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	_tcscpy( pType->m_szTypeName, _T("HTML") );
	_tcscpy( pType->m_szTypeExts, _T("html,htm,shtml,plg") );

	//設定
	pType->m_cBlockComments[0].SetBlockCommentRule( L"<!--", L"-->" );	/* ブロックコメントデリミタ */
	pType->m_nStringType = STRING_LITERAL_HTML;							/* 文字列区切り記号エスケープ方法 */
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
	pType->m_nKeyWordSetIdx[0] = 1;										/* キーワードセット */
	pType->m_eDefaultOutline = OUTLINE_HTML;							/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = true;				//シングルクォートの色分けOFF
}




/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.19 zenryaku 空要素を判定
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
	@date 2008.08.15 aroka 見出しと段落の深さ制御を追加 2008.09.07修正
	@date 2014.12.25 Moca XMLモード追加.タグマッチを大文字小文字同一視するように.CDATA処理追加
*/
void CDocOutline::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr, bool bXml)
{
	const wchar_t*	pLineBuf;	//	pLineBuf は行全体を指し、
	const wchar_t*	pLine;		//	pLine は処理中の文字以降の部分を指します。
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	bool			bEndTag;
	bool			bCommentTag = false;
	bool			bCDATA = false;
	bool			bParaTag = false;	//	2008.08.15 aroka

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	// 2014.12.25 ネスト32→64
	const int		nMaxStack = 64;	//	ネストの最深
	int				nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t			pszStack[nMaxStack][32];
	wchar_t			szTitle[32];			//	一時領域
	wchar_t			szTag[32];				//	一時領域  小文字で保持して高速化しています。

	enum ELabelType {						//	列挙体：ラベルの種別
		LT_DEFAULT,		LT_INLINE,		LT_IGNORE,		LT_EMPTY,
		LT_BLOCK,		LT_PARAGRAPH,	LT_HEADING
	};
	enum ELabelType	nLabelType;				// ラベルの種別
	CLogicInt		nLineCount;
	/*	同じ見出し要素（hy）を次に上位レベルの見出し(hx)が現れるまで同じ深さにそろえます。
		このため、見出しの深さを記憶しておきます。
		下位レベルの見出しの深さは現れるまで不定で、前の章節での深さは影響しません。 2008.08.15 aroka
	*/
	int				nHeadDepth[6+1];		// [0]は 空けておく
	for(k=0;k<=6;k++){
		nHeadDepth[k] = -1;
	}
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLineBuf	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
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
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2014.12.25 Moca CDATA
			if( bCDATA ){
				if( i < nLineLen - 3 && 0 == wmemcmp( L"]]>", pLine, 3 ) )
				{
					bCDATA = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!=L'<' || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine==L'/' )
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1; )
			{
				// タグ名を切り出す
				// スペース、タブ、「_:-.英数」以外の半角文字、１文字目の「-.数字」は認めない。
				if( (pLine[j]==L' ' || pLine[j]==L'\t') ||
					(pLine[j]<0x80 && !wcschr(L"_:-.",pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>=L'0' && pLine[j]<=L'9') || pLine[j]==L'-' || pLine[j]==L'.' )) )
				{
					break;
				}
				int nCharSize = CNativeW::GetSizeOfChar(pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize * sizeof(wchar_t));
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				// 2014.12.25 Moca CDATA
				if( bXml && i < nLineLen - 8 && 0 == wmemcmp( L"![CDATA[", pLine, 8 )){
					bCDATA = true;
					i += 8;
					pLine += 8;
				}
				continue;
			}
			szTitle[j] = '\0';
			/*	タグの種類ごとに処理を変える必要があるが、
				都度比較するのはコストが高いので、最初に分類しておく。 2008.08.15 aroka
				比較の回数が多いため、小文字に変換しておいてstrcmpを使う。
			*/
			wcscpy( szTag, szTitle );
			if( !bXml ){
				_wcslwr( szTag );
			}
			
			nLabelType = LT_DEFAULT;
			if( !bXml ){
				// 物理要素（見た目を変えるためのタグ）は構造解析しない。
				if( !wcscmp(szTag,L"b") || !wcscmp(szTag,L"big") || !wcscmp(szTag,L"blink")
				 || !wcscmp(szTag,L"font") || !wcscmp(szTag,L"i") || !wcscmp(szTag,L"marquee")
				 || !wcscmp(szTag,L"nobr") || !wcscmp(szTag,L"s") || !wcscmp(szTag,L"small")
				 || !wcscmp(szTag,L"strike") || !wcscmp(szTag,L"tt") || !wcscmp(szTag,L"u")
				 || !wcscmp(szTag,L"bdo") || !wcscmp(szTag,L"sub") || !wcscmp(szTag,L"sup") )
	 			{
					nLabelType = LT_INLINE;
				}
				// インラインテキスト要素（テキストを修飾するタグ）は構造解析しない?
//				if( !wcscmp(szTag,L"abbr") || !wcscmp(szTag,L"acronym") || !wcscmp(szTag,L"dfn")
//				 || !wcscmp(szTag,L"em") || !wcscmp(szTag,L"strong") || !wcscmp(szTag,L"span")
//				 || !wcscmp(szTag,L"code") || !wcscmp(szTag,L"samp") || !wcscmp(szTag,L"kbd")
//				 || !wcscmp(szTag,L"var") || !wcscmp(szTag,L"cite") || !wcscmp(szTag,L"q") )
//				{
//					nLabelType = LT_INLINE;
//				}
				// ルビ要素（XHTML1.1）は構造解析しない。
				if( !wcscmp(szTag,L"rbc") || !wcscmp(szTag,L"rtc") || !wcscmp(szTag,L"ruby")
				 || !wcscmp(szTag,L"rb") || !wcscmp(szTag,L"rt") || !wcscmp(szTag,L"rp") )
				{
					nLabelType = LT_INLINE;
				}
				// 空要素（内容を持たないタグ）のうち構造に関係ないものは構造解析しない。
				if( !wcscmp(szTag,L"br") || !wcscmp(szTag,L"base") || !wcscmp(szTag,L"basefont")
				 || !wcscmp(szTag,L"frame")
				 // 2014.12.26 Moca 以下の要素を追加
				 || !wcscmp(szTag,L"wbr")
				 )
				{
					nLabelType = LT_IGNORE;
				}
				// 空要素（内容を持たないタグ）のうち構造に関係するもの。
				if( !wcscmp(szTag,L"area") || !wcscmp(szTag,L"hr") || !wcscmp(szTag,L"img")
				 || !wcscmp(szTag,L"input") || !wcscmp(szTag,L"link") || !wcscmp(szTag,L"meta")
				 || !wcscmp(szTag,L"param")
				 // 2014.12.26 Moca 以下の要素を追加
				 || !wcscmp(szTag,L"col") || !wcscmp(szTag,L"command") || !wcscmp(szTag,L"embed")
				 || !wcscmp(szTag,L"keygen") || !wcscmp(szTag,L"source") || !wcscmp(szTag,L"track")
				 )
				{
					nLabelType = LT_EMPTY;
				}
				if( !wcscmp(szTag,L"div") || !wcscmp(szTag,L"center")
				 || !wcscmp(szTag,L"address") || !wcscmp(szTag,L"blockquote")
				 || !wcscmp(szTag,L"noscript") || !wcscmp(szTag,L"noframes")
				 || !wcscmp(szTag,L"ol") || !wcscmp(szTag,L"ul") || !wcscmp(szTag,L"dl")
				 || !wcscmp(szTag,L"dir") || !wcscmp(szTag,L"menu")
				 || !wcscmp(szTag,L"pre") || !wcscmp(szTag,L"table")
				 || !wcscmp(szTag,L"form") || !wcscmp(szTag,L"fieldset") || !wcscmp(szTag,L"isindex") )
				{
					nLabelType = LT_BLOCK;
				}
				if( !wcscmp(szTag,L"p") )
				{
					nLabelType = LT_PARAGRAPH;
				}
				if( (szTag[0]==L'h') && (L'1'<=szTitle[1]&&szTitle[1]<=L'6') ){
					nLabelType = LT_HEADING;
				}
			}

			// 2009.08.13 syat 「/>」で終わるタグの判定のため、終了タグ処理を開始タグ処理の後にした。
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
						if( nHeadDepth[szTitle[1]-L'0'] != -1 ) // 小見出し:既出
						{
							nDepth = nHeadDepth[szTitle[1]-L'0'];
							for(k=szTitle[1]-L'0';k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-L'0'] = nDepth;
							bParaTag = false;
						}
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = true;
					}
					if( nLabelType==LT_BLOCK ){
						bParaTag = false;
					}

					CLayoutPoint ptPos;

					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if( nLabelType!=LT_EMPTY ){
						// 終了タグなしを除く全てのタグらしきものを判定
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						int x = j;
						// 2014.12.25 32文字以上のとき,別の行のときにも「/>」bEndTagに対応
						{
							bool bLoop = true;
							while (bLoop && pLine) {
								for (; j < nLineLen - i; j++) {
									if (pLine[j] == L'/' && pLine[j+1]== L'>') {
										bEndTag = true; // <emptytag />
										bLoop = false;
										j+=2;
										break;
									}else if ( pLine[j] == L'>' ) {
										bLoop = false;
										j++;
										break;
									}
								}
								if (bLoop) {
									nLineCount++;
									pLineBuf = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
									pLine = pLineBuf;
									j = 0;
									i = 0;
								}
							}
						}
						if (x < _countof(szTitle) - 3) {
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
								for (; i+j < nLineLen && k < _countof(szTitle) -1 ; k++, j++) {
									if (pLine[j]==L'<' || WCODE::IsLineDelimiter(pLine[j], bExtEol)){
										j--;
										break;
									}
									szTitle[k] = pLine[j];
								}
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptPos.GetY2()+CLayoutInt(1), szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[j]==L'>')
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			if( bEndTag ) // 終了タグ
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca 追加
				while(nDepth>0)
				{
					nDepth--;
					if (bXml) {
						if(!wcscmp(pszStack[nDepth],szTitle)){
							break;
						}
					}else{
						if(!wcsicmp(pszStack[nDepth],szTitle)){
							break;
						}
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if (bXml) {
						if(wcscmp(pszStack[nDepth],szTitle)){
							nDepth = nDepthOrg;
						}
					}else{
						if(wcsicmp(pszStack[nDepth],szTitle)){
							nDepth = nDepthOrg;
						}
					}
				}else{
					if( nLabelType==LT_HEADING ){	//	見出しの終わり
						nHeadDepth[szTitle[1]-L'0'] = nDepth;
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



const wchar_t* g_ppszKeywordsHTML[] = {
	L"_blank",
	L"_parent",
	L"_self",
	L"_top",
	L"A",
	L"ABBR",
	L"ABOVE",
	L"absbottom",
	L"absmiddle",
	L"ACCESSKEY",
	L"ACRONYM",
	L"ACTION",
	L"ADDRESS",
	L"ALIGN",
	L"all",
	L"APPLET",
	L"AREA",
	L"AUTOPLAY",
	L"AUTOSTART",
	L"B",
	L"BACKGROUND",
	L"BASE",
	L"BASEFONT",
	L"baseline",
	L"BEHAVIOR",
	L"BELOW",
	L"BGCOLOR",
	L"BGSOUND",
	L"BIG",
	L"BLINK",
	L"BLOCKQUOTE",
	L"BODY",
	L"BORDER",
	L"BORDERCOLOR",
	L"BORDERCOLORDARK",
	L"BORDERCOLORLIGHT",
	L"BOTTOM",
	L"box",
	L"BR",
	L"BUTTON",
	L"CAPTION",
	L"CELLPADDING",
	L"CELLSPACING",
	L"CENTER",
	L"CHALLENGE",
	L"char",
	L"checkbox",
	L"CHECKED",
	L"CITE",
	L"CLEAR",
	L"CLIP",
	L"CODE",
	L"CODEBASE",
	L"CODETYPE",
	L"COL",
	L"COLGROUP",
	L"COLOR",
	L"COLS",
	L"COLSPAN",
	L"COMMENT",
	L"CONTROLS",
	L"DATA",
	L"DD",
	L"DECLARE",
	L"DEFER",
	L"DEL",
	L"DELAY",
	L"DFN",
	L"DIR",
	L"DIRECTION",
	L"DISABLED",
	L"DIV",
	L"DL",
	L"DOCTYPE",
	L"DT",
	L"EM",
	L"EMBED",
	L"ENCTYPE",
	L"FACE",
	L"FIELDSET",
	L"file",
	L"FONT",
	L"FOR",
	L"FORM",
	L"FRAME",
	L"FRAMEBORDER",
	L"FRAMESET",
	L"GET",
	L"groups",
	L"GROUPS",
	L"GUTTER",
	L"H1",
	L"H2",
	L"H3",
	L"H4",
	L"H5",
	L"H6",
	L"H7",
	L"HEAD",
	L"HEIGHT",
	L"HIDDEN",
	L"Hn",
	L"HR",
	L"HREF",
	L"hsides",
	L"HSPACE",
	L"HTML",
	L"I",
	L"ID",
	L"IFRAME",
	L"ILAYER",
	L"image",
	L"IMG",
	L"INDEX",
	L"inherit",
	L"INPUT",
	L"INS",
	L"ISINDEX",
	L"JavaScript",
	L"justify",
	L"KBD",
	L"KEYGEN",
	L"LABEL",
	L"LANGUAGE",
	L"LAYER",
	L"LEFT",
	L"LEGEND",
	L"lhs",
	L"LI",
	L"LINK",
	L"LISTING",
	L"LOOP",
	L"MAP",
	L"MARQUEE",
	L"MAXLENGTH",
	L"MENU",
	L"META",
	L"METHOD",
	L"METHODS",
	L"MIDDLE",
	L"MULTICOL",
	L"MULTIPLE",
	L"NAME",
	L"NEXT",
	L"NEXTID",
	L"NOBR",
	L"NOEMBED",
	L"NOFRAMES",
	L"NOLAYER",
	L"none",
	L"NOSAVE",
	L"NOSCRIPT",
	L"NOTAB",
	L"NOWRAP",
	L"OBJECT",
	L"OL",
	L"onBlur",
	L"onChange",
	L"onClick",
	L"onFocus",
	L"onLoad",
	L"onMouseOut",
	L"onMouseOver",
	L"onReset",
	L"onSelect",
	L"onSubmit",
	L"OPTION",
	L"P",
	L"PAGEX",
	L"PAGEY",
	L"PALETTE",
	L"PANEL",
	L"PARAM",
	L"PARENT",
	L"password",
	L"PLAINTEXT",
	L"PLUGINSPAGE",
	L"POST",
	L"PRE",
	L"PREVIOUS",
	L"Q",
	L"radio",
	L"REL",
	L"REPEAT",
	L"reset",
	L"REV",
	L"rhs",
	L"RIGHT",
	L"rows",
	L"ROWSPAN",
	L"RULES",
	L"S",
	L"SAMP",
	L"SAVE",
	L"SCRIPT",
	L"SCROLLAMOUNT",
	L"SCROLLDELAY",
	L"SELECT",
	L"SELECTED",
	L"SERVER",
	L"SHAPES",
	L"show",
	L"SIZE",
	L"SMALL",
	L"SONG",
	L"SPACER",
	L"SPAN",
	L"SRC",
	L"STANDBY",
	L"STRIKE",
	L"STRONG",
	L"STYLE",
	L"SUB",
	L"submit",
	L"SUMMARY",
	L"SUP",
	L"TABINDEX",
	L"TABLE",
	L"TARGET",
	L"TBODY",
	L"TD",
	L"TEXT",
	L"TEXTAREA",
	L"textbottom",
	L"TEXTFOCUS",
	L"textmiddle",
	L"texttop",
	L"TFOOT",
	L"TH",
	L"THEAD",
	L"TITLE",
	L"TOP",
	L"TR",
	L"TT",
	L"TXTCOLOR",
	L"TYPE",
	L"U",
	L"UL",
	L"URN",
	L"USEMAP",
	L"VALIGN",
	L"VALUE",
	L"VALUETYPE",
	L"VAR",
	L"VISIBILITY",
	L"void",
	L"vsides",
	L"VSPACE",
	L"WBR",
	L"WIDTH",
	L"WRAP",
	L"XMP"
};
int g_nKeywordsHTML = _countof(g_ppszKeywordsHTML);
