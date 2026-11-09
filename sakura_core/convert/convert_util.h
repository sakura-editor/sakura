/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CONVERT_UTIL_277AEF33_C47A_4760_A892_04C57763274E_H_
#define SAKURA_CONVERT_UTIL_277AEF33_C47A_4760_A892_04C57763274E_H_
#pragma once

//2007.10.18 kobake

//! 全角ひらがな→全角カタカナ (文字数は不変)
void Convert_ZenhiraToZenkata(WCHAR* pData, int nLength);

//! 全角カタカナ→全角ひらがな (文字数は不変)
void Convert_ZenkataToZenhira(WCHAR* pData, int nLength);

//! 全角英数→半角英数 (文字数は不変)
void Convert_ZeneisuToHaneisu(WCHAR* pData, int nLength);

/*!
	全角カタカナ→半角カタカナ
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ZenkataToHankata(const WCHAR* pSrc, int nSrcLength, WCHAR* pDst, int* nDstLength);

/*!
	全角→半角
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ToHankaku(const WCHAR* pSrc, int nSrcLength, WCHAR* pDst, int* nDstLength);

//! 半角英数→全角英数 (文字数は不変)
void Convert_HaneisuToZeneisu(WCHAR* pData, int nLength);

/*!
	半角カタカナ→全角カタカナ
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenkata(const WCHAR* pSrc, int nSrcLength, WCHAR* pDst, int* nDstLength);

/*!
	半角カタカナ→全角ひらがな
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenhira(const WCHAR* pSrc, int nSrcLength, WCHAR* pDst, int* nDstLength);
#endif /* SAKURA_CONVERT_UTIL_277AEF33_C47A_4760_A892_04C57763274E_H_ */
