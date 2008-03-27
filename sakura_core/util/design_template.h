/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/

#pragma once

/*!
	Singletonパターン

	2008.03.03 kobake 作成
*/
template <class T>
class TSingleton{
public:
	//公開インターフェース
	static T* Instance()
	{
		if(!gm_instance)
			gm_instance = new T;
		return gm_instance;
	}

protected:
	//※2個以上のインスタンスは想定していません。assertが破綻を検出します。
	TSingleton(){ assert(gm_instance==NULL); gm_instance=static_cast<T*>(this); }
	~TSingleton(){ assert(gm_instance); gm_instance=NULL; }

private:
	static T* gm_instance;
};
template <class T>
T* TSingleton<T>::gm_instance = NULL;


/*!
	1個しかインスタンスが存在しないクラスからのインスタンス取得インターフェースをstaticで提供。
	Singletonパターンとは異なり、Instance()呼び出しにより、インスタンスが自動生成されない点に注意。

	2007.10.23 kobake 作成
*/
template <class T>
class TSingleInstance{
public:
	//公開インターフェース
	static T* Instance(){ return gm_instance; } //!< 作成済みのインスタンスを返す。インスタンスが存在しなければ NULL。

protected:
	//※2個以上のインスタンスは想定していません。assertが破綻を検出します。
	TSingleInstance(){ assert(gm_instance==NULL); gm_instance=static_cast<T*>(this); }
	~TSingleInstance(){ assert(gm_instance); gm_instance=NULL; }
private:
	static T* gm_instance;
};
template <class T>
T* TSingleInstance<T>::gm_instance = NULL;



//記録もする
#include <vector>
template <class T> class TInstanceHolder{
public:
	TInstanceHolder()
	{
		gm_table.push_back(static_cast<T*>(this));
	}
	virtual ~TInstanceHolder()
	{
		for(size_t i=0;i<gm_table.size();i++){
			if(gm_table[i]==static_cast<T*>(this)){
				gm_table.erase(gm_table.begin()+i);
				break;
			}
		}
	}
	static int GetInstanceCount(){ return (int)gm_table.size(); }
	static T* GetInstance(int nIndex)
	{
		if(nIndex>=0 && nIndex<(int)gm_table.size()){
			return gm_table[nIndex];
		}else{
			return 0;
		}
	}

private:
	static std::vector<T*> gm_table;
};
template <class T> std::vector<T*> TInstanceHolder<T>::gm_table;
