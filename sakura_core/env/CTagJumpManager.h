/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

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
