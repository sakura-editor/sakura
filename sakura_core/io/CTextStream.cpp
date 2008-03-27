#include "stdafx.h"
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
		//BOM�m�F -> m_bIsUtf8
		static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
		BYTE buf[3];
		fread(&buf,1,sizeof(UTF8_BOM),GetFp());
		m_bIsUtf8 = (memcmp(buf,UTF8_BOM,sizeof(UTF8_BOM))==0);

		//UTF-8����Ȃ���΁A�t�@�C���|�C���^�����ɖ߂�
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
	//$$ ����������Ǎ��̂Ƃ���͋����āB�B
	CMemory line;
	while(1){
		int c=getc(GetFp());
		if(c==EOF)break; //EOF�ŏI��
		if(c=='\r'){ c=getc(GetFp()); if(c!='\n')ungetc(c,GetFp()); break; } //"\r" �܂��� "\r\n" �ŏI��
		if(c=='\n')break; //"\n" �ŏI��
		line.AppendRawData(&c,sizeof(char));
	}

	//UTF-8 �� UNICODE
	if(m_bIsUtf8){
		CNativeW cUnicode;
		CUtf8::UTF8ToUnicode(line,&cUnicode);
		return cUnicode.GetStringPtr();
	}
	//Shift_JIS �� UNICODE
	else{
		CShiftJis::SJISToUnicode(&line);
		return (const wchar_t*)line.GetRawPtr();
	}
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CTextOutputStream                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextOutputStream::CTextOutputStream(const TCHAR* tszPath, ECodeType eCodeType, bool bExceptionMode)
: COutputStream(tszPath,_T("wb"),bExceptionMode)
{
	m_pcCodeBase = CCodeFactory::CreateCodeBase(eCodeType,0);
	if(Good()){
		//BOM�t��
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
	const wchar_t*	szData,	//!< �������ޕ�����
	int				nLen	//!< �������ޕ����񒷁B-1��n���Ǝ����v�Z�B
)
{
	//$$����: �����ϊ����ɂ��������R�s�[������Ă�̂Ō����������B��X�������P�\��B

	int nDataLen = nLen;
	if(nDataLen<0)nDataLen = wcslen(szData);
	const wchar_t* pData = szData;
	const wchar_t* pEnd = szData + nDataLen;

	//1�s���ɃJ�L�R�B"\n"��"\r\n"�ɕϊ����Ȃ���o�́B�������A"\r\n"��"\r\r\n"�ɕϊ����Ȃ��B
	const wchar_t* p = pData;
	while(1){
		//\n�����o�B������\r\n�͏��O�B
		const wchar_t* q = p;
		while(q<pEnd){
			if(*q==L'\n' && !((q-1)>=p && *(q-1)==L'\r') )break;
			q++;
		}
		const wchar_t* lf;
		if(q<pEnd)lf = q;
		else lf = NULL;

		if(lf){
			//\n�̑O�܂�(p�`lf)�o��
			CNativeW cSrc(p,lf-p);
			CMemory cDst;
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst); //�R�[�h�ϊ�
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());

			//\r\n���o��
			cSrc.SetString(L"\r\n");
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst);
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());

			//����
			p=lf+1;
		}
		else{
			//�c�肺��ԏo��
			CNativeW cSrc(p,pEnd-p);
			CMemory cDst;
			m_pcCodeBase->UnicodeToCode(cSrc,&cDst); //�R�[�h�ϊ�
			fwrite(cDst.GetRawPtr(),1,cDst.GetRawLength(),GetFp());
			break;
		}
	}
}

void CTextOutputStream::WriteF(const wchar_t* format, ...)
{
	//�e�L�X�g���` -> buf
	static wchar_t buf[16*1024]; //$$ �m�ۂ����������H
	va_list v;
	va_start(v,format);
	auto_vsprintf_s(buf,_countof(buf),format,v);
	va_end(v);

	//�o��
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
