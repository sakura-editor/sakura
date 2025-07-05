/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "relation_tool.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CSubject                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CSubject::CSubject()
{
}

CSubject::~CSubject()
{
	//リスナを解除
	for(int i=0;i<(int)m_vListenersRef.size();i++){
		m_vListenersRef[i]->Listen(nullptr);
	}
	m_vListenersRef.clear();
}

void CSubject::_AddListener(CListener* pcListener)
{
	//既に追加済みなら何もしない
	for(int i=0;i<(int)m_vListenersRef.size();i++){
		if(m_vListenersRef[i]==pcListener){
			return;
		}
	}
	//追加
	m_vListenersRef.push_back(pcListener);
}

void CSubject::_RemoveListener(CListener* pcListener)
{
	//配列から削除
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
: m_pcSubjectRef(nullptr)
{
}

CListener::~CListener()
{
	Listen(nullptr);
}

CSubject* CListener::Listen(CSubject* pcSubject)
{
	CSubject* pOld = GetListeningSubject();

	//古いサブジェクトを解除
	if(m_pcSubjectRef){
		m_pcSubjectRef->_RemoveListener(this);
		m_pcSubjectRef = nullptr;
	}

	//新しく設定
	m_pcSubjectRef = pcSubject;
	if(m_pcSubjectRef){
		m_pcSubjectRef->_AddListener(this);
	}

	return pOld;
}
