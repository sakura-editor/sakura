/*!	@file
@brief CViewCommanderクラスのコマンド(変換系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, Stonee, Misaka
	Copyright (C) 2002, ai

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"
#include "io/CBinaryStream.h"


/* 小文字 */
void CViewCommander::Command_TOLOWER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOLOWER );
	return;
}



/* 大文字 */
void CViewCommander::Command_TOUPPER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOUPPER );
	return;
}



/* 全角→半角 */
void CViewCommander::Command_TOHANKAKU( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANKAKU );
	return;
}



/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
void CViewCommander::Command_TOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUKATA );
	return;
}



/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
void CViewCommander::Command_TOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUHIRA );
	return;
}



/*! 半角英数→全角英数 */			//July. 30, 2001 Misaka
void CViewCommander::Command_TOZENEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENEI );
	return;
}



/*! 全角英数→半角英数 */
void CViewCommander::Command_TOHANEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANEI );
	return;
}



/* 全角カタカナ→半角カタカナ */		//Aug. 29, 2002 ai
void CViewCommander::Command_TOHANKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANKATA );
	return;
}



/* 半角カタカナ→全角カタカナ */
void CViewCommander::Command_HANKATATOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENKATA );
	return;
}



/* 半角カタカナ→全角ひらがな */
void CViewCommander::Command_HANKATATOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENHIRA );
	return;
}



/* TAB→空白 */
void CViewCommander::Command_TABTOSPACE( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TABTOSPACE );
	return;
}

/* 空白→TAB */ //---- Stonee, 2001/05/27
void CViewCommander::Command_SPACETOTAB( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_SPACETOTAB );
	return;
}



/* 自動判別→SJISコード変換 */
void CViewCommander::Command_CODECNV_AUTO2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_AUTO2SJIS );
	return;
}



/* E-Mail(JIS→SJIS)コード変換 */
void CViewCommander::Command_CODECNV_EMAIL( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EMAIL );
	return;
}




/* EUC→SJISコード変換 */
void CViewCommander::Command_CODECNV_EUC2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EUC2SJIS );
	return;
}



/* Unicode→SJISコード変換 */
void CViewCommander::Command_CODECNV_UNICODE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODE2SJIS );
	return;
}



/* UnicodeBE→SJISコード変換 */
void CViewCommander::Command_CODECNV_UNICODEBE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODEBE2SJIS );
	return;
}



/* UTF-8→SJISコード変換 */
void CViewCommander::Command_CODECNV_UTF82SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF82SJIS );
	return;
}



/* UTF-7→SJISコード変換 */
void CViewCommander::Command_CODECNV_UTF72SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF72SJIS );
	return;
}



/* SJIS→JISコード変換 */
void CViewCommander::Command_CODECNV_SJIS2JIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2JIS );
	return;
}



/* SJIS→EUCコード変換 */
void CViewCommander::Command_CODECNV_SJIS2EUC( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2EUC );
	return;
}



/* SJIS→UTF-8コード変換 */
void CViewCommander::Command_CODECNV_SJIS2UTF8( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF8 );
	return;
}



/* SJIS→UTF-7コード変換 */
void CViewCommander::Command_CODECNV_SJIS2UTF7( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF7 );
	return;
}



/* Base64デコードして保存 */
void CViewCommander::Command_BASE64DECODE( void )
{
	/* テキストが選択されているか */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedDataSimple(ctextBuf) ){
		ErrorBeep();
		return;
	}

	/* Base64デコード */
	CMemory cmemBuf;
	bool bret = CDecode_Base64Decode().CallDecode(ctextBuf, &cmemBuf);
	if( !bret ){
		return;
	}
	ctextBuf.Clear();

	/* 保存ダイアログ モーダルダイアログの表示 */
	TCHAR		szPath[_MAX_PATH] = _T("");
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//データ
	int nDataLen;
	const void* pData = cmemBuf.GetRawPtr(&nDataLen);

	//カキコ
	CBinaryOutputStream out(szPath);
	if(!out)goto err;
	if( nDataLen != out.Write(pData, nDataLen) )goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD14), szPath );
}




/* uudecodeして保存 */
void CViewCommander::Command_UUDECODE( void )
{
	/* テキストが選択されているか */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	// 選択範囲のデータを取得 -> cmemBuf
	// 正常時はTRUE,範囲未選択の場合はFALSEを返す
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedDataSimple(ctextBuf) ){
		ErrorBeep();
		return;
	}

	// uudecode(デコード)  ctextBuf -> cmemBin, szPath
	CMemory cmemBin;
	TCHAR szPath[_MAX_PATH]=_T("");
	CDecode_UuDecode decoder;
	if( !decoder.CallDecode(ctextBuf, &cmemBin) ){
		return;
	}
	decoder.CopyFilename( szPath );
	ctextBuf.Clear();

	/* 保存ダイアログ モーダルダイアログの表示 */
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//データ
	int nDataLen;
	const void* pData = cmemBin.GetRawPtr(&nDataLen);

	//カキコ
	CBinaryOutputStream out(szPath);
	if( !out )goto err;
	if( nDataLen != out.Write(pData,nDataLen) )goto err;

	//完了
	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD16), szPath );
}
