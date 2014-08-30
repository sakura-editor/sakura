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
#include "my_icmp.h" // 2002/11/30 Moca 追加

/*! COBOL アウトライン解析 */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
//	int						j;
//	int						nCharChars;
//	int						nCharChars2;
//	int						nStartsLen;
//	char*					pszText;
	int						k;
//	int						m;
	char					szDivision[1024];
	char					szLabel[1024];
	const char*				pszKeyWord;
	int						nKeyWordLen;
	BOOL					bDivision;

	szDivision[0] = '\0';
	szLabel[0] =  '\0';


	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == '*' ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != ' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == CR
				 || pLine[i] == LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == ' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != ' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = '\0';
//			MYTRACE( _T("szLabel=[%s]\n"), szLabel );



			pszKeyWord = "division";
			nKeyWordLen = lstrlen( pszKeyWord );
			bDivision = FALSE;
			int nLen = (int)lstrlen( szLabel ) - nKeyWordLen;
			for( i = 0; i <= nLen; ++i ){
				if( 0 == my_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = '\0';
					strcpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/

			int		nPosX;
			int		nPosY;
			char	szWork[1024];
			m_cLayoutMgr.LogicToLayout(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			wsprintf( szWork, "%s::%s", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)szWork, 0 );
		}
	}
	return;
}

/*[EOF]*/
