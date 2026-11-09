/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_CodeAutoToSjis.h"

#include "charset/CCodeFactory.h"
#include "charset/CCodeMediator.h"

/*!
	コンストラクタ
 */
CConvert_CodeAutoToSjis::CConvert_CodeAutoToSjis(const SEncodingConfig& sEncodingConfig) noexcept
	: m_sEncodingConfig(sEncodingConfig)
{
}

/*!
	文字コード変換 自動判別→SJIS

	@date 2014/02/10 Moca F_CODECNV_AUTO2SJIS追加。
 */
bool CConvert_CodeAutoToSjis::DoConvert(CNativeW* pcData)
{
	// Shift-JISに変換する（変換エラーは無視する）
	CMemory cmemSjis;
	CCodeFactory::CreateCodeBase(CODE_SJIS)->UnicodeToCode(*pcData, &cmemSjis);

	// バイナリシーケンスの文字コードを自動検出する
	CCodeMediator m(m_sEncodingConfig);
	ECodeType eCodeType = m.CheckKanjiCode(LPCSTR(cmemSjis.GetRawPtr()), size_t(cmemSjis.GetRawLength()));

	// 検出された文字コードに基づいて変換に使うCCodeBaseを生成する
	std::unique_ptr<CCodeBase> pcCodeBase;
	switch (eCodeType)
	{
	case CODE_JIS:
		// JIS変換はbase64デコードを行うモードを使う
		pcCodeBase = std::unique_ptr<CCodeBase>(CCodeFactory::CreateCodeBase(CODE_JIS, true));
		break;

	case CODE_EUC:
	case CODE_UNICODE:
	case CODE_UNICODEBE:
	case CODE_UTF8:
	case CODE_UTF7:
		// サポートされているその他の変換
		pcCodeBase = CCodeFactory::CreateCodeBase(eCodeType);
		break;

	default:
		// サポートされていない変換は失敗扱いにする(変換しない)
		return false;
	}

	// 検出された文字コードに基づいてUnicode変換する（変換エラーは無視する）
	pcCodeBase->CodeToUnicode(cmemSjis, pcData);

	return true;
}
