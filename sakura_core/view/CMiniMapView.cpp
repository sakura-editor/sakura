/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "view/CMiniMapView.h"

#include "doc/CEditDoc.h"

BOOL CMiniMapView::Create( HWND hWndParent )
{
	auto pcEditDoc = CEditDoc::getInstance();
	return CEditView::Create( hWndParent, pcEditDoc, -1, FALSE, true );
}