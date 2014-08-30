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

/*! アセンブラ アウトライン解析

	@author MIK
	@date 2004.04.12 作り直し
*/
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	int nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( int nLineCount = 0; nLineCount < nTotalLine; nLineCount++ ){
		const TCHAR* pLine;
		int nLineLen;
		TCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		TCHAR* token[MAX_ASM_TOKEN];
		int j;
		TCHAR* p;

		//1行取得する。
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//作業用にコピーを作成する。バイナリがあったらその後ろは知らない。
		pTmpLine = _tcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( _tcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//バイナリを含んでいたら短くなるので...
			pTmpLine[ nLineLen ] = _T('\0');	//指定長で切り詰め
		}

		//行コメント削除
		p = _tcsstr( pTmpLine, _T(";") );
		if( p ) *p = _T('\0');

		length = _tcslen( pTmpLine );
		offset = 0;

		//トークンに分割
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok( pTmpLine, length, &offset, _T(" \t\r\n") );
			if( token[ j ] == NULL ) break;
			//トークンに含まれるべき文字でないか？
			if( _tcsstr( token[ j ], _T("\"")) != NULL
			 || _tcsstr( token[ j ], _T("\\")) != NULL
			 || _tcsstr( token[ j ], _T("'" )) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//トークンが1個以上ある
			int nFuncId = -1;
			TCHAR* entry_token = NULL;

			length = _tcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == _T(':') ){	//ラベル
				token[ 0 ][ length - 1 ] = _T('\0');
				nFuncId = 51;
				entry_token = token[ 0 ];
			}else
			if( token[ 1 ] != NULL ){	//トークンが2個以上ある
				if( my_stricmp( token[ 1 ], _T("proc") ) == 0 ){	//関数
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( my_stricmp( token[ 1 ], _T("endp") ) == 0 ){	//関数終了
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//マクロ
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//構造体
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				int		nPosX;
				int		nPosY;
				m_cLayoutMgr.LogicToLayout(
					0,
					nLineCount/*nFuncLine - 1*/,
					&nPosX,
					&nPosY
				);
				pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}

/*[EOF]*/
