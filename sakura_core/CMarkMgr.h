// $Id$
//
/*!	@file
	@brief 現在行のマークを管理する

	@author genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __CMARKMGR_H_
#define __CMARKMGR_H_

#include <vector>
#include <string>
//#include "CStr.h"
//#include "CVectorBase.h"

using namespace std;

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
		CMark( int x, int y ) : m_pos(x), m_line(y), m_extra(0) {}
		CMark( void ) : m_extra(-1) {}
		
		//	type converter
		const char *GetNameStr(void) const { return m_name.c_str(); }
		void SetNameStr(const char* newstr) { m_name = newstr; }
		
		int GetLine(void) const { return m_line; }
		int GetPos(void) const { return m_pos; }
		void SetPosition(int x, int y) { m_pos = x; m_line = y; }

		int GetExtra(void) const { return m_extra; }
		void SetExtra(int l) { m_extra = l; }
		
		bool IsValid(void) const { return m_extra != -1; }
		void Invalidate(void){ m_extra = -1; }
		
		bool operator==(CMark &r) const { return m_line == r.m_line; }
		bool operator!=(CMark &r) const { return m_line != r.m_line; }

	private:
		string m_name;	//!<	要素名
		int	m_line;		//!<	該当行番号: 行番号は論理行で数える
		int m_pos;		//!<	該当桁位置
		int m_extra;	//!<	サブクラスで使える予備領域。
	};

	// GENERATE_FACTORY(CMark,CMarkFactory);	//	CMark用Factory class
	
	//	型宣言
	typedef std::vector<CMark> CMarkChain;
	typedef std::vector<CMark>::iterator	CMarkIterator;

	//	Interface
	//	constructor
	CMarkMgr() : curpos(0), maxitem(10){}
	// CMarkMgr(const CDocLineMgr *p) : doc(p) {}
	
	int Count(void) const { return dat.size(); }	//!<	項目数を返す
	int GetMax(void) const { return maxitem; }	//!<	最大項目数を返す
	void SetMax(int max);	//!<	最大項目数を設定

	virtual void Add(const CMark& m) = 0;	//!<	要素の追加
	
	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//!<	要素の全消去

	//!	要素の取得
	const CMark& GetCurrent(void) const { return dat[curpos]; }
	
	//	有効性の確認
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;
	
	//	現在位置の移動
	bool NextValid(void);
	bool PrevValid(void);

	const CMark& operator[](int index) const { return dat[index]; }
	
	//	連続取得インターフェース
	CMarkIterator CurrentPos(void) const { return (CMarkIterator)dat.begin() + curpos; }
	CMarkIterator Begin(void) const { return (CMarkIterator)dat.begin(); }
	CMarkIterator End(void) const { return (CMarkIterator)dat.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory m_factory;	//	Factory Class (マクロで生成される）
	CMarkChain dat;	//	マークデータ本体
	int curpos;	//	現在位置（番号）
	
	int maxitem;	//	保管可能アイテムの最大数
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
