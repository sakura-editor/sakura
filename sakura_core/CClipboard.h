#pragma once

//!サクラエディタ用クリップボードクラス。後々はこの中で全てのクリップボードAPIを呼ばせたい。
class CClipboard{
public:
	//コンストラクタ・デストラクタ
	CClipboard(HWND hwnd); //!< コンストラクタ内でクリップボードが開かれる
	virtual ~CClipboard(); //!< デストラクタ内でCloseが呼ばれる

	//インターフェース
	void Empty(); //!< クリップボードを空にする
	void Close(); //!< クリップボードを閉じる
	bool SetText(const wchar_t* str,bool bColmnSelect, bool bLineSelect);   //!< テキストを設定する
	bool GetText(CNativeW* cmemBuf, BOOL* pbColmnSelect, BOOL* pbLineSelect); //!< テキストを取得する

	//演算子
	operator bool() const{ return m_bOpenResult!=FALSE; } //!< クリップボードを開けたならtrue

private:
	HWND m_hwnd;
	BOOL m_bOpenResult;




	// -- -- staticインターフェース -- -- //
public:
	static bool HasValidData();    //!< クリップボード内に、サクラエディタで扱えるデータがあればtrue
	static UINT GetSakuraFormat(); //!< サクラエディタ独自のクリップボードデータ形式
};
