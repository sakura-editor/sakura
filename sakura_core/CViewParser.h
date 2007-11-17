#pragma once

class CEditView;

//!品詞解析クラス
class CViewParser{
public:
	CViewParser(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CViewParser(){}

	//! カーソル直前の単語を取得
	int GetLeftWord( CNativeW* pcmemWord, int nMaxWordLen ) const;

	//! キャレット位置の単語を取得
	// 2006.03.24 fon
	BOOL GetCurrentWord( CNativeW* pcmemWord ) const;

private:
	const CEditView* m_pEditView;
};
