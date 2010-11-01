//2007.10.18 kobake

#pragma once


//! 全角ひらがな→全角カタカナ (文字数は不変)
void Convert_ZenhiraToZenkata(wchar_t* pData, int nLength);

//! 全角カタカナ→全角ひらがな (文字数は不変)
void Convert_ZenkataToZenhira(wchar_t* pData, int nLength);

//! 全角英数→半角英数 (文字数は不変)
void Convert_ZeneisuToHaneisu(wchar_t* pData, int nLength);

/*!
	全角カタカナ→半角カタカナ
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ZenkataToHankata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);

/*!
	全角→半角
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ToHankaku(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);


//! 半角英数→全角英数 (文字数は不変)
void Convert_HaneisuToZeneisu(wchar_t* pData, int nLength);

/*!
	半角カタカナ→全角カタカナ
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenkata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);

/*!
	半角カタカナ→全角ひらがな
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenhira(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);
