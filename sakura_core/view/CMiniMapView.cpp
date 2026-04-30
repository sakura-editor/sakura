/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

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

void CMiniMapView::OnSetFocus()
{
	// no operation, CEditミニマップは CEditView と振る舞いを変える。
}

void CMiniMapView::OnKillFocus()
{
	// no operation, CEditミニマップは CEditView と振る舞いを変える。
}

void CMiniMapView::OnLBUTTONDBLCLK([[maybe_unused]] WPARAM fwKeys, [[maybe_unused]] int _xPos, [[maybe_unused]] int _yPos)
{
	// no operation, CEditミニマップは CEditView と振る舞いを変える。
}

void CMiniMapView::SetFont(HWND hWnd)
{
	using WindowDcHolder = cxx::ResourceHolder<&::ReleaseDC>;
	WindowDcHolder hdc{ hWnd };
	hdc = ::GetDC(hWnd);

	GetTextMetrics().Update(hdc, GetFontset().GetFontHan(), 0, 0);

	GetTextArea().UpdateAreaMetrics();
	GetTextArea().DetectWidthOfLineNumberArea(false);

	::InvalidateRect(hWnd, nullptr, TRUE);
}
