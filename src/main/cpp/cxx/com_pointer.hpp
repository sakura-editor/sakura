/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#ifdef __MINGW32__
#  include "cxx/TComPtr.hpp"
#endif // __MINGW32__

namespace cxx {

#ifdef _MSC_VER
template<typename TargetInterface>
using com_pointer = _COM_SMARTPTR<_COM_SMARTPTR_LEVEL2<TargetInterface, &__uuidof(TargetInterface)>>;
#endif // _MSC_VER

#ifdef __MINGW32__
template<typename TargetInterface>
using com_pointer = TComPtr<TargetInterface>;
#endif // __MINGW32__

} // namespace cxx

#define DEFINE_COM_SMARTPTR(type) using type##Ptr = cxx::com_pointer<type>
