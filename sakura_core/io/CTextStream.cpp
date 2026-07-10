/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "io/CTextStream.h"

#include "charset/CCodeFactory.h"
#include "charset/CShiftJis.h"	// move from CCodeMediator.h	2010/6/14 Uchi
#include "charset/CUtf8.h"		// move from CCodeMediator.h	2010/6/14 Uchi
#include "basis/CEol.h"
#include "util/file.h"			// _IS_REL_PATH
#include "util/module.h"

namespace cxx {

std::wstring_view MultiByteToWideChar(UINT codePage, std::string_view source, std::wstring& buffer);

} // namespace cxx

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextInputStream                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextInputStream::CTextInputStream(const WCHAR* pszPath)
: CStream(pszPath,L"rb")
{
	if(Good()){
		//BOM確認 -> m_bIsUtf8
		static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
		BYTE buf[3];
		if( sizeof(UTF8_BOM) == fread(&buf,1,sizeof(UTF8_BOM),GetFp()) ){
			m_bIsUtf8 = (memcmp(buf,UTF8_BOM,sizeof(UTF8_BOM))==0);
		}

		//UTF-8じゃなければ、ファイルポインタを元に戻す
		if(!m_bIsUtf8){
			fseek(GetFp(),0,SEEK_SET);
		}
	}
}

/*
CTextInputStream::CTextInputStream()
: CStream()
{
	m_bIsUtf8=false;
}
*/

CTextInputStream::~CTextInputStream()
{
}

/*!
 * @brief 1行読込。改行は削る
 */
void CTextInputStream::ReadLineW(std::wstring& line)
{
	//$$ 非効率だけど今のところは許して。。

	// 実は言う程「非効率」でもない。
	// ・ファイルデータ → 内部バッファ m_Buffer
	// ・内部バッファ m_Buffer → UNICODE文字列

	// 現在のバッファ書込位置はイテレータで管理する
	auto it = m_Buffer.begin();
	for (;; ++it) {
		int c = ::getc(GetFp());
		// CRを検出したらCRLFかどうかチェックする
		if ('\r' == c) {
			c = ::getc(GetFp());
			if ('\n' != c) {
				::ungetc(c, GetFp());
				c = '\r';
			}
		}

		// CRLF, LF, CR, EOF で終了
		if ('\n' == c || '\r' == c || EOF == c) break; //EOFで終了

		// バッファ末尾に到達したら拡張する
		if (it == m_Buffer.end()) {
			const auto pos = std::distance(m_Buffer.begin(), it);
			m_Buffer.resize(m_Buffer.size() * 2);
			it = m_Buffer.begin() + pos;
		}

		// バッファに書き込む
		*it = static_cast<char>(c);
	}

	// 変換に使うコードページを確定させる
	const auto codePage = m_bIsUtf8 ? CP_UTF8 : CP_SJIS;	// UTF-8ならCP_UTF8、そうでなければShift_JIS(CP932)とする

	// 変換を実行する
	cxx::MultiByteToWideChar(codePage, std::string_view{ m_Buffer.begin(), it }, line);
}

/*!
 * @brief 1行読込。改行は削る
 *
 * @note 既存コード互換のため残しておく。
 */
std::wstring CTextInputStream::ReadLineW()
{
	std::wstring line;
	ReadLineW(line);
	return line;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextOutputStream                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextOutputStream::CTextOutputStream(const WCHAR* pszPath, ECodeType eCodeType, bool bExceptionMode, bool bBom)
: COutputStream(pszPath,L"wb",bExceptionMode)
{
	m_pcCodeBase = CCodeFactory::CreateCodeBase(eCodeType,0);
	if(Good() && bBom){
		//BOM付加
		CMemory cmemBom;
		m_pcCodeBase->GetBom(&cmemBom);
		if(cmemBom.GetRawLength()>0){
			fwrite(cmemBom.GetRawPtr(),cmemBom.GetRawLength(),1,GetFp());
		}
	}
}

CTextOutputStream::~CTextOutputStream()
{
	delete m_pcCodeBase;
}

void CTextOutputStream::WriteString(
	const wchar_t*	szData,	//!< 書き込む文字列
	int				nLen	//!< 書き込む文字列長。-1を渡すと自動計算。
)
{
	//$$メモ: 文字変換時にいちいちコピーを作ってるので効率が悪い。後々効率改善予定。

	int nDataLen = nLen;
	if(nDataLen<0)nDataLen = (int)wcslen(szData);
	const wchar_t* pData = szData;
	const wchar_t* pEnd = szData + nDataLen;

	//1行毎にカキコ。"\n"は"\r\n"に変換しながら出力。ただし、"\r\n"は"\r\r\n"に変換しない。
	const wchar_t* p = pData;
	for (;;) {
		//\nを検出。ただし\r\nは除外。
		const wchar_t* q = p;
		while(q<pEnd){
			if(*q==L'\n' && !((q-1)>=p && *(q-1)==L'\r') )break;
			q++;
		}
		const wchar_t* lf;
		if(q<pEnd)lf = q;
		else lf = nullptr;

		if(lf){
			//\nの前まで(p～lf)出力
			CNativeW cSrc(p,lf-p);
			CMemory cDst;
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst); //コード変換
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());

			//\r\nを出力
			cSrc.SetString(L"\r\n");
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst);
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());

			//次へ
			p=lf+1;
		}
		else{
			//残りぜんぶ出力
			CNativeW cSrc(p,pEnd-p);
			CMemory cDst;
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst); //コード変換
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());
			break;
		}
	}
}

void CTextOutputStream::WriteF(const wchar_t* format, ...)
{
	//テキスト整形 -> buf
	static wchar_t buf[16*1024]; //$$ 確保しすぎかも？
	va_list v;
	va_start(v,format);
	auto_vsprintf_s(buf, std::size(buf),format,v);
	va_end(v);

	//出力
	WriteString(buf);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  CTextInputStream_AbsIni                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

static const WCHAR* _Resolve(const WCHAR* fname, bool bOrExedir)
{
	if( _IS_REL_PATH( fname ) ){
		static WCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		return path;
	}
	return fname;
}

CTextInputStream_AbsIni::CTextInputStream_AbsIni(const WCHAR* fname, bool bOrExedir )
: CTextInputStream(_Resolve(fname,bOrExedir))
{
}
