/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

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
private:
	using Base = CEditView;
	using Me = CMiniMapView;

public:
	//コンストラクタは流用する
	using Base::Base;

	BOOL Create( HWND hWndParent );

	void	SetFont(HWND hWnd) override;

	void	OnKillFocus() override;
	void	OnLBUTTONDBLCLK(WPARAM fwKeys, int _xPos, int _yPos) override;
	void	OnSetFocus() override;
};
