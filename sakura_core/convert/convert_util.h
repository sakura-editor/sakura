//2007.10.18 kobake

#pragma once


//! 全角ひらがな→全角カタカナ (文字数は不変)
void Convert_ZenhiraToZenkana(wchar_t* pData, int nLength);

//! 全角カタカナ→全角ひらがな (文字数は不変)
void Convert_ZenkanaToZenhira(wchar_t* pData, int nLength);

//! 全角英数→半角英数 (文字数は不変)
void Convert_ZenEisuu_To_HanEisuu(wchar_t* pData, int nLength);

/*!
	全角カタカナ→半角カタカナ
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ZenKana_To_HanKana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst);


//! 半角英数→全角英数 (文字数は不変)
void Convert_HanEisuu_To_ZenEisuu(wchar_t* pData, int nLength);

/*!
	半角カタカナ→全角カタカナ
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankanaToZenkana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst);
