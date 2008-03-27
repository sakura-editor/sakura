#pragma once

class CFileAttribute;

//例外
class CError_FileOpen{};	//!< 例外：ファイルオープンに失敗
class CError_FileWrite{};	//!< 例外：ファイル書き込み失敗
class CError_FileRead{};	//!< 例外：ファイル読み込み失敗

//ストリーム基底クラス
class CStream{
public:
	//コンストラクタ・デストラクタ
	CStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false);
//	CStream();
	virtual ~CStream();

	//演算子
	operator bool() const{ return Good(); }

	//オープン・クローズ
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();

	//操作
	void SeekSet(	//!< シーク
		long offset	//!< ストリーム先頭からのオフセット 
	);
	void SeekEnd(   //!< シーク
		long offset //!< ストリーム終端からのオフセット
	);

	//状態
	virtual bool Good() const{ return m_fp!=NULL && !Eof(); }
	bool Eof() const{ return m_fp==NULL || feof(m_fp); }

	//ファイルハンドル
	FILE* GetFp() const{ return m_fp; }

	//モード
	bool IsExceptionMode() const{ return m_bExceptionMode; }
private:
	FILE*			m_fp;
	CFileAttribute*	m_pcFileAttribute;
	bool			m_bExceptionMode;
};


class COutputStream : public CStream{
public:
	COutputStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false)
	: CStream(tszPath, tszMode, bExceptionMode)
	{
	}

	//! データを無変換で書き込む。戻り値は書き込んだバイト数。
	int Write(const void* pBuffer, int nSizeInBytes)
	{
		int nRet = fwrite(pBuffer,1,nSizeInBytes,GetFp());
		if(nRet!=nSizeInBytes && IsExceptionMode())throw CError_FileWrite();
		return nRet;
	}
};



