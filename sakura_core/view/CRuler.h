#pragma once

class CTextArea;
class CEditView;
class CEditDoc;
class CTextMetrics;

class CRuler{
public:
	CRuler(const CEditView* pEditView, const CEditDoc* pEditDoc);
	virtual ~CRuler();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! ルーラー描画 (背景とキャレット)
	void DispRuler( HDC );

	//! ルーラーの背景のみ描画 2007.08.29 kobake 追加
	void DrawRulerBg(HDC hdc);

	//! ルーラーのキャレットのみ描画 2002.02.25 Add By KK
	void DrawRulerCaret( HDC hdc );

public:
	void SetRedrawFlag(){ m_bRedrawRuler = true; }

private:
	void _DrawRulerCaret( HDC hdc, int nCaretDrawX, int nCaretWidth );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	//参照
	const CEditView*	m_pEditView;
	const CEditDoc*		m_pEditDoc;

	//状態
	bool	m_bRedrawRuler;		// ルーラー全体を描き直す時 = true      2002.02.25 Add By KK
	int		m_nOldRulerDrawX;	// 前回描画したルーラーのキャレット位置 2002.02.25 Add By KK  2007.08.26 kobake 名前変更
	int		m_nOldRulerWidth;	// 前回描画したルーラーのキャレット幅   2002.02.25 Add By KK  2007.08.26 kobake 名前変更
};
