﻿/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CFUNCINFOARR_6E07CB18_7314_42F8_BA10_7C20E5ADCB76_H_
#define SAKURA_CFUNCINFOARR_6E07CB18_7314_42F8_BA10_7C20E5ADCB76_H_
#pragma once

class CFuncInfo;
#include <string>
#include <map>
#include "util/design_template.h"
#include "basis/SakuraBasis.h"
#include "basis/CMyString.h"

// 標準的な付加情報定数
#define FL_OBJ_DEFINITION	0	// 親クラスの定義位置
#define FL_OBJ_DECLARE		1	// 関数プロトタイプ宣言
#define FL_OBJ_FUNCTION		2	// 関数
#define FL_OBJ_CLASS		3	// クラス
#define FL_OBJ_STRUCT		4	// 構造体
#define FL_OBJ_ENUM			5	// 列挙体
#define FL_OBJ_UNION		6	// 共用体
#define FL_OBJ_NAMESPACE	7	// 名前空間
#define FL_OBJ_INTERFACE	8	// インタフェース
#define FL_OBJ_GLOBAL		9	// グローバル（組み込み解析では使用しない）
#define FL_OBJ_ELEMENT_MAX	30	// プラグインで追加可能な定数の上限

//! アウトライン解析 データ配列
class CFuncInfoArr {
public:
	CFuncInfoArr();	/* CFuncInfoArrクラス構築 */
	~CFuncInfoArr();	/* CFuncInfoArrクラス消滅 */
	CFuncInfo* GetAt( int nIdx );	/* 0<=の指定番号のデータを返す */
	void AppendData( CFuncInfo* pcFuncInfo );	/* 配列の最後にデータを追加する */
	void AppendData( CLogicInt nFuncLineCRLF, CLayoutInt nFuncLineLAYOUT, const WCHAR* pszFuncName,
					 int nInfo, int nDepth = 0 );	/* 配列の最後にデータを追加する 2002.04.01 YAZAKI 深さ導入*/
	void AppendData( CLogicInt nLogicLine, CLogicInt nLogicCol, CLayoutInt nLayoutLine, CLayoutInt nLayoutCol, const WCHAR*, const WCHAR*, int, int nDepth = 0 );	/* 配列の最後にデータを追加する 2010.03.01 syat 桁導入*/
	int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* 配列要素数を返す */
	void Empty( void );
	void DUMP( void );
	void SetAppendText( int info, std::wstring s, bool overwrite );
	std::wstring GetAppendText( int info );
	int AppendTextLenMax(){ return m_nAppendTextLenMax; }

public:
	SFilePath	m_szFilePath;	/*!< 解析対象ファイル名 */
private:
	int			m_nFuncInfoArrNum;	/*!< 配列要素数 */
	CFuncInfo**	m_ppcFuncInfoArr;	/*!< 配列 */
	std::map<int, std::wstring>	m_AppendTextArr;	// 追加文字列のリスト
	int			m_nAppendTextLenMax;

	DISALLOW_COPY_AND_ASSIGN(CFuncInfoArr);
};
#endif /* SAKURA_CFUNCINFOARR_6E07CB18_7314_42F8_BA10_7C20E5ADCB76_H_ */
