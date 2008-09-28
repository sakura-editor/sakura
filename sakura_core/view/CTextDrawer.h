#pragma once

#include "env/CShareData.h" //ColorInfo
class CTextMetrics;
class CTextArea;
class CViewFont;
class CEol;
class CEditView;
class CLayout;
#include "DispPos.h"

class CTextDrawer{
public:
	CTextDrawer(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CTextDrawer(){}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         外部依存                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//領域のインスタンスを求める
	const CTextArea* GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
	//実際には pX と nX が更新される。
	void DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const; // テキスト表示 (新しい版)
	void DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const;      // テキスト表示 (古い版)

	// -- -- 指定桁縦線描画 -- -- //
	//!	指定桁縦線描画関数	// 2005.11.08 Moca
	void DispVerticalLines( CGraphics& gr, int nTop, int nBottom, CLayoutInt nLeftCol, CLayoutInt nRightCol ) const;

	// -- -- 行番号 -- -- //
	void DispLineNumber( CGraphics& gr, CLayoutInt nLineNum, int y ) const;		// 行番号表示

public: //####仮
	// -- -- 実装補助 -- -- //
	EColorIndexType _GetColorIdx(EColorIndexType nColorIdx) const;


private:
	const CEditView* m_pEditView;
};

