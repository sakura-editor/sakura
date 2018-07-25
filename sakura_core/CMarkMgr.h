//
/*!	@file
	@brief 現在行のマークを管理する

	@author genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka

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

#ifndef __CMARKMGR_H_
#define __CMARKMGR_H_

#include <vector>


/*!
	行マークを管理するクラス。
	純粋仮想関数を含むので、実際にはサブクラスを作って使う。

	@par 通常操作（共通）
	Add()で追加．場所と名前を登録できる．操作そのものはカスタマイズ可能．
	[番号]で該当番号の要素を取得できる．

	@par 内部動作
	最大値を超えた場合はprotectedな関数で処理する．（カスタマイズ可能）
	Add()の処理はサブクラスに任せる．

	@par 現在位置の管理
	現在位置はManager内で管理する．

	削除操作はサブクラスにまかせる

*/
class CMarkMgr {
public:

	//	項目のクラス
	class CMark {
	public:
		//	constructor
		CMark( const CLogicPoint& pt ) : m_ptLogic(pt) { }

		CLogicPoint GetPosition() const { return m_ptLogic; }
		void SetPosition(const CLogicPoint& pt) { m_ptLogic = pt; }

		bool IsValid(void) const { return true; }

		bool operator==(CMark &r) const { return m_ptLogic.y == r.m_ptLogic.y; }
		bool operator!=(CMark &r) const { return m_ptLogic.y != r.m_ptLogic.y; }

	private:
		CLogicPoint m_ptLogic;
	};

	// GENERATE_FACTORY(CMark,CMarkFactory);	//	CMark用Factory class

	//	型宣言
	typedef std::vector<CMark> CMarkChain;
	typedef std::vector<CMark>::const_iterator	CMarkIterator;

	//	Interface
	//	constructor
	CMarkMgr() : m_nCurpos(0), m_nMaxitem(10){}
	// CMarkMgr(const CDocLineMgr *p) : doc(p) {}

	int Count(void) const { return (int)m_cMarkChain.size(); }	//!<	項目数を返す
	int GetMax(void) const { return m_nMaxitem; }	//!<	最大項目数を返す
	void SetMax(int max);	//!<	最大項目数を設定

	virtual void Add(const CMark& m) = 0;	//!<	要素の追加

	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//!<	要素の全消去

	//!	要素の取得
	const CMark& GetCurrent(void) const { return m_cMarkChain[m_nCurpos]; }

	//	有効性の確認
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;

	//	現在位置の移動
	bool NextValid(void);
	bool PrevValid(void);

	const CMark& operator[](int index) const { return m_cMarkChain[index]; }

	//	連続取得インターフェース
//	CMarkIterator CurrentPos(void) const { return (CMarkIterator)m_cMarkChain.begin() + m_nCurpos; }
//	CMarkIterator Begin(void) const { return (CMarkIterator)m_cMarkChain.begin(); }
//	CMarkIterator End(void) const { return (CMarkIterator)m_cMarkChain.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory m_factory;	//	Factory Class (マクロで生成される）
	CMarkChain m_cMarkChain;	//	マークデータ本体
	int m_nCurpos;	//	現在位置（番号）

	int m_nMaxitem;	//	保管可能アイテムの最大数
private:
	//CMarkMgr( const CMarkMgr& );	//	Copy禁止

};

// ----------------------------------------------------
/*!
	@brief 移動履歴の管理クラス

	CMarkMgr を継承し、動作が規定されていない部分を実装する。
*/
class CAutoMarkMgr : public CMarkMgr{
public:
	virtual void Add(const CMark& m);	//!<	要素の追加
	virtual void Expire(void);	//!<	要素数の調整
};

#endif



