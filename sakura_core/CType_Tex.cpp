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
#include "charcode.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka

/*! TeX アウトライン解析

	@author naoh
	@date 2003.07.21 naoh 新規作成
	@date 2005.01.03 naoh 「マ」などの"}"を含む文字に対する修正、prosperのslideに対応
*/
void CEditDoc::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;

	const int nMaxStack = 8;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char szTag[32], szTitle[256];			//	一時領域
	int thisSection=0, lastSection = 0;	// 現在のセクション種類と一つ前のセクション種類
	int stackSection[nMaxStack];		// 各深さでのセクションの番号
	int nStartTitlePos;					// \section{dddd} の dddd の部分の始まる番号
	int bNoNumber;						// * 付の場合はセクション番号を付けない

	// 一行ずつ
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	(const char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine) break;
		// 一文字ずつ
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == '%' && !(i>0 && _IS_SJIS_1(pLine[i-1])) ) break;	// コメントなら以降はいらない
			if(pLine[i] != '\\' 
				&& !(i>0 && _IS_SJIS_1(pLine[i-1]))	// 「\」の前の文字がSJISの1バイト目なら次の文字へ
				|| nDepth>=nMaxStack) continue;	// 「\」がないなら次の文字へ
			++i;
			// 見つかった「\」以降の文字列チェック
			for(j=0;i+j<nLineLen && j<sizeof(szTag)-1;j++)
			{
				if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = '\0';
			}else{
				szTag[j]   = '\0';
			}
//			MessageBox(NULL, szTitle, "", MB_OK);

			thisSection = 0;
			if(!strcmp(szTag,"subsubsection")) thisSection = 4;
			else if(!strcmp(szTag,"subsection")) thisSection = 3;
			else if(!strcmp(szTag,"section")) thisSection = 2;
			else if(!strcmp(szTag,"chapter")) thisSection = 1;
			else if(!strcmp(szTag,"begin")) {		// beginなら prosperのslideの可能性も考慮
				// さらに{slide}{}まで読みとっておく
				if(strstr(pLine, "{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<sizeof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// sectionの中身取得
				for(k=0;nStartTitlePos+k<nLineLen && k<sizeof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// 次はチェック不要
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				int		nPosX;
				int		nPosY;
				TCHAR tmpstr[256];
				TCHAR secstr[4];

				m_cLayoutMgr.LogicToLayout(
					i,
					nLineCount,
					&nPosX,
					&nPosY
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = '\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						sprintf(secstr, "%d.", stackSection[k]);
						strcat(tmpstr, secstr);
					}
					strcat(tmpstr, " ");
				}
				strcat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1, tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}

/*[EOF]*/
