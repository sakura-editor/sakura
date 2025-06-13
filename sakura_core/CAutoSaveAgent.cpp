/*! @file
	@brief ファイルの自動保存

	@author genta
	@date 2000
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib

*/
#include "StdAfx.h"
#include "CAutoSaveAgent.h"
#include "doc/CEditDoc.h"
#include "env/DLLSHAREDATA.h"

//	From Here Aug. 21, 2000 genta
//
//	自動保存を行うかどうかのチェック
//
void CAutoSaveAgent::CheckAutoSave()
{
	if( m_cPassiveTimer.CheckAction() ){
		CEditDoc* pcDoc = GetListeningDoc();

		//	上書き保存

		if( !pcDoc->m_cDocEditor.IsModified() )	//	変更無しなら何もしない
			return;				//	ここでは，「無変更でも保存」は無視する

		//	2003.10.09 zenryaku 保存失敗エラーの抑制
		if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() )	//	まだファイル名が設定されていなければ保存しない
			return;

		bool en = m_cPassiveTimer.IsEnabled();
		m_cPassiveTimer.Enable(false);	//	2重呼び出しを防ぐため
		pcDoc->m_cDocFileOperation.FileSave();	//	保存
		m_cPassiveTimer.Enable(en);
	}
}

//
//	設定変更を自動保存動作に反映する
//
void CAutoSaveAgent::ReloadAutoSaveParam()
{
	m_cPassiveTimer.SetInterval( GetDllShareData().m_Common.m_sBackup.GetAutoBackupInterval() );
	m_cPassiveTimer.Enable( GetDllShareData().m_Common.m_sBackup.IsAutoBackupEnabled() );
}

//----------------------------------------------------------
//	class CPassiveTimer
//
//----------------------------------------------------------
/*!
	時間間隔の設定
	@param m 間隔(min)
	間隔を0以下に設定したときは1秒とみなす。設定可能な最大間隔は35791分。
*/
void CPassiveTimer::SetInterval(int m)
{
	constexpr int nMaxInterval = INT_MAX / MSec2Min;
	static_assert(nMaxInterval == 35791);

	m = std::clamp(m, 1, nMaxInterval);
	nInterval = m * MSec2Min;
}
/*!
	タイマーの有効・無効の切り替え
	@param flag true:有効 / false: 無効
	無効→有効に切り替えたときはリセットされる。
*/
void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	変更があるとき
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}
/*!
	外部で定期に実行されるところから呼び出される関数。
	呼び出されると経過時間をチェックする。

	@retval true 所定時間が経過した。このときは測定基準が自動的にリセットされる。
	@retval false 所定の時間に達していない。
*/
bool CPassiveTimer::CheckAction(void)
{
	if( !IsEnabled() )	//	有効でなければ何もしない
		return false;

	//	時刻比較
	DWORD now = ::GetTickCount();
	int diff;

	diff = now - nLastTick;	//	TickCountが一回りしてもこれでうまくいくはず...

	if( diff < nInterval )	//	規定時間に達していない
		return false;

	Reset();
	return true;
}
