/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "apiwrap/HGlobal.hpp"

namespace apiwrap {

/*!
 * コンストラクタ
 */
HGlobal::HGlobal(HGLOBAL hGlobal_, std::shared_ptr<Kernel32Dll> Kernel32Dll_) noexcept
	: Kernel32DllClient(std::move(Kernel32Dll_))
	, _Holder(hGlobal_, HGlobalFreeFunc(GetKernel32Dll()))
{
}

/*!
 * コンストラクタ
 */
HGlobal::HGlobal(UINT flags, SIZE_T bytes, std::shared_ptr<Kernel32Dll> Kernel32Dll_) noexcept
	: HGlobal(Kernel32Dll_->GlobalAlloc(flags, bytes), Kernel32Dll_)
{
}

} //end of namespace apiwrap
