//2007.08.25 kobake 追加

#pragma once

#include <vector>

class CTextMetrics{
public:
	//コンストラクタ・デストラクタ
	CTextMetrics();
	virtual ~CTextMetrics();
	void CopyTextMetricsStatus(CTextMetrics* pDst) const;
	void Update(HFONT hFont);

	//設定
	void SetHankakuWidth(int nHankakuWidth);   //!< 半角文字の幅を設定。単位はピクセル。
	void SetHankakuHeight(int nHankakuHeight); //!< 半角文字の縦幅を設定。単位はピクセル。
	void SetHankakuDx(int nHankakuDx);         //!< 半角文字の文字間隔を設定。単位はピクセル。
	void SetHankakuDy(int nHankakuDy);         //!< 半角文字の行間隔を設定。単位はピクセル。

	//取得
	int GetHankakuWidth() const{ return m_nCharWidth; }		//!< 半角文字の横幅を取得。単位はピクセル。
	int GetHankakuHeight() const{ return m_nCharHeight; }	//!< 半角文字の縦幅を取得。単位はピクセル。
	int GetHankakuDx() const{ return m_nDxBasis; }			//!< 半角文字の文字間隔を取得。単位はピクセル。
	int GetZenkakuDx() const{ return m_nDxBasis*2; }		//!< 全角文字の文字間隔を取得。単位はピクセル。
	int GetHankakuDy() const{ return m_nDyBasis; }			//!< Y方向文字間隔。文字縦幅＋行間隔。単位はピクセル。

	//文字間隔配列を取得
	const int* GetDxArray_AllHankaku() const{ return m_anHankakuDx; } //!<半角文字列の文字間隔配列を取得。要素数は64。
	const int* GetDxArray_AllZenkaku() const{ return m_anZenkakuDx; } //!<半角文字列の文字間隔配列を取得。要素数は64。

	//! 指定した文字列により文字間隔配列を生成する。
	static const int* GenerateDxArray(
		std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
		const wchar_t* pText,           //!< [in]  文字列
		int nLength,                    //!< [in]  文字列長
		int nHankakuDx                  //!< [in]  半角文字の文字間隔
	);

	//!文字列のピクセル幅を返す。
	static int CalcTextWidth(
		const wchar_t* pText, //!< 文字列
		int nLength,          //!< 文字列長
		const int* pnDx       //!< 文字間隔の入った配列
	);

	//!文字列のピクセル幅を返す。
	static int CalcTextWidth2(
		const wchar_t* pText, //!< 文字列
		int nLength,          //!< 文字列長
		int nHankakuDx        //!< 半角文字の文字間隔
	);

private:
//	HDC m_hdc; //!< 計算に用いるデバイスコンテキスト
	int	m_nCharWidth;      //!< 半角文字の横幅
	int m_nCharHeight;     //!< 半角文字の縦幅
	int m_nDxBasis;        //!< 半角文字の文字間隔 (横幅+α)
	int m_nDyBasis;        //!< 半角文字の行間隔 (縦幅+α)
	int m_anHankakuDx[64]; //!< 半角用文字間隔配列
	int m_anZenkakuDx[64]; //!< 全角用文字間隔配列
};



