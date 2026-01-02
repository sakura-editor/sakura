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

CSubject::CSubject() = default;

CSubject::~CSubject()
{
	//リスナを解除
	while (!m_vListenersRef.empty()) {
		_RemoveListener(m_vListenersRef.back());
	}
}

/*!
 * リスナーを追加する
 */
void CSubject::_AddListener(CListener* pcListener)
{
	//NULLは追加できない
	assert(pcListener);

	//既に追加済みなら何もしない
	if (auto found = std::find(m_vListenersRef.cbegin(), m_vListenersRef.cend(), pcListener); found != m_vListenersRef.cend()) {
		return;
	}

	//追加
	m_vListenersRef.push_back(pcListener);
}

/*!
 * リスナーを削除する
 */
void CSubject::_RemoveListener(CListener* pcListener) noexcept
{
	//NULLは追加できないので、削除も想定しない
	assert(pcListener);

	//削除前のリスナー数を記録
	const auto llisteners = m_vListenersRef.size();

	//配列から削除
	if (auto found = std::find(m_vListenersRef.begin(), m_vListenersRef.end(), pcListener); found != m_vListenersRef.end()) {
		//確実に解除するため、リスナー側の終了メソッドを呼び出す
		pcListener->_EndListen();

		//リスナー数が代わってないときだけ削除に進む
		if (llisteners == m_vListenersRef.size()) {
			//リスナー配列から削除
			m_vListenersRef.erase(found);
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CListener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CListener::CListener() = default;

CListener::~CListener()
{
	_EndListen();
}

/*!
 * 監視を開始する
 */
CSubject* CListener::Listen(
	CSubject* pcSubject	//!< [in] 監視するサブジェクト。NULLで監視終了
)
{
	//操作前の参照を取得
	auto pOld = GetListeningSubject();

	//古いサブジェクトを解放
	_EndListen();

	//新しく設定
	if (pcSubject) {
		//サブジェクト側にリスナー追加を要求する
		pcSubject->_AddListener(this);

		//参照を保存
		m_pcSubjectRef = pcSubject;
	}

	return pOld;
}

/*!
 * 監視を終了する
 */
void CListener::_EndListen() noexcept
{
	if (auto pcSubject = m_pcSubjectRef) {
		//構造的に再帰呼出なので、先に参照を消しておく
		m_pcSubjectRef = nullptr;

		//サブジェクト側にリスナー削除を要求する
		pcSubject->_RemoveListener(this);
	}
}
