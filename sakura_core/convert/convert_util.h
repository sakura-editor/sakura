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
#ifndef SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_
#define SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_

//2007.10.18 kobake

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

#endif /* SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_ */
/*[EOF]*/
