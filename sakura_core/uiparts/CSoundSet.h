/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSOUNDSET_2EB52973_40E4_438F_A670_6FFA850251E6_H_
#define SAKURA_CSOUNDSET_2EB52973_40E4_438F_A670_6FFA850251E6_H_
#pragma once

class CSoundSet{
public:
	CSoundSet() : m_nMuteCount(0) { }
	void NeedlessToSaveBeep(); //上書き不要ビープ音
	void MuteOn(){ m_nMuteCount++; }
	void MuteOff(){ m_nMuteCount--; }
private:
	int	m_nMuteCount;
};
#endif /* SAKURA_CSOUNDSET_2EB52973_40E4_438F_A670_6FFA850251E6_H_ */
