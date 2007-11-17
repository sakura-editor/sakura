#include "stdafx.h"
#include "CStream.h"

//コンストラクタ・デストラクタ
CStream::CStream(const TCHAR* tszPath, const TCHAR* tszMode)
{
	m_fp = NULL;
	Open(tszPath,tszMode);
}

CStream::CStream()
{
	m_fp = NULL;
}

CStream::~CStream()
{
	Close();
}

//操作
void CStream::Open(const TCHAR* tszPath, const TCHAR* tszMode)
{
	Close(); //既に開いていたら、一度閉じる
	m_fp = _tfopen(tszPath,tszMode);
}

void CStream::Close()
{
	if(m_fp){
		fclose(m_fp);
		m_fp=NULL;
	}
}

void CStream::SeekSet(	//!< シーク
	long offset	//!< ストリーム先頭からのオフセット 
)
{
	fseek(m_fp,offset,SEEK_SET);
}

void CStream::SeekEnd(   //!< シーク
	long offset //!< ストリーム終端からのオフセット
)
{
	fseek(m_fp,offset,SEEK_END);
}
