/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CBACKUPAGENT_54267C70_F49D_418B_B1EA_0F98DD5DE4B19_H_
#define SAKURA_CBACKUPAGENT_54267C70_F49D_418B_B1EA_0F98DD5DE4B19_H_

#include "doc/CDocListener.h"

class CBackupAgent : public CDocListenerEx{
public:
	ECallbackResult OnPreBeforeSave(SSaveInfo* pSaveInfo);

protected:
	int MakeBackUp( const TCHAR* target_file );								//!< バックアップの作成
	bool FormatBackUpPath( TCHAR*, size_t, const TCHAR* );	//!< バックアップパスの作成 2005.11.21 aroka
};

#endif /* SAKURA_CBACKUPAGENT_54267C70_F49D_418B_B1EA_0F98DD5DE4B19_H_ */
/*[EOF]*/
