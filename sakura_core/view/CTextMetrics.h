/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_
#define SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_

//2007.08.25 kobake 追加

#include <vector>

class CTextMetrics;

class CTextMetrics{
public:
	//コンストラクタ・デストラクタ
	CTextMetrics();
	virtual ~CTextMetrics();
	void CopyTextMetricsStatus(CTextMetrics* pDst) const;
	void Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace);

	//設定
private:
	void SetHankakuWidth(int nHankakuWidth);   //!< 半角文字の幅を設定。単位はピクセル。
	void SetHankakuHeight(int nHankakuHeight); //!< 半角文字の縦幅を設定。単位はピクセル。
	void SetHankakuDx(int nHankakuDx);         //!< 半角文字の文字間隔を設定。単位はピクセル。
	void SetHankakuDy(int nHankakuDy);         //!< 半角文字の行間隔を設定。単位はピクセル。

	//取得
public:
	int GetHankakuWidth() const{ return m_nCharWidth; }		//!< 半角文字の横幅を取得。単位はピクセル。
	int GetHankakuHeight() const{ return m_nCharHeight; }	//!< 半角文字の縦幅を取得。単位はピクセル。
	int GetHankakuDx() const{ return m_nDxBasis; }			//!< 半角文字の文字間隔を取得。単位はピクセル。
	int GetZenkakuDx() const{ return m_nDxBasis*2; }		//!< 全角文字の文字間隔を取得。単位はピクセル。
	int GetHankakuDy() const{ return m_nDyBasis; }			//!< Y方向文字間隔。文字縦幅＋行間隔。単位はピクセル。

	CPixelXInt GetCharSpacing() const {
		return GetHankakuDx() - GetHankakuWidth();
	}
	// レイアウト幅分のピクセル幅を取得する
	int GetCharPxWidth(CLayoutXInt col) const{
		return (Int)col;
	}

	int GetCharPxWidth() const{
		return 1;
	}
	int GetCharHeightMarginByFontNo(int n) const{
		return m_aFontHeightMargin[n];
	}

	// 固定文字x桁のレイアウト幅を取得する
	CLayoutXInt GetLayoutXDefault(CKetaXInt chars) const{
		return CLayoutXInt(GetHankakuDx() * (Int)chars);
	}
	// 固定文字1桁あたりのレイアウト幅を取得する
	CLayoutXInt GetLayoutXDefault() const{ return GetLayoutXDefault(CKetaXInt(1));}
	

	//文字間隔配列を取得
	const int* GetDxArray_AllHankaku() const{ return m_anHankakuDx; } //!<半角文字列の文字間隔配列を取得。要素数は64。
	const int* GetDxArray_AllZenkaku() const{ return m_anZenkakuDx; } //!<半角文字列の文字間隔配列を取得。要素数は64。

	const int* GenerateDxArray2(
		std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
		const wchar_t* pText,           //!< [in]  文字列
		int nLength                     //!< [in]  文字列長
	) const {
		return GenerateDxArray(vResultArray, pText, nLength, GetHankakuDx(), 8, 0, GetCharSpacing());
	}

	//! 指定した文字列により文字間隔配列を生成する。
	static const int* GenerateDxArray(
		std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
		const wchar_t* pText,           //!< [in]  文字列
		int nLength,                    //!< [in]  文字列長
		int	nHankakuDx,					//!< [in]  半角文字の文字間隔
		int	nTabSpace = 8,				//   [in]  TAB幅
		int	nIndent = 0,				//   [in]  インデント
		int nCharSpacing = 0			//   [in]  文字の間隔
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
		int nHankakuDx,       //!< 半角文字の文字間隔
		int nCharSpacing
	);

	int CalcTextWidth3(
		const wchar_t* pText, //!< 文字列
		int nLength           //!< 文字列長
	) const;

private:
//	HDC m_hdc; //!< 計算に用いるデバイスコンテキスト
	int	m_nCharWidth;      //!< 半角文字の横幅
	int m_nCharHeight;     //!< 半角文字の縦幅
	int m_nDxBasis;        //!< 半角文字の文字間隔 (横幅+α)
	int m_nDyBasis;        //!< 半角文字の行間隔 (縦幅+α)
	int m_anHankakuDx[64]; //!< 半角用文字間隔配列
	int m_anZenkakuDx[64]; //!< 全角用文字間隔配列
	std::vector<int> m_aFontHeightMargin;
};

#endif /* SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_ */
/*[EOF]*/
