#include "stdafx.h"
#include "relation_tool.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CSubject                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CSubject::CSubject()
{
}

CSubject::~CSubject()
{
	//���X�i������
	for(int i=0;i<(int)m_vListenersRef.size();i++){
		m_vListenersRef[i]->Listen(NULL);
	}
	m_vListenersRef.clear();
}

void CSubject::_AddListener(CListener* pcListener)
{
	//���ɒǉ��ς݂Ȃ牽�����Ȃ�
	for(int i=0;i<(int)m_vListenersRef.size();i++){
		if(m_vListenersRef[i]==pcListener){
			return;
		}
	}
	//�ǉ�
	m_vListenersRef.push_back(pcListener);
}

void CSubject::_RemoveListener(CListener* pcListener)
{
	//�z�񂩂�폜
	for(int i=0;i<(int)m_vListenersRef.size();i++){
		if(m_vListenersRef[i]==pcListener){
			m_vListenersRef.erase(m_vListenersRef.begin()+i);
			break;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CListener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CListener::CListener()
: m_pcSubjectRef(NULL)
{
}

CListener::~CListener()
{
	Listen(NULL);
}

CSubject* CListener::Listen(CSubject* pcSubject)
{
	CSubject* pOld = GetListeningSubject();

	//�Â��T�u�W�F�N�g������
	if(m_pcSubjectRef){
		m_pcSubjectRef->_RemoveListener(this);
		m_pcSubjectRef = NULL;
	}

	//�V�����ݒ�
	m_pcSubjectRef = pcSubject;
	if(m_pcSubjectRef){
		m_pcSubjectRef->_AddListener(this);
	}

	return pOld;
}

