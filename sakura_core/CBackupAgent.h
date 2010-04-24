#pragma once

#include "doc/CDocListener.h"

class CBackupAgent : public CDocListenerEx{
public:
	ECallbackResult OnPreBeforeSave(SSaveInfo* pSaveInfo);

protected:
	int MakeBackUp( const TCHAR* target_file );								//!< バックアップの作成
	bool FormatBackUpPath( TCHAR*, size_t, const TCHAR* );	//!< バックアップパスの作成 2005.11.21 aroka
};
