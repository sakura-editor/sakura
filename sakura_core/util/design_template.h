/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/

#pragma once

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

