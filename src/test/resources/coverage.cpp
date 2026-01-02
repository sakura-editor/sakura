/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include <CodeCoverage/CodeCoverage.h>

// Exclude all the code from a particular files:
// see https://docs.microsoft.com/ja-jp/visualstudio/test/using-code-coverage-to-determine-how-much-code-is-being-tested
ExcludeSourceFromCodeCoverage(Exclusion1, L"*\\build\\*");
ExcludeSourceFromCodeCoverage(Exclusion2, L"*\\src\\test\\cpp\\*");
ExcludeSourceFromCodeCoverage(Exclusion3, L"*\\Windows Kits\\10\\Include\\*");
ExcludeSourceFromCodeCoverage(Exclusion4, L"*\\VC\\Tools\\MSVC\\*\\include\\*");
ExcludeSourceFromCodeCoverage(Exclusion5, L"*\\externals\\*");
