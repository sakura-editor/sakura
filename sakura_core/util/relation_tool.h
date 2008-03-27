/*
	関連の管理
*/
#pragma once

#include <vector>
class CSubject;
class CListener;

//! 複数のCListenerからウォッチされる
class CSubject{
public:
	//コンストラクタ・デストラクタ
	CSubject();
	virtual ~CSubject();

	//公開インターフェース
	int GetListenerCount() const{ return (int)m_vListenersRef.size(); }
	CListener* GetListener(int nIndex) const{ return m_vListenersRef[nIndex]; }

public:
	//管理用
	void _AddListener(CListener* pcListener);
	void _RemoveListener(CListener* pcListener);

private:
	std::vector<CListener*> m_vListenersRef;
};

//! 1つのCSubjectをウォッチする
class CListener{
public:
	CListener();
	virtual ~CListener();

	//公開インターフェース
	CSubject* Listen(CSubject* pcSubject); //!< 直前にウォッチしていたサブジェクトを返す
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
