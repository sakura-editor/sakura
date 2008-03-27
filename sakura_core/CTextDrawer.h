#pragma once

#include "CShareData.h" //ColorInfo
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

	//2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
	//2007.08.25 kobake 引数から nCharWidth, nLineHeight を削除
	//実際には pX と nX が更新される。
	//	EOF描画関数	//	2004.05.29 genta
	//2007.08.28 kobake 引数 fuOptions を削除
	void DispEOF( HDC hdc, DispPos* pDispPos) const;

	//May 23, 2000 genta
	//画面描画補助関数
	//@@@ 2001.12.21 YAZAKI 改行記号の書きかたが変だったので修正
	void _DrawEOL(HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight,
		CEol cEol, bool bBold, COLORREF pColor ) const;
	//2007.08.30 kobake 追加
	void DispEOL(HDC hdc, DispPos* pDispPos, CEol cEol, bool bSearchStringMode) const;

	// -- -- タブ描画 -- -- //
	//2007.08.28 kobake 追加
	void DispTab( HDC hdc, DispPos* pDispPos, int nColorIdx ) const;
	//タブ矢印描画関数	//@@@ 2003.03.26 MIK
	void _DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor ) const;

	// -- -- スペース描画 -- -- //
	void DispZenkakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const;
	void DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const;

	// -- -- 折り返し描画 -- -- //
	void DispWrap(HDC hdc, DispPos* pDispPos) const;

	// -- -- 空(から)行描画 -- -- //
	bool DispEmptyLine(HDC hdc, DispPos* pDispPos) const;

	// -- -- 指定桁縦線描画 -- -- //
	//!	指定桁縦線描画関数	// 2005.11.08 Moca
	void DispVerticalLines( HDC hdc, int nTop, int nBottom, CLayoutInt nLeftCol, CLayoutInt nRightCol ) const;

	// -- -- 行番号 -- -- //
	void DispLineNumber( HDC hdc, const CLayout* pcLayout, int nLineNum, int y ) const;		// 行番号表示

protected:
	// -- -- 実装補助 -- -- //
	int _GetColorIdx(int nColorIdx,bool bSearchStringMode) const;


private:
	const CEditView* m_pEditView;
};

