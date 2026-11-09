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

/* HTML */
//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
//Feb. 2, 2005 genta 苦情が多いのでシングルクォートの色分けはHTMLでは行わない
//2012.01.03 シングルクォートの色分けをする
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	wcscpy( pType->m_szTypeName, LTEXT("HTML") );
	wcscpy( pType->m_szTypeExts, LTEXT("html,htm,shtml,plg") );

	//設定
	pType->m_cBlockComments[0].SetBlockCommentRule( LTEXT("<!--"), LTEXT("-->") );	/* ブロックコメントデリミタ */
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
	const WCHAR*	pLineBuf;	//	pLineBuf は行全体を指し、
	const WCHAR*	pLine;		//	pLine は処理中の文字以降の部分を指します。
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	bool			bEndTag;
	bool			bCommentTag = false;
	bool			bCDATA = false;
	bool			bParaTag = false;	//	2008.08.15 aroka

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダーは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	// 2014.12.25 ネスト32→64
	const int		nMaxStack = 64;	//	ネストの最深
	int				nDepth = 0;				//	いまのアイテムの深さを表す数値。
	WCHAR			pszStack[nMaxStack][32];
	WCHAR			szTitle[32];			//	一時領域
	WCHAR			szTag[32];				//	一時領域  小文字で保持して高速化しています。

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
				if( i < nLineLen - 3 && 0 == wmemcmp( LTEXT("-->"), pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2014.12.25 Moca CDATA
			if( bCDATA ){
				if( i < nLineLen - 3 && 0 == wmemcmp( LTEXT("]]>"), pLine, 3 ) )
				{
					bCDATA = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!=LCHAR('<') || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine==LCHAR('/') )
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<int(std::size(szTitle))-1; )
			{
				// タグ名を切り出す
				// スペース、タブ、「_:-.英数」以外の半角文字、１文字目の「-.数字」は認めない。
				if( (pLine[j]==LCHAR(' ') || pLine[j]==LCHAR('\t')) ||
					(pLine[j]<0x80 && !wcschr(LTEXT("_:-."),pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>=LCHAR('0') && pLine[j]<=LCHAR('9')) || pLine[j]==LCHAR('-') || pLine[j]==LCHAR('.') )) )
				{
					break;
				}
				int nCharSize = CNativeW::GetSizeOfChar(pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize * sizeof(WCHAR));
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == wmemcmp( LTEXT("!--"), pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				// 2014.12.25 Moca CDATA
				if( bXml && i < nLineLen - 8 && 0 == wmemcmp( LTEXT("![CDATA["), pLine, 8 )){
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
				if( !wcscmp(szTag,LTEXT("b")) || !wcscmp(szTag,LTEXT("big")) || !wcscmp(szTag,LTEXT("blink"))
				 || !wcscmp(szTag,LTEXT("font")) || !wcscmp(szTag,LTEXT("i")) || !wcscmp(szTag,LTEXT("marquee"))
				 || !wcscmp(szTag,LTEXT("nobr")) || !wcscmp(szTag,LTEXT("s")) || !wcscmp(szTag,LTEXT("small"))
				 || !wcscmp(szTag,LTEXT("strike")) || !wcscmp(szTag,LTEXT("tt")) || !wcscmp(szTag,LTEXT("u"))
				 || !wcscmp(szTag,LTEXT("bdo")) || !wcscmp(szTag,LTEXT("sub")) || !wcscmp(szTag,LTEXT("sup")) )
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
				if( !wcscmp(szTag,LTEXT("rbc")) || !wcscmp(szTag,LTEXT("rtc")) || !wcscmp(szTag,LTEXT("ruby"))
				 || !wcscmp(szTag,LTEXT("rb")) || !wcscmp(szTag,LTEXT("rt")) || !wcscmp(szTag,LTEXT("rp")) )
				{
					nLabelType = LT_INLINE;
				}
				// 空要素（内容を持たないタグ）のうち構造に関係ないものは構造解析しない。
				if( !wcscmp(szTag,LTEXT("br")) || !wcscmp(szTag,LTEXT("base")) || !wcscmp(szTag,LTEXT("basefont"))
				 || !wcscmp(szTag,LTEXT("frame"))
				 // 2014.12.26 Moca 以下の要素を追加
				 || !wcscmp(szTag,LTEXT("wbr"))
				 )
				{
					nLabelType = LT_IGNORE;
				}
				// 空要素（内容を持たないタグ）のうち構造に関係するもの。
				if( !wcscmp(szTag,LTEXT("area")) || !wcscmp(szTag,LTEXT("hr")) || !wcscmp(szTag,LTEXT("img"))
				 || !wcscmp(szTag,LTEXT("input")) || !wcscmp(szTag,LTEXT("link")) || !wcscmp(szTag,LTEXT("meta"))
				 || !wcscmp(szTag,LTEXT("param"))
				 // 2014.12.26 Moca 以下の要素を追加
				 || !wcscmp(szTag,LTEXT("col")) || !wcscmp(szTag,LTEXT("command")) || !wcscmp(szTag,LTEXT("embed"))
				 || !wcscmp(szTag,LTEXT("keygen")) || !wcscmp(szTag,LTEXT("source")) || !wcscmp(szTag,LTEXT("track"))
				 )
				{
					nLabelType = LT_EMPTY;
				}
				if( !wcscmp(szTag,LTEXT("div")) || !wcscmp(szTag,LTEXT("center"))
				 || !wcscmp(szTag,LTEXT("address")) || !wcscmp(szTag,LTEXT("blockquote"))
				 || !wcscmp(szTag,LTEXT("noscript")) || !wcscmp(szTag,LTEXT("noframes"))
				 || !wcscmp(szTag,LTEXT("ol")) || !wcscmp(szTag,LTEXT("ul")) || !wcscmp(szTag,LTEXT("dl"))
				 || !wcscmp(szTag,LTEXT("dir")) || !wcscmp(szTag,LTEXT("menu"))
				 || !wcscmp(szTag,LTEXT("pre")) || !wcscmp(szTag,LTEXT("table"))
				 || !wcscmp(szTag,LTEXT("form")) || !wcscmp(szTag,LTEXT("fieldset")) || !wcscmp(szTag,LTEXT("isindex")) )
				{
					nLabelType = LT_BLOCK;
				}
				if( !wcscmp(szTag,LTEXT("p")) )
				{
					nLabelType = LT_PARAGRAPH;
				}
				if( (szTag[0]==LCHAR('h')) && (LCHAR('1')<=szTitle[1]&&szTitle[1]<=LCHAR('6')) ){
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
						if( nHeadDepth[szTitle[1]-LCHAR('0')] != -1 ) // 小見出し:既出
						{
							nDepth = nHeadDepth[szTitle[1]-LCHAR('0')];
							for(k=szTitle[1]-LCHAR('0');k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-LCHAR('0')] = nDepth;
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
									if (pLine[j] == LCHAR('/') && pLine[j+1]== LCHAR('>')) {
										bEndTag = true; // <emptytag />
										bLoop = false;
										j+=2;
										break;
									}else if ( pLine[j] == LCHAR('>') ) {
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
						if (x < int(std::size(szTitle)) - 3) {
							if(!bEndTag)
							{
								szTitle[k++]	=	LCHAR(' ');
								bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
								for (; i+j < nLineLen && k < int(std::size(szTitle)) -1 ; k++, j++) {
									if (pLine[j]==LCHAR('<') || WCODE::IsLineDelimiter(pLine[j], bExtEol)){
										j--;
										break;
									}
									szTitle[k] = pLine[j];
								}
							}
						}
						szTitle[k]	=	LCHAR('\0');
						pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptPos.GetY2()+CLayoutInt(1), szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<int(std::size(szTitle))-1;j++)
						{
							if(pLine[j]==LCHAR('>'))
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	LCHAR('\0');
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
						if(!_wcsicmp(pszStack[nDepth],szTitle)){
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
						if(_wcsicmp(pszStack[nDepth],szTitle)){
							nDepth = nDepthOrg;
						}
					}
				}else{
					if( nLabelType==LT_HEADING ){	//	見出しの終わり
						nHeadDepth[szTitle[1]-LCHAR('0')] = nDepth;
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

const WCHAR* g_ppszKeywordsHTML[] = {
	LTEXT("_blank"),
	LTEXT("_parent"),
	LTEXT("_self"),
	LTEXT("_top"),
	LTEXT("A"),
	LTEXT("ABBR"),
	LTEXT("ABOVE"),
	LTEXT("absbottom"),
	LTEXT("absmiddle"),
	LTEXT("ACCESSKEY"),
	LTEXT("ACRONYM"),
	LTEXT("ACTION"),
	LTEXT("ADDRESS"),
	LTEXT("ALIGN"),
	LTEXT("all"),
	LTEXT("APPLET"),
	LTEXT("AREA"),
	LTEXT("ARTICLE"),
	LTEXT("ASIDE"),
	LTEXT("AUDIO"),
	LTEXT("AUTOPLAY"),
	LTEXT("AUTOSTART"),
	LTEXT("B"),
	LTEXT("BACKGROUND"),
	LTEXT("BASE"),
	LTEXT("BASEFONT"),
	LTEXT("baseline"),
	LTEXT("BDI"),
	LTEXT("BDO"),
	LTEXT("BEHAVIOR"),
	LTEXT("BELOW"),
	LTEXT("BGCOLOR"),
	LTEXT("BGSOUND"),
	LTEXT("BIG"),
	LTEXT("BLINK"),
	LTEXT("BLOCKQUOTE"),
	LTEXT("BODY"),
	LTEXT("BORDER"),
	LTEXT("BORDERCOLOR"),
	LTEXT("BORDERCOLORDARK"),
	LTEXT("BORDERCOLORLIGHT"),
	LTEXT("BOTTOM"),
	LTEXT("box"),
	LTEXT("BR"),
	LTEXT("BUTTON"),
	LTEXT("CANVAS"),
	LTEXT("CAPTION"),
	LTEXT("CELLPADDING"),
	LTEXT("CELLSPACING"),
	LTEXT("CENTER"),
	LTEXT("CHALLENGE"),
	LTEXT("char"),
	LTEXT("checkbox"),
	LTEXT("CHECKED"),
	LTEXT("CITE"),
	LTEXT("CLEAR"),
	LTEXT("CLIP"),
	LTEXT("CODE"),
	LTEXT("CODEBASE"),
	LTEXT("CODETYPE"),
	LTEXT("COL"),
	LTEXT("COLGROUP"),
	LTEXT("COLOR"),
	LTEXT("COLS"),
	LTEXT("COLSPAN"),
	LTEXT("COMMENT"),
	LTEXT("CONTROLS"),
	LTEXT("DATA"),
	LTEXT("DATALIST"),
	LTEXT("DD"),
	LTEXT("DECLARE"),
	LTEXT("DEFER"),
	LTEXT("DEL"),
	LTEXT("DETAILS"),
	LTEXT("DELAY"),
	LTEXT("DFN"),
	LTEXT("DIALOG"),
	LTEXT("DIR"),
	LTEXT("DIRECTION"),
	LTEXT("DISABLED"),
	LTEXT("DIV"),
	LTEXT("DL"),
	LTEXT("DOCTYPE"),
	LTEXT("DT"),
	LTEXT("EM"),
	LTEXT("EMBED"),
	LTEXT("ENCTYPE"),
	LTEXT("FACE"),
	LTEXT("FIELDSET"),
	LTEXT("FIGCAPTION"),
	LTEXT("FIGURE"),
	LTEXT("file"),
	LTEXT("FONT"),
	LTEXT("FOOTER"),
	LTEXT("FOR"),
	LTEXT("FORM"),
	LTEXT("FRAME"),
	LTEXT("FRAMEBORDER"),
	LTEXT("FRAMESET"),
	LTEXT("GET"),
	LTEXT("groups"),
	LTEXT("GROUPS"),
	LTEXT("GUTTER"),
	LTEXT("H1"),
	LTEXT("H2"),
	LTEXT("H3"),
	LTEXT("H4"),
	LTEXT("H5"),
	LTEXT("H6"),
	LTEXT("H7"),
	LTEXT("HEAD"),
	LTEXT("HEADER"),
	LTEXT("HGROUP"),
	LTEXT("HEIGHT"),
	LTEXT("HIDDEN"),
	LTEXT("Hn"),
	LTEXT("HR"),
	LTEXT("HREF"),
	LTEXT("hsides"),
	LTEXT("HSPACE"),
	LTEXT("HTML"),
	LTEXT("I"),
	LTEXT("ID"),
	LTEXT("IFRAME"),
	LTEXT("ILAYER"),
	LTEXT("image"),
	LTEXT("IMG"),
	LTEXT("INDEX"),
	LTEXT("inherit"),
	LTEXT("INPUT"),
	LTEXT("INS"),
	LTEXT("ISINDEX"),
	LTEXT("JavaScript"),
	LTEXT("justify"),
	LTEXT("KBD"),
	LTEXT("KEYGEN"),
	LTEXT("LABEL"),
	LTEXT("LANGUAGE"),
	LTEXT("LAYER"),
	LTEXT("LEFT"),
	LTEXT("LEGEND"),
	LTEXT("lhs"),
	LTEXT("LI"),
	LTEXT("LINK"),
	LTEXT("LISTING"),
	LTEXT("LOOP"),
	LTEXT("MAIN"),
	LTEXT("MAP"),
	LTEXT("MARK"),
	LTEXT("MARQUEE"),
	LTEXT("MAXLENGTH"),
	LTEXT("MENU"),
	LTEXT("META"),
	LTEXT("METHOD"),
	LTEXT("METHODS"),
	LTEXT("MIDDLE"),
	LTEXT("MULTICOL"),
	LTEXT("MULTIPLE"),
	LTEXT("NAME"),
	LTEXT("NAV"),
	LTEXT("NEXT"),
	LTEXT("NEXTID"),
	LTEXT("NOBR"),
	LTEXT("NOEMBED"),
	LTEXT("NOFRAMES"),
	LTEXT("NOLAYER"),
	LTEXT("none"),
	LTEXT("NOSAVE"),
	LTEXT("NOSCRIPT"),
	LTEXT("NOTAB"),
	LTEXT("NOWRAP"),
	LTEXT("OBJECT"),
	LTEXT("OL"),
	LTEXT("onBlur"),
	LTEXT("onChange"),
	LTEXT("onClick"),
	LTEXT("onFocus"),
	LTEXT("onLoad"),
	LTEXT("onMouseOut"),
	LTEXT("onMouseOver"),
	LTEXT("onReset"),
	LTEXT("onSelect"),
	LTEXT("onSubmit"),
	LTEXT("OPTGROUP"),
	LTEXT("OPTION"),
	LTEXT("OUTPUT"),
	LTEXT("P"),
	LTEXT("PAGEX"),
	LTEXT("PAGEY"),
	LTEXT("PALETTE"),
	LTEXT("PANEL"),
	LTEXT("PARAM"),
	LTEXT("PARENT"),
	LTEXT("password"),
	LTEXT("PICTURE"),
	LTEXT("PLAINTEXT"),
	LTEXT("PLUGINSPAGE"),
	LTEXT("POST"),
	LTEXT("PRE"),
	LTEXT("PREVIOUS"),
	LTEXT("PROGRESS"),
	LTEXT("Q"),
	LTEXT("radio"),
	LTEXT("REL"),
	LTEXT("REPEAT"),
	LTEXT("reset"),
	LTEXT("REV"),
	LTEXT("rhs"),
	LTEXT("RIGHT"),
	LTEXT("rows"),
	LTEXT("ROWSPAN"),
	LTEXT("RP"),
	LTEXT("RT"),
	LTEXT("RUBY"),
	LTEXT("RULES"),
	LTEXT("S"),
	LTEXT("SAMP"),
	LTEXT("SAVE"),
	LTEXT("SCRIPT"),
	LTEXT("SCROLLAMOUNT"),
	LTEXT("SCROLLDELAY"),
	LTEXT("SEARCH"),
	LTEXT("SECTION"),
	LTEXT("SELECT"),
	LTEXT("SELECTED"),
	LTEXT("SERVER"),
	LTEXT("SHAPES"),
	LTEXT("show"),
	LTEXT("SIZE"),
	LTEXT("SMALL"),
	LTEXT("SOURCE"),
	LTEXT("SONG"),
	LTEXT("SPACER"),
	LTEXT("SPAN"),
	LTEXT("SRC"),
	LTEXT("STANDBY"),
	LTEXT("STRIKE"),
	LTEXT("STRONG"),
	LTEXT("STYLE"),
	LTEXT("SUB"),
	LTEXT("submit"),
	LTEXT("SUMMARY"),
	LTEXT("SUP"),
	LTEXT("SVG"),
	LTEXT("TABINDEX"),
	LTEXT("TABLE"),
	LTEXT("TARGET"),
	LTEXT("TBODY"),
	LTEXT("TD"),
	LTEXT("TEMPLATE"),
	LTEXT("TEXT"),
	LTEXT("TEXTAREA"),
	LTEXT("textbottom"),
	LTEXT("TEXTFOCUS"),
	LTEXT("textmiddle"),
	LTEXT("texttop"),
	LTEXT("TFOOT"),
	LTEXT("TH"),
	LTEXT("THEAD"),
	LTEXT("TIME"),
	LTEXT("TITLE"),
	LTEXT("TOP"),
	LTEXT("TR"),
	LTEXT("TRACK"),
	LTEXT("TT"),
	LTEXT("TXTCOLOR"),
	LTEXT("TYPE"),
	LTEXT("U"),
	LTEXT("UL"),
	LTEXT("URN"),
	LTEXT("USEMAP"),
	LTEXT("VALIGN"),
	LTEXT("VALUE"),
	LTEXT("VALUETYPE"),
	LTEXT("VAR"),
	LTEXT("VIDEO"),
	LTEXT("VISIBILITY"),
	LTEXT("void"),
	LTEXT("vsides"),
	LTEXT("VSPACE"),
	LTEXT("WBR"),
	LTEXT("WIDTH"),
	LTEXT("WRAP"),
	LTEXT("XMP")
};
int g_nKeywordsHTML = int(std::size(g_ppszKeywordsHTML));
