/*! @file
	@brief 代码页转换表 macos/platform/data/codepages.bin 的嵌入

	用汇编的 .incbin 把文件原样放进只读数据段。文件路径写在构建时生成的
	codepage_data_path.h 里（SAKURA_CODEPAGES_INCBIN）。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "codepage_data_path.h"

#if defined(__APPLE__)
__asm__(
	".section __TEXT,__const\n"
	".globl _g_sakuraCodePagesData\n"
	".p2align 4\n"
	"_g_sakuraCodePagesData:\n"
	SAKURA_CODEPAGES_INCBIN
	".globl _g_sakuraCodePagesEnd\n"
	"_g_sakuraCodePagesEnd:\n"
	".byte 0\n"
	".text\n");
#else
__asm__(
	".section .rodata\n"
	".globl g_sakuraCodePagesData\n"
	".p2align 4\n"
	"g_sakuraCodePagesData:\n"
	SAKURA_CODEPAGES_INCBIN
	".globl g_sakuraCodePagesEnd\n"
	"g_sakuraCodePagesEnd:\n"
	".byte 0\n"
	".text\n");
#endif
