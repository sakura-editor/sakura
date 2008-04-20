#pragma once

#include "doc/CDocListener.h"

// �t�@�C�����X�V���ꂽ�ꍇ�ɍēǍ����s�����ǂ����̃t���O
enum WatchUpdate {
	WU_QUERY,	//!< �ēǍ����s�����ǂ����_�C�A���O�{�b�N�X�Ŗ₢���킹��
	WU_NOTIFY,	//!< �X�V���ꂽ���Ƃ��X�e�[�^�X�o�[�Œʒm
	WU_NONE		//!< �X�V�Ď����s��Ȃ�
};

class CAutoReloadAgent : public CDocListenerEx{
public:
	CAutoReloadAgent();
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Ď��̈ꎞ��~
	void PauseWatching(){ m_nPauseCount++; }
	void ResumeWatching(){ m_nPauseCount--; assert(m_nPauseCount>=0); }
	bool IsPausing() const{ return m_nPauseCount>=1; }

public://#####��
	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����

public:
	WatchUpdate		m_eWatchUpdate;	//!< �X�V�Ď����@

private:
	int m_nPauseCount; //���ꂪ1�ȏ�̏ꍇ�͊Ď������Ȃ�
};
