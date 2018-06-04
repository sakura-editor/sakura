#include "StdAfx.h"
#include "build_config.h"
#include <stdlib.h> //malloc,free

//デバッグ検証用：newされた領域をわざと汚す。2007.11.27 kobake
#ifdef FILL_STRANGE_IN_NEW_MEMORY

inline void _fill_new_memory(void* p, size_t nSize, const char* pSrc, size_t nSrcLen)
{
	char* s = (char*)p;
	size_t i;
	for (i = 0; i < nSize; i++)
	{
		*s++ = pSrc[i%nSrcLen];
	}
}

void* operator new(size_t nSize)
{
	void* p = ::malloc(nSize);
	_fill_new_memory(p, nSize, "n_e_w_!_", 8); //確保されたばかりのメモリ状態は「n_e_w_!_....」となります
	return p;
}

#ifdef _MSC_VER
#if _MSC_VER == 1500
_Ret_bytecap_(_Size)	// for VS2008 Debug mode
#endif
#endif

void* operator new[](size_t nSize)
{
	void* p = ::malloc(nSize);
	_fill_new_memory(p, nSize, "N_E_W_!_", 8); //確保されたばかりのメモリ状態は「N_E_W_!_N_E_W_!_N_E_W_!_....」となります
	return p;
}

void operator delete(void* p)
{
	::free(p);
}

void operator delete[](void* p)
{
	::free(p);
}

#endif
