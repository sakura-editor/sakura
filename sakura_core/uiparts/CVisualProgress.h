/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
