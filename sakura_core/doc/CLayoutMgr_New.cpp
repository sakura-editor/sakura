/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "debug/Debug.h"
#include <commctrl.h>
#include "debug/CRunningTimer.h"
#include "doc/CLayout.h"/// 2002/2/10 aroka
#include "doc/CDocLine.h"/// 2002/2/10 aroka
#include "doc/CDocLineMgr.h"// 2002/2/10 aroka
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "util/window.h"
#include "view/colors/CColorStrategy.h"




/*!
	行頭禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuHead( wchar_t wc )
{
	return m_pszKinsokuHead_1.exist(wc);
}

/*!
	行末禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuTail( wchar_t wc )
{
	return m_pszKinsokuTail_1.exist(wc);
}


/*!
	禁則対象句読点に該当するかを調べる．

	@param [in] pLine  調べる文字へのポインタ
	@param [in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuKuto( wchar_t wc )
{
	return m_pszKinsokuKuto_1.exist(wc);
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosHead(
	CLayoutInt nRest,		//!< [in] 行の残り文字数
	CLayoutInt nCharKetas,	//!< [in] 現在位置の文字サイズ
	CLayoutInt nCharKetas2	//!< [in] 現在位置の次の文字サイズ
)
{
	switch( (Int)nRest )
	{
	//    321012  ↓マジックナンバー
	// 3 "る）" : 22 "）"の2バイト目で折り返しのとき
	// 2  "Z）" : 12 "）"の2バイト目で折り返しのとき
	// 2  "る）": 22 "）"で折り返しのとき
	// 2  "る)" : 21 ")"で折り返しのとき
	// 1   "Z）": 12 "）"で折り返しのとき
	// 1   "Z)" : 11 ")"で折り返しのとき
	//↑何文字前か？
	// ※ただし、"るZ"部分が禁則なら処理しない。
	case 3:	// 3文字前
		if( nCharKetas == 2 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 2:	// 2文字前
		if( nCharKetas == 2 ){
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 1:	// 1文字前
		if( nCharKetas == 1 ){
			return true;
		}
		break;
	}
	return false;
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosTail(
	CLayoutInt nRest,		//!< [in] 行の残り文字数
	CLayoutInt nCharKetas,	//!< [in] 現在位置の文字サイズ
	CLayoutInt nCharKetas2	//!< [in] 現在位置の次の文字サイズ
)
{
	switch( (Int)nRest )
	{
	case 3:	// 3文字前
		if( nCharKetas == 2 && nCharKetas2 == 2){
			// "（あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 2:	// 2文字前
		if( nCharKetas == 2 ){
			// "（あ": "あ"で折り返しのとき
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2){
			// "(あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 1:	// 1文字前
		if( nCharKetas == 1 ){
			// "(あ": "あ"で折り返しのとき
			return true;
		}
		break;
	}
	return false;
}

int CLayoutMgr::Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr/*int nLineLen, const wchar_t* pLine*/ ) const
{
	int nCharChars;
	int i;
	for( i = nPos; i < cLineStr.GetLength(); ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( cLineStr.GetPtr(), cLineStr.GetLength(), i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_sTypeConfig.m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && cLineStr.At(i) == L'\\' ){
				++i;
			}else
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				return i + 1;
			}
		}
		else if(	m_sTypeConfig.m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				if( i + 1 < cLineStr.GetLength() && cLineStr.At(i + 1) == wcQuote ){
					++i;
				}else{
					return i + 1;
				}
			}
		}
		if( 2 == nCharChars ){
			++i;
		}
	}
	return cLineStr.GetLength();
}

/*!
	@brief 行の長さを計算する (2行目以降の字下げ無し)
	
	字下げを行わないので，常に0を返す．
	引数は使わない．
	
	@return 1行の表示文字数 (常に0)
	
	@author genta
	@date 2002.10.01
*/
CLayoutInt CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return CLayoutInt(0);
}

/*!
	@brief インデント幅を計算する (Tx2x)
	
	前の行の最後のTABの位置をインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author Yazaki
	@return インデントすべき文字数
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI 名称変更, 処理見直し
*/
CLayoutInt CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	CLayoutInt nIpos = pLayoutPrev->GetIndent();

	//	前の行が折り返し行ならばそれに合わせる
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*!
	@brief インデント幅を計算する (スペース字下げ版)
	
	論理行行頭のホワイトスペースの終わりインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author genta
	@return インデントすべき文字数
	
	@date 2002.10.01 
*/
CLayoutInt CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	//	インデントの計算
	CLayoutInt nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	折り返しの3行目以降は1つ前の行のインデントに合わせる．
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI インデントの計算
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );

	//	Jul. 20, 2003 genta 自動インデントに準じた動作にする
	bool bZenSpace = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
	const wchar_t* szSpecialIndentChar = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && WCODE::IsIndentChar(it.getCurrentChar(),bZenSpace) )
		{
			//	インデントのカウントを継続する
		}
		//	Jul. 20, 2003 genta インデント対象文字
		else if( szSpecialIndentChar[0] != L'\0' ){
			wchar_t buf[3]; // 文字の長さは1 or 2
			wmemcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = L'\0';
			if( NULL != wcsstr( szSpecialIndentChar, buf )){
				//	インデントのカウントを継続する
			}
			else {
				nIpos = it.getColumn();	//	終了
				break;
			}
		}
		else {
			nIpos = it.getColumn();	//	終了
			break;
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}


