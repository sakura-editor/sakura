/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CSoundSet.h"
#include "util/MessageBoxF.h"

void CSoundSet::NeedlessToSaveBeep()
{
	if( m_nMuteCount>=1 )return;
	ErrorBeep();
}
