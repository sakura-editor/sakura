/*! @file */
/*
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
		m_vListenersRef[i]->Listen(NULL);
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

	//古いサブジェクトを解除
	if(m_pcSubjectRef){
		m_pcSubjectRef->_RemoveListener(this);
		m_pcSubjectRef = NULL;
	}

	//新しく設定
	m_pcSubjectRef = pcSubject;
	if(m_pcSubjectRef){
		m_pcSubjectRef->_AddListener(this);
	}

	return pOld;
}
