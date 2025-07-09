/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/tchar_convert.h"
#include "mem/CRecycledBuffer.h"
#include "charset/charcode.h"

static CRecycledBuffer        g_bufSmall;
static CRecycledBufferDynamic g_bufBig;

const WCHAR* to_wchar(const ACHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_wchar(src,strlen(src));
}

const WCHAR* to_wchar(const ACHAR* pSrc, int nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	//必要なサイズを計算
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		nullptr,
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
	if(src==nullptr)return nullptr;

	return to_achar(src,wcslen(src));
}

const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	//必要なサイズを計算
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		nullptr,
		0,
		nullptr,
		nullptr
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
		nullptr,
		nullptr
	);
	pDst[nDstLen] = '\0';

	return pDst;
}
