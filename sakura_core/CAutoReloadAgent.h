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
#ifndef SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_
#define SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_

#include "doc/CDocListener.h"

// �t�@�C�����X�V���ꂽ�ꍇ�ɍēǍ����s�����ǂ����̃t���O
enum WatchUpdate {
	WU_QUERY,	//!< �ēǍ����s�����ǂ����_�C�A���O�{�b�N�X�Ŗ₢���킹��
	WU_NOTIFY,	//!< �X�V���ꂽ���Ƃ��X�e�[�^�X�o�[�Œʒm
	WU_NONE,	//!< �X�V�Ď����s��Ȃ�
	WU_AUTOLOAD,//!< �X�V���ꖢ�ҏW�̏ꍇ�ɍă��[�h
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
	int m_nPauseCount;	//���ꂪ1�ȏ�̏ꍇ�͊Ď������Ȃ�
	int m_nDelayCount;	//���ҏW�ōă��[�h���̒x���J�E���^
};

#endif /* SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_ */
/*[EOF]*/
