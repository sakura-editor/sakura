﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CVISUALPROGRESS_A9390FAB_E0F3_4EA2_8A6C_1ACB3143DD3F_H_
#define SAKURA_CVISUALPROGRESS_A9390FAB_E0F3_4EA2_8A6C_1ACB3143DD3F_H_
#pragma once

#include "doc/CDocListener.h"
#include "util/design_template.h"
class CWaitCursor;

class CVisualProgress final : public CDocListenerEx, public CProgressListener{
public:
	//コンストラクタ・デストラクタ
	CVisualProgress();
	virtual ~CVisualProgress();

	//ロード前後
	void OnBeforeLoad(SLoadInfo* sLoadInfo) override;
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;

	//セーブ前後
	void OnBeforeSave(const SSaveInfo& sSaveInfo) override;
	void OnFinalSave(ESaveResult eSaveResult) override;

	//プログレス受信
	void OnProgress(int nPer) override;

protected:
	//実装補助
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	CWaitCursor* m_pcWaitCursor;
	int	nOldValue;

	DISALLOW_COPY_AND_ASSIGN(CVisualProgress);
};
#endif /* SAKURA_CVISUALPROGRESS_A9390FAB_E0F3_4EA2_8A6C_1ACB3143DD3F_H_ */
