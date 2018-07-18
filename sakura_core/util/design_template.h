/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/
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
#ifndef SAKURA_DESIGN_TEMPLATE_8F7F7545_B66E_47C3_AE3A_0E406B3A0B0B_H_
#define SAKURA_DESIGN_TEMPLATE_8F7F7545_B66E_47C3_AE3A_0E406B3A0B0B_H_

// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/*!
	Singletonパターン

	2008.03.03 kobake 作成
*/
template <class T>
class TSingleton{
public:
	//公開インターフェース
	static T* getInstance()
	{
		static T instance;
		return &instance;
	}

protected:
	TSingleton(){}
private:
	DISALLOW_COPY_AND_ASSIGN(TSingleton);
};

/*!
	1個しかインスタンスが存在しないクラスからのインスタンス取得インターフェースをstaticで提供。
	Singletonパターンとは異なり、Instance()呼び出しにより、インスタンスが自動生成されない点に注意。

	2007.10.23 kobake 作成
*/
template <class T>
class TSingleInstance{
public:
	//公開インターフェース
	static T* getInstance(){ return gm_instance; } //!< 作成済みのインスタンスを返す。インスタンスが存在しなければ NULL。

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

#endif /* SAKURA_DESIGN_TEMPLATE_8F7F7545_B66E_47C3_AE3A_0E406B3A0B0B_H_ */
/*[EOF]*/
