#pragma once

struct DispPos{
public:
	DispPos(int nDx,int nDy)
	: m_nDx(nDx)
	, m_nDy(nDy)
	{
		m_ptDrawOrigin.x=0;
		m_ptDrawOrigin.y=0;
		m_ptDrawLayout.x=CLayoutInt(0);
		m_ptDrawLayout.y=CLayoutInt(0);
		m_nLineRef=CLayoutInt(0);
		//キャッシュ
		m_pcLayoutRef = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( m_nLineRef );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画位置                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//固定値
	void InitDrawPos(const POINT& pt)
	{
		m_ptDrawOrigin=pt;
		m_ptDrawLayout.x=m_ptDrawLayout.y=CLayoutInt(0);
	}

	//取得
	CMyPoint GetDrawPos() const
	{
		return CMyPoint(
			m_ptDrawOrigin.x + (Int)m_ptDrawLayout.x * m_nDx,
			m_ptDrawOrigin.y + (Int)m_ptDrawLayout.y * m_nDy
		);
	}

	//進む
	void ForwardDrawCol (int nColOffset ){ m_ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine){ m_ptDrawLayout.y += nOffsetLine; }

	//リセット
	void ResetDrawCol(){ m_ptDrawLayout.x = CLayoutInt(0); }

	//取得
	CLayoutInt GetDrawCol() const{ return m_ptDrawLayout.x; }
	CLayoutInt GetDrawLine() const{ return m_ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     テキスト参照位置                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//変更
	void SetLayoutLineRef(CLayoutInt nOffsetLine)
	{
		m_nLineRef = nOffsetLine;
		//キャッシュ更新
		m_pcLayoutRef = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( m_nLineRef );
	}
	void ForwardLayoutLineRef(int nOffsetLine);


	//取得
	CLayoutInt		GetLayoutLineRef() const{ return m_nLineRef; }
	const CLayout*	GetLayoutRef() const{ return m_pcLayoutRef; }


private:
	//固定要素
	int				m_nDx;			//半角文字の文字間隔。固定。
	int				m_nDy;			//半角文字の行間隔。固定。
	POINT			m_ptDrawOrigin;	//描画位置基準。単位はピクセル。固定。

	//描画位置
	CLayoutPoint	m_ptDrawLayout; //描画位置。相対レイアウト単位。

	//テキスト参照位置
	CLayoutInt		m_nLineRef; //絶対レイアウト単位。

	//キャッシュ############
	CLayout*		m_pcLayoutRef;
};
