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

