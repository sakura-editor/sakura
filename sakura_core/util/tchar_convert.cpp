#include "stdafx.h"
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

	//�K�v�ȃT�C�Y���v�Z
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		NULL,
		0
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//�o�b�t�@�擾
	WCHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<WCHAR>()){
		pDst=g_bufSmall.GetBuffer<WCHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<WCHAR>(nDstCnt);
	}

	//�ϊ�
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

	//�K�v�ȃT�C�Y���v�Z
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

	//�o�b�t�@�擾
	ACHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<ACHAR>()){
		pDst=g_bufSmall.GetBuffer<ACHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<ACHAR>(nDstCnt);
	}

	//�ϊ�
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


const WCHAR* easy_format(const WCHAR* format, ...)
{
	WCHAR* buf=g_bufBig.GetBuffer<WCHAR>(1024);
	va_list v;
	va_start(v,format);
	tchar_vswprintf(buf,format,v);
	va_end(v);
	return buf;
}
