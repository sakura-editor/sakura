/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_
#define SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_
#pragma once

class CEditView;
class CNativeW;

//!品詞解析クラス
class CViewParser{
public:
	CViewParser(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CViewParser(){}

	//! カーソル直前の単語を取得
	int GetLeftWord( CNativeW* pcmemWord, int nMaxWordLen ) const;

	//! キャレット位置の単語を取得
	// 2006.03.24 fon
	bool GetCurrentWord( CNativeW* pcmemWord ) const;

private:
	const CEditView* m_pEditView;
};
#endif /* SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_ */
