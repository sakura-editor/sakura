#pragma once

class CSoundSet{
public:
	CSoundSet() : m_nMuteCount(0) { }
	void NeedlessToSaveBeep(); //�㏑���s�v�r�[�v��
	void MuteOn(){ m_nMuteCount++; }
	void MuteOff(){ m_nMuteCount--; }
private:
	int	m_nMuteCount;
};

