/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include "view/CEditView.h"

/*!
	ミニマップ

	編集ビューのコードを流用して縮小ビューを表示する
 */
class CMiniMapView : public CEditView
{
public:
	BOOL Create( HWND hWndParent );
};
