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
#ifndef SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_
#define SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_

#include <vector>
#include "view/colors/CColorStrategy.h" //SColorStrategyInfo


//$$レイアウト構築フロー(DoLayout)も CFigure で行うと整理しやすい
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText, int nTextLen) const = 0;

	//! 色分け表示対象判定
	virtual bool Disp(void) const = 0;

	//! 設定更新
	virtual void Update(void)
	{
		CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	}
protected:
	const STypeConfig* m_pTypeData;
};

//! 通常テキスト描画
class CFigure_Text : public CFigure{
public:
	bool DrawImp(SColorStrategyInfo* pInfo);
	bool Match(const wchar_t* pText, int nTextLen) const
	{
		return true;
	}

	//! 色分け表示対象判定
	virtual bool Disp(void) const
	{
		return true;
	}
};

//! 各種空白（半角空白／全角空白／タブ／改行）描画用の基本クラス
class CFigureSpace : public CFigure{
public:
	virtual bool DrawImp(SColorStrategyInfo* pInfo);
protected:
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	//! 色分け表示対象判定
	virtual bool Disp(void) const
	{
		EColorIndexType nColorIndex = GetColorIdx();
		return m_pTypeData->m_ColorInfoArr[nColorIndex].m_bDisp;
	}

	virtual void Update(void)
	{
		CFigure::Update();

		EColorIndexType nColorIndex = GetColorIdx();
		if( m_pTypeData->m_ColorInfoArr[nColorIndex].m_bDisp ){
			m_nDispColorIndex = nColorIndex;
		}else{
			m_nDispColorIndex = COLORIDX_TEXT;
		}
	}

	EColorIndexType GetDispColorIdx(void) const{ return m_nDispColorIndex; }

	// 実装補助
	bool DrawImp_StyleSelect(SColorStrategyInfo* pInfo);
	void DrawImp_StylePop(SColorStrategyInfo* pInfo);
	void DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos&);

protected:
	EColorIndexType m_nDispColorIndex;
};

#endif /* SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_ */
/*[EOF]*/
