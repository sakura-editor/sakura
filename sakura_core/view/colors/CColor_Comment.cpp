/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_LineComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// 行コメント
	if( m_pTypeData->m_cLineComment.Match( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		return true;
	}
	return false;
}

bool CColor_LineComment::EndColor(const CStringRef& cStr, int nPos)
{
	//文字列終端
	if( nPos >= cStr.GetLength() ){
		return true;
	}

	//改行
	if( WCODE::IsLineDelimiter(cStr.At(nPos), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
		return true;
	}

	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_BlockComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// ブロックコメント
	if( m_pcBlockComment->Match_CommentFrom( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		/* この物理行にブロックコメントの終端があるか */	//@@@ 2002.09.22 YAZAKI
		this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
			nPos + m_pcBlockComment->getBlockFromLen(),
			cStr
		);

		return true;
	}
	return false;
}

bool CColor_BlockComment::EndColor(const CStringRef& cStr, int nPos)
{
	if( 0 == this->m_nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
			nPos,
			cStr
		);
	}
	else if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}
