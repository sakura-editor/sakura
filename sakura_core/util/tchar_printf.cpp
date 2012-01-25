#include "StdAfx.h"
#include "tchar_printf.h"
#include "util/tchar_template.h"

#define MAX_BUF 0x7FFFFFFF

//�e���v���[�g�� TEXT<T> �g����΁A����ȉ����R�s�y���Ȃ��čςނ̂Ɂc
template <class T>
inline bool is_field_begin(T c)
{
	return c==_T2(T,'%');
}

//�����w��: flag
template <class T>
inline const T* skip_field_flag(const T* p)
{
	while(*p){
		T c=*p;
		if(c==_T2(T,'-') || c==_T2(T,'+') || c==_T2(T,'0') || c==_T2(T,' ') || c==_T2(T,'#')){ p++; continue; }
		break;
	}
	return p;
}

//�����w��: width
template <class T>
inline const T* skip_field_width(const T* p)
{
	if(*p>=_T2(T,'1') && *p<=_T2(T,'9'))p++; else return p; //�ꌅ�ڂ�0���󂯕t���Ȃ�
	while(*p>=_T2(T,'0') && *p<=_T2(T,'9'))p++;
	return p;
}

//�����w��: precision
template <class T>
inline const T* skip_field_precision(const T* p)
{
	if(*p==_T2(T,'.'))p++; else return p; //�h�b�g�Ŏn�܂镶����̂ݎ󂯕t����
	while(*p>=_T2(T,'0') && *p<=_T2(T,'9'))p++; //�悭�킩���̂łƂ肠�����S�������󂯕t����
	return p;
}

//�����w��: prefix
template <class T>
inline const T* skip_field_prefix(const T* p)
{
	if(*p==_T2(T,'t'))return p+1; //�Ǝ��g��
	if(*p==_T2(T,'h'))return p+1;
	if(p[0]==_T2(T,'l') && p[1]==_T2(T,'l'))return p+2;
	if(*p==_T2(T,'l'))return p+1;
	if(p[0]==_T2(T,'I') && p[1]==_T2(T,'3') && p[2]==_T2(T,'2'))return p+3;
	if(p[0]==_T2(T,'I') && p[1]==_T2(T,'6') && p[2]==_T2(T,'4'))return p+3;
	if(*p==_T2(T,'I'))return p+1;
	return p;
}

//�����w��: type
inline bool is_field_type(char c)
{
	return strchr("cCdiouxXeEfgGaAnpsS",c)!=NULL;
}
inline bool is_field_type(wchar_t c)
{
	return wcschr(L"cCdiouxXeEfgGaAnpsS",c)!=NULL;
}


//vsprintf_s API
static int local_vsprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v)
{
	return vsprintf_s(buf,nBufCount,format,v);
}

static int local_vsprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v)
{
	return vswprintf_s(buf,nBufCount,format,v);
}

//vsprintf API
static int local_vsprintf(char* buf, const char* format, va_list& v)
{
	return vsprintf(buf,format,v);
}

static int local_vsprintf(wchar_t* buf, const wchar_t* format, va_list& v)
{
	return vswprintf(buf,format,v);
}

//vsnprintf_s API
static int local_vsnprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v)
{
	return vsnprintf_s(buf,nBufCount,_TRUNCATE,format,v);
}

static int local_vsnprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v)
{
	return _vsnwprintf_s(buf,nBufCount,_TRUNCATE,format,v);
}

static void my_va_forward(va_list& v, const char* field)
{
	if(*field==0)return;
	const char* field_end=auto_strchr(field,0)-1;
	switch(*field_end){
	case 's':
	case 'S':
	case 'p':
		va_arg(v,void*);
		break;
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		va_arg(v,int);
		break;
	case 'c':
		if(field_end-1>=field && *(field_end-1)=='w')va_arg(v,wchar_t);
		else if(field_end-1>=field && *(field_end-1)=='l')va_arg(v,wchar_t);
		else va_arg(v,int);
		break;
	case 'C':
		if(field_end-1>=field && *(field_end-1)=='h')va_arg(v,int);
		else va_arg(v,wchar_t);
		break;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	case 'a':
	case 'A':
		va_arg(v,double);
		break;
	}
}
static void my_va_forward(va_list& v, const wchar_t* field)
{
	if(*field==0)return;
	const wchar_t* field_end=auto_strchr(field,0)-1;
	switch(*field_end){
	case L's':
	case L'S':
	case L'p':
		va_arg(v,void*);
		break;
	case L'd':
	case L'i':
	case L'o':
	case L'u':
	case L'x':
	case L'X':
		va_arg(v,int);
		break;
	case 'c':
		if(field_end-1>=field && *(field_end-1)==L'h')va_arg(v,int);
		else va_arg(v,int);
		break;
	case 'C':
		if(field_end-1>=field && *(field_end-1)==L'l')va_arg(v,wchar_t);
		else if(field_end-1>=field && *(field_end-1)==L'w')va_arg(v,wchar_t);
		else va_arg(v,int);
		break;
	case L'e':
	case L'E':
	case L'f':
	case L'g':
	case L'G':
	case L'a':
	case L'A':
		va_arg(v,double);
		break;
	}
}

static void field_convert(char* src)
{
	if(strncmp(src,"%ts",3)==0 || strncmp(src,"%tc",3)==0){
#ifdef _UNICODE
		src[1]='l';
#else
		src[1]='h';
#endif
	}
}

static void field_convert(wchar_t* src)
{
	if(wcsncmp(src,L"%ts",3)==0 || wcsncmp(src,L"%tc",3)==0){
#ifdef _UNICODE
		src[1]=L'l';
#else
		src[1]=L'h';
#endif
	}
}


//"%ts","%tc"���T�|�[�g
//�����{��l�����Ȃ��B(UNICODE�łł͂���Ŗ�肪�������Ȃ�)
template <class T>
int tchar_vsprintf_s_imp(T* buf, size_t nBufCount, const T* format, va_list& v, bool truncate)
{
	T* buf_end=buf+nBufCount; //�ϊ����~�b�g

	T* dst=buf;          //�ϊ��惏�[�N�ϐ�
	const T* src=format; //�ϊ������[�N�ϐ�
	while(*src){
		if(nBufCount!=MAX_BUF && dst>=buf_end-1)break;
		//�����w��t�B�[���h���擾
		if(is_field_begin(*src)){
			const T* field_begin=src;
			src++;
			src=skip_field_flag(src);
			src=skip_field_width(src);
			src=skip_field_precision(src);
			src=skip_field_prefix(src);

			if(is_field_type(*src)){
				src++;
				const T* field_end=src;

				//�t�B�[���h���ꎞ�ϐ��ɃR�s�[
				T field[64];
				if(field_end-field_begin>=_countof(field))field_end=field_begin+_countof(field)-1; //�t�B�[���h������
				auto_strncpy(field,field_begin,field_end-field_begin);
				field[field_end-field_begin] = 0;
				
				//�t�B�[���h����%ts�܂���%tc����������A�K�؂ɕϊ�
				field_convert(field);

				//�ϊ������͕W�����C�u�����ɈϏ�
				int ret;
				va_list tmp_v=v; //��v���R�s�[���ėp����
				if(truncate){
					ret=local_vsnprintf_s(dst,buf_end-dst,field,tmp_v);
					if( ret<0 ){
						//�o�b�t�@�ɓ��肫��Ȃ������񂪐؂�̂Ă�ꂽ
						return -1;
					}
				}
				else if(nBufCount!=MAX_BUF){
					ret=local_vsprintf_s(dst,buf_end-dst,field,tmp_v);
				}
				else{
					ret=local_vsprintf(dst,field,tmp_v);
				}

				//v��i�߂�B���M�Ȃ�����
				my_va_forward(v,field);

				//�ϊ��惏�[�N�|�C���^��i�߂�
				if(ret!=-1){
					dst+=ret;
				}
				src=field_end;
			}
			else{
				//�L���Ȍ^�t�B�[���h�ł͂Ȃ������̂ŁA���̂܂�܏o�͂����Ⴄ
				*dst++ = *src++;
			}
		}
		else{
			//���ϊ�
			*dst++ = *src++;
		}
	}
	//�I�[
	*dst = 0;

	if( truncate && *src != '\0' ){		//�؂�l�߂���ŁAsrc�̏������������Ă��Ȃ��ꍇ
		return -1;						//�؂�l�߂�ꂽ
	}
	return dst-buf;
}


int tchar_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<ACHAR>(buf,nBufCount,format,v,false);
}

int tchar_vswprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<WCHAR>(buf,nBufCount,format,v,false);
}




// vsprintf���b�v
// ��buf�ɏ\���ȗe�ʂ����邱�ƂɎ��M������Ƃ������A�g���Ă��������B
//
int tchar_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s(buf,MAX_BUF,format,v);
}
int tchar_vswprintf(WCHAR* buf, const WCHAR* format, va_list& v)
{
	return tchar_vswprintf_s(buf,MAX_BUF,format,v);
}


// vsnprintf_s���b�v
// �o�b�t�@���o�͕������菬�����ꍇ�͉\�Ȍ���o�͂��Ė�����\0��t���A�߂�l-1�ŕԂ�܂��B
//
int tchar_vsnprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<ACHAR>(buf,nBufCount,format,v,true);
}
int tchar_vsnwprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<WCHAR>(buf,nBufCount,format,v,true);
}


// sprintf_s���b�v
//
// (�����ɂ���)
//     ���e�������Ȃ̂ŁAtemplate�ł��ǂ������̂ł����A
//     ��������ƁAACHAR, WCHAR �ȊO�̌^����̈Öقň��S�ȃL���X�g��
//     �����Ȃ��Ȃ�A�R�[�f�B���O���s�ւɂȂ邽�߁A
//     �����āAACHAR, WCHAR �Ŋ֐����ЂƂ���`���Ă��܂��B
//
int tchar_sprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,nBufCount,format,v);
	va_end(v);
	return ret;
}
int tchar_swprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vswprintf_s(buf,nBufCount,format,v);
	va_end(v);
	return ret;
}


// sprintf���b�v
// ��buf�ɏ\���ȗe�ʂ����邱�ƂɎ��M������Ƃ������A�g���Ă��������B
//
// (�����ɂ���)
//     ���e�������Ȃ̂ŁAtemplate�ł��ǂ������̂ł����A
//     ��������ƁAACHAR, WCHAR �ȊO�̌^����̈Öقň��S�ȃL���X�g��
//     �����Ȃ��Ȃ�A�R�[�f�B���O���s�ւɂȂ邽�߁A
//     �����āAACHAR, WCHAR �Ŋ֐����ЂƂ���`���Ă��܂��B
//
int tchar_sprintf(ACHAR* buf, const ACHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,MAX_BUF,format,v);
	va_end(v);
	return ret;
}

int tchar_swprintf(WCHAR* buf, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vswprintf_s(buf,MAX_BUF,format,v);
	va_end(v);
	return ret;
}

// snprintf_s���b�v
// �o�b�t�@���o�͕������菬�����ꍇ�͉\�Ȍ���o�͂��Ė�����\0��t���A�߂�l-1�ŕԂ�܂��B
//
int tchar_snprintf_s(ACHAR* buf, size_t count, const ACHAR* format, ...) 
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,count,format,v);
	va_end(v);
	return ret;
}
int tchar_snwprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vswprintf_s(buf,count,format,v);
	va_end(v);
	return ret;
}


