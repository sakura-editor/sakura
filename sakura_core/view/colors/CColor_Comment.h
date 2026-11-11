/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLOR_COMMENT_C056FE79_A77B_4451_AA5E_1FA5517C0EC1_H_
#define SAKURA_CCOLOR_COMMENT_C056FE79_A77B_4451_AA5E_1FA5517C0EC1_H_
#pragma once

#include "view/colors/CColorStrategy.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_LineComment final : public CColorStrategy{
public:
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_COMMENT; }
	void InitStrategyStatus() override{}
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool EndColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{
		// タイプ別設定 『カラー』プロパティのコメントのリストアイテムのチェックが付いているか
		if (!m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp)
			return false;
		// 行型コメントの始点記号が入力されているか
		auto& lineComment = m_pTypeData->m_cLineComment;
		for (int i = 0; i < COMMENT_DELIMITER_NUM; ++i) {
			if (lineComment.getLineComment(i)[0])
				return true;
		}
		return false;
	}
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_BlockComment final : public CColorStrategy{
public:
	CColor_BlockComment(EColorIndexType nType) : m_nType(nType), m_nCOMMENTEND(0){}
	void Update(void) override {
		const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
		m_pcBlockComment = &m_pTypeData->m_cBlockComments[m_nType - COLORIDX_BLOCK1];
	}
	EColorIndexType GetStrategyColor() const override{ return m_nType; }
	void InitStrategyStatus() override{ m_nCOMMENTEND = 0; }
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool EndColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{
		// タイプ別設定 『カラー』プロパティのコメントのリストアイテムのチェックが付いているか
		if (!m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp)
			return false;
		// ブロック型の始点・終端記号が入力されているか
		return m_pcBlockComment->getBlockFromLen() > 0 && m_pcBlockComment->getBlockToLen() > 0;
	}
private:
	EColorIndexType m_nType;
	const CBlockComment* m_pcBlockComment;
	int m_nCOMMENTEND;
};
#endif /* SAKURA_CCOLOR_COMMENT_C056FE79_A77B_4451_AA5E_1FA5517C0EC1_H_ */
