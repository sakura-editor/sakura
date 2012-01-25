#include "StdAfx.h"
#include "CTextStream.h"
#include <vector>
using namespace std;
#include "charset/CCodeMediator.h"
#include "util/file.h"
#include "util/module.h"
#include "charset/CCodeFactory.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextInputStream                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextInputStream::CTextInputStream(const TCHAR* tszPath)
: CStream(tszPath,_T("rb"))
{
	m_bIsUtf8=false;

	if(Good()){
		//BOM確認 -> m_bIsUtf8
		static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
		BYTE buf[3];
		fread(&buf,1,sizeof(UTF8_BOM),GetFp());
		m_bIsUtf8 = (memcmp(buf,UTF8_BOM,sizeof(UTF8_BOM))==0);

		//UTF-8じゃなければ、ファイルポインタを元に戻す
		if(!m_bIsUtf8){
			fseek(GetFp(),0,SEEK_SET);
		}
	}
	else{
		m_bIsUtf8 = false;
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


wstring CTextInputStream::ReadLineW()
{
	//$$ 非効率だけど今のところは許して。。
	CMemory line;
	while(1){
		int c=getc(GetFp());
		if(c==EOF)break; //EOFで終了
		if(c=='\r'){ c=getc(GetFp()); if(c!='\n')ungetc(c,GetFp()); break; } //"\r" または "\r\n" で終了
		if(c=='\n')break; //"\n" で終了
		line.AppendRawData(&c,sizeof(char));
	}

	//UTF-8 → UNICODE
	if(m_bIsUtf8){
		CUtf8::UTF8ToUnicode(&line);
	}
	//Shift_JIS → UNICODE
	else{
		CShiftJis::SJISToUnicode(&line);
	}

	return wstring().assign( (wchar_t*)line.GetRawPtr(), line.GetRawLength()/sizeof(wchar_t) );	// EOL まで NULL 文字も含める
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextOutputStream                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextOutputStream::CTextOutputStream(const TCHAR* tszPath, ECodeType eCodeType, bool bExceptionMode)
: COutputStream(tszPath,_T("wb"),bExceptionMode)
{
	m_pcCodeBase = CCodeFactory::CreateCodeBase(eCodeType,0);
	if(Good()){
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
	if(nDataLen<0)nDataLen = wcslen(szData);
	const wchar_t* pData = szData;
	const wchar_t* pEnd = szData + nDataLen;

	//1行毎にカキコ。"\n"は"\r\n"に変換しながら出力。ただし、"\r\n"は"\r\r\n"に変換しない。
	const wchar_t* p = pData;
	while(1){
		//\nを検出。ただし\r\nは除外。
		const wchar_t* q = p;
		while(q<pEnd){
			if(*q==L'\n' && !((q-1)>=p && *(q-1)==L'\r') )break;
			q++;
		}
		const wchar_t* lf;
		if(q<pEnd)lf = q;
		else lf = NULL;

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
	auto_vsprintf_s(buf,_countof(buf),format,v);
	va_end(v);

	//出力
	WriteString(buf);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  CTextInputStream_AbsIni                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

static const TCHAR* _Resolve(const TCHAR* fname, bool bOrExedir)
{
	if( _IS_REL_PATH( fname ) ){
		static TCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		return path;
	}
	return fname;
}

CTextInputStream_AbsIni::CTextInputStream_AbsIni(const TCHAR* fname, bool bOrExedir )
: CTextInputStream(_Resolve(fname,bOrExedir))
{
}
