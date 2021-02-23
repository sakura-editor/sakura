/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "util/tchar_convert.h"
#include "mem/CRecycledBuffer.h"

static CRecycledBuffer        g_bufSmall;
static CRecycledBufferDynamic g_bufBig;

const WCHAR* to_wchar(const ACHAR* src)
{
	if(src==NULL)return NULL;

	return to_wchar(src,strlen(src));
}

const WCHAR* to_wchar(const ACHAR* pSrc, int nSrcLength)
{
	if(pSrc==NULL)return NULL;

	//必要なサイズを計算
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		NULL,
		0
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	WCHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<WCHAR>()){
		pDst=g_bufSmall.GetBuffer<WCHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<WCHAR>(nDstCnt);
	}

	//変換
	nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		pDst,
		nDstLen
	);
	pDst[nDstLen] = L'\0';

	return pDst;
}

const ACHAR* to_achar(const WCHAR* src)
{
	if(src==NULL)return NULL;

	return to_achar(src,wcslen(src));
}

const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength)
{
	if(pSrc==NULL)return NULL;

	//必要なサイズを計算
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		NULL,
		0,
		NULL,
		NULL
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	ACHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<ACHAR>()){
		pDst=g_bufSmall.GetBuffer<ACHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<ACHAR>(nDstCnt);
	}

	//変換
	nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		pDst,
		nDstLen,
		NULL,
		NULL
	);
	pDst[nDstLen] = '\0';

	return pDst;
}
