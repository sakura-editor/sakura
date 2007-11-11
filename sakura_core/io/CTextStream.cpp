#include "stdafx.h"
#include "CTextStream.h"
#include <vector>
using namespace std;
#include "charset/CCodeMediator.h"
#include "util/file.h"
#include "util/module.h"

//! c1 か c2 、先に見つかったほうのアドレスを返す
const char* strchr2(const char* str, char c1, char c2)
{
	const char* p=str;
	//※whileを先行すると、c1,c2が'\0'のときにNULLを返してしまうので、do～whileとした。
	do{
		if(*p==c1 || *p==c2)return p;
	}while(*p++);
	return NULL;
}

//定数
static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextInputStream                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextInputStream::CTextInputStream(const TCHAR* tszPath)
: CStream()
{
	m_bIsUtf8=false;
	Open(tszPath);
}

CTextInputStream::CTextInputStream()
: CStream()
{
	m_bIsUtf8=false;
}

CTextInputStream::~CTextInputStream()
{
}

void CTextInputStream::Open(const TCHAR* tszPath)
{
	CStream::Open(tszPath,_T("rb"));

	if(Good()){
		//BOM確認 -> m_bIsUtf8
		BYTE buf[3];
		fread(&buf,1,sizeof(UTF8_BOM),m_fp);
		m_bIsUtf8 = (memcmp(buf,UTF8_BOM,sizeof(UTF8_BOM))==0);

		//UTF-8じゃなければ、ファイルポインタを元に戻す
		if(!m_bIsUtf8){
			fseek(m_fp,0,SEEK_SET);
		}
	}
	else{
		m_bIsUtf8 = false;
	}
}

wstring CTextInputStream::ReadLineW()
{
	//$$ 非効率だけど今のところは許して。。
	CMemory line;
	while(1){
		int c=getc(m_fp);
		if(c==EOF)break; //EOFで終了
		if(c=='\r'){ c=getc(m_fp); if(c!='\n')ungetc(c,m_fp); break; } //"\r" または "\r\n" で終了
		if(c=='\n')break; //"\n" で終了
		line.AppendRawData(&c,sizeof(char));
	}

	//UTF-8 → UNICODE
	if(m_bIsUtf8){
		CNativeW cUnicode;
		CUtf8::UTF8ToUnicode(&line,&cUnicode);
		return cUnicode.GetStringPtr();
	}
	//Shift_JIS → UNICODE
	else{
		CShiftJis::SJISToUnicode(&line);
		return (const wchar_t*)line.GetRawPtr();
	}
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextOutputStream                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextOutputStream::CTextOutputStream(const TCHAR* tszPath)
: CStream(tszPath,_T("wb"))
{
	if(Good()){
		//BOM付加
		fwrite(UTF8_BOM,1,sizeof(UTF8_BOM),m_fp);
	}
}

CTextOutputStream::~CTextOutputStream()
{
}

void CTextOutputStream::WriteString(const wchar_t* szData)
{
	int nDataLen = wcslen(szData);

	//UTF-8に変換 -> out
	int nUtfLen = WideCharToMultiByte(
		CP_UTF8,
		0,
		szData,
		nDataLen,
		NULL,
		0,
		NULL,
		NULL
	);
	char* pUtf = new char[nUtfLen+1];
	nUtfLen = WideCharToMultiByte(
		CP_UTF8,
		0,
		szData,
		nDataLen,
		pUtf,
		nUtfLen,
		NULL,
		NULL
	);
	pUtf[nUtfLen]='\0';

	//カキコ。"\n"は"\r\n"に変換しながら出力。ただし、"\r\n"は"\r\r\n"に変換しない。
	const char* p=pUtf;
	const char* end=pUtf+nUtfLen;
	while(1){
		//\nを検出。ただし\r\nは除外。
		const char* lf=strchr(p,'\n');
		while(lf && (lf-1)>=p && *(lf-1)=='\r')lf=strchr(lf+1,'\n');
		
		if(lf){
			//\nまで出力
			fwrite(p,sizeof(char),lf-p,m_fp);
			p=lf+1;

			//\nを\r\nとして出力
			fwrite("\r\n",sizeof(char),2,m_fp);
		}
		else{
			//ぜんぶ出力
			fwrite(p,sizeof(char),end-p,m_fp);
			break;
		}
	}

	//後始末
	delete[] pUtf;
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

CTextInputStream_AbsIni::CTextInputStream_AbsIni(const TCHAR* fname, bool bOrExedir )
: CTextInputStream()
{
	if( _IS_REL_PATH( fname ) ){
		TCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		Open(path);
	}
	Open(fname);
}
