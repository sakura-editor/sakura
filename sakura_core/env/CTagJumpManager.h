/*
	2008.05.18 kobake CShareData から分離
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
#ifndef SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_
#define SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_

// 要先行定義
// #define DLLSHAREDATA.h


// 2004/06/21 novice タグジャンプ機能追加
//! タグジャンプ情報
struct TagJump {
	HWND		hwndReferer;				//!< 参照元ウィンドウ
	CLogicPoint	point;						//!< ライン, カラム
};


//共有メモリ内構造体
//2004/06/21 タグジャンプ機能追加
//2005.04.03 MIK キーワード指定タグジャンプ
struct SShare_TagJump{
	//型
	typedef StaticVector<
		StaticString<WCHAR, _MAX_PATH>,
		MAX_TAGJUMP_KEYWORD
	>					ATagJumpKeywords;

	//データ
	int					m_TagJumpNum;					//!< タグジャンプ情報の有効データ数
	int					m_TagJumpTop;					//!< スタックの一番上の位置
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< タグジャンプ情報
	ATagJumpKeywords	m_aTagJumpKeywords;
	BOOL				m_bTagJumpICase;				//!< 大文字小文字を同一視
	BOOL				m_bTagJumpAnyWhere;				//!< 文字列の途中にマッチ
};


class CTagJumpManager{
public:
	CTagJumpManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//タグジャンプ関連	// 2004/06/21 novice タグジャンプ機能追加
	void PushTagJump(const TagJump *);		//!< タグジャンプ情報の保存
	bool PopTagJump(TagJump *);				//!< タグジャンプ情報の参照
private:
	DLLSHAREDATA* m_pShareData;
};

#endif /* SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_ */
/*[EOF]*/
