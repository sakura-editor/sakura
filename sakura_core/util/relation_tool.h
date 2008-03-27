/*
	�֘A�̊Ǘ�
*/
#pragma once

#include <vector>
class CSubject;
class CListener;

//! ������CListener����E�H�b�`�����
class CSubject{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CSubject();
	virtual ~CSubject();

	//���J�C���^�[�t�F�[�X
	int GetListenerCount() const{ return (int)m_vListenersRef.size(); }
	CListener* GetListener(int nIndex) const{ return m_vListenersRef[nIndex]; }

public:
	//�Ǘ��p
	void _AddListener(CListener* pcListener);
	void _RemoveListener(CListener* pcListener);

private:
	std::vector<CListener*> m_vListenersRef;
};

//! 1��CSubject���E�H�b�`����
class CListener{
public:
	CListener();
	virtual ~CListener();

	//���J�C���^�[�t�F�[�X
	CSubject* Listen(CSubject* pcSubject); //!< ���O�ɃE�H�b�`���Ă����T�u�W�F�N�g��Ԃ�
	CSubject* GetListeningSubject() const{ return m_pcSubjectRef; }

private:
	CSubject* m_pcSubjectRef;
};




template <class LISTENER> class CSubjectT : public CSubject{
public:
	LISTENER* GetListener(int nIndex) const
	{
		return static_cast<LISTENER*>(CSubject::GetListener(nIndex));
	}
};

template <class SUBJECT> class CListenerT : public CListener{
public:
	SUBJECT* Listen(SUBJECT* pcSubject)
	{
		return static_cast<SUBJECT*>(CListener::Listen(static_cast<CSubject*>(pcSubject)));
	}
	SUBJECT* GetListeningSubject() const
	{
		return static_cast<SUBJECT*>(CListener::GetListeningSubject());
	}
};
