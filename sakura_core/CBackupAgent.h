#pragma once

#include "CDocListener.h"

class CBackupAgent : public CDocListenerEx{
public:
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);

protected:
	int MakeBackUp( const TCHAR* target_file );								//!< �o�b�N�A�b�v�̍쐬
	bool FormatBackUpPath( TCHAR* szNewPath, const TCHAR* target_file );	//!< �o�b�N�A�b�v�p�X�̍쐬 2005.11.21 aroka
};
