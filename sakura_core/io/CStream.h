#pragma once

//ストリーム基底クラス
class CStream{
public:
	//コンストラクタ・デストラクタ
	CStream(const TCHAR* tszPath, const TCHAR* tszMode);
	CStream();
	virtual ~CStream();

	//演算子
	operator bool() const{ return Good(); }

	//操作
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();
	void SeekSet(	//!< シーク
		long offset	//!< ストリーム先頭からのオフセット 
	);
	void SeekEnd(   //!< シーク
		long offset //!< ストリーム終端からのオフセット
	);

	//状態
	virtual bool Good() const{ return m_fp!=NULL && !Eof(); }
	bool Eof() const{ return m_fp==NULL || feof(m_fp); }
protected:
	FILE* m_fp;
};

