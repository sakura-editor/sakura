/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "tchar_printf.h"
#include "util/tchar_template.h"

#define MAX_BUF 0x7FFFFFFF

//テンプレートで TEXT<T> 使えれば、こんな汚いコピペしなくて済むのに…
template <class T>
inline bool is_field_begin(T c)
{
	return c==_T2(T,'%');
}

//書式指定: flag
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

//書式指定: width
template <class T>
inline const T* skip_field_width(const T* p)
{
	if(*p>=_T2(T,'1') && *p<=_T2(T,'9'))p++; else return p; //一桁目は0を受け付けない
	while(*p>=_T2(T,'0') && *p<=_T2(T,'9'))p++;
	return p;
}

//書式指定: precision
template <class T>
inline const T* skip_field_precision(const T* p)
{
	if(*p==_T2(T,'.'))p++; else return p; //ドットで始まる文字列のみ受け付ける
	while(*p>=_T2(T,'0') && *p<=_T2(T,'9'))p++; //よくわからんのでとりあえず全数字を受け付ける
	return p;
}

//書式指定: prefix
template <class T>
inline const T* skip_field_prefix(const T* p)
{
	if(*p==_T2(T,'t'))return p+1; //独自拡張
	if(*p==_T2(T,'h'))return p+1;
	if(p[0]==_T2(T,'l') && p[1]==_T2(T,'l'))return p+2;
	if(*p==_T2(T,'l'))return p+1;
	if(p[0]==_T2(T,'I') && p[1]==_T2(T,'3') && p[2]==_T2(T,'2'))return p+3;
	if(p[0]==_T2(T,'I') && p[1]==_T2(T,'6') && p[2]==_T2(T,'4'))return p+3;
	if(*p==_T2(T,'I'))return p+1;
	return p;
}

//書式指定: type
inline bool is_field_type(char c)
{
	return strchr("cCdiouxXeEfgGaAnpsS",c)!=NULL;
}
inline bool is_field_type(wchar_t c)
{
	return wcschr(L"cCdiouxXeEfgGaAnpsS",c)!=NULL;
}


//vsprintf_s API
static inline int local_vsprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v)
{
	return vsprintf_s(buf,nBufCount,format,v);
}

static inline int local_vsprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v)
{
	return vswprintf_s(buf,nBufCount,format,v);
}

//vsprintf API
static inline int local_vsprintf(char* buf, const char* format, va_list& v)
{
	return vsprintf(buf,format,v);
}

static inline int local_vsprintf(wchar_t* buf, const wchar_t* format, va_list& v)
{
	return vswprintf(buf,format,v);
}

//vsnprintf_s API
static inline int local_vsnprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v)
{
	return vsnprintf_s(buf,nBufCount,_TRUNCATE,format,v);
}

static inline int local_vsnprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v)
{
	return _vsnwprintf_s(buf,nBufCount,_TRUNCATE,format,v);
}

static void my_va_forward(va_list& v, const char* field, const char* prefix)
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
		{
			// 2014.06.12 64bit値対応
			const char *p = prefix;
			if( p[0]=='I' && p[1]=='6' && p[2]=='4' ){
				va_arg(v,LONGLONG);
			}else{
				va_arg(v,int);
			}
		}
		break;
	case 'c':
		if(field_end-1>=field && *(field_end-1)=='w')va_arg(v,int); // wchar_t
		else if(field_end-1>=field && *(field_end-1)=='l')va_arg(v,int); // wchar_t
		else va_arg(v,int);
		break;
	case 'C':
		if(field_end-1>=field && *(field_end-1)=='h')va_arg(v,int);
		else va_arg(v,int); // wchar_t
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
static void my_va_forward(va_list& v, const wchar_t* field, const wchar_t* prefix)
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
		// 2014.06.12 64bit値対応
		{
			const wchar_t *p = prefix;
			if( p[0]==L'I' && p[1]==L'6' && p[2]==L'4' ){
				va_arg(v,LONGLONG);
			}else{
				va_arg(v,int);
			}
		}
		break;
	case 'c':
		if(field_end-1>=field && *(field_end-1)==L'h')va_arg(v,int);
		else va_arg(v,int);
		break;
	case 'C':
		if(field_end-1>=field && *(field_end-1)==L'l')va_arg(v,int); // wchar_t
		else if(field_end-1>=field && *(field_end-1)==L'w')va_arg(v,int); // wchar_t
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


//"%ts","%tc"をサポート
//※日本語考慮しない。(UNICODE版ではこれで問題が発生しない)
template <class T>
int tchar_vsprintf_s_imp(T* buf, size_t nBufCount, const T* format, va_list& v, bool truncate)
{
	T* buf_end=buf+nBufCount; //変換リミット

	T* dst=buf;          //変換先ワーク変数
	const T* src=format; //変換元ワーク変数
	while(*src){
		if(nBufCount!=MAX_BUF && dst>=buf_end-1)break;
		//書式指定フィールドを取得
		if(is_field_begin(*src)){
			const T* field_begin=src;
			src++;
			src=skip_field_flag(src);
			src=skip_field_width(src);
			src=skip_field_precision(src);
			const T* prefix = src;
			src=skip_field_prefix(src);

			if(is_field_type(*src)){
				src++;
				const T* field_end=src;

				//フィールドを一時変数にコピー
				T field[64];
				if(field_end-field_begin>=_countof(field))field_end=field_begin+_countof(field)-1; //フィールド長制限
				auto_strncpy(field,field_begin,field_end-field_begin);
				field[field_end-field_begin] = 0;
				
				//フィールド内に%tsまたは%tcがあったら、適切に変換
				field_convert(field);

				//変換処理は標準ライブラリに委譲
				int ret;
				va_list tmp_v=v; //※vをコピーして用いる
				if(truncate){
					ret=local_vsnprintf_s(dst,buf_end-dst,field,tmp_v);
					if( ret<0 ){
						//バッファに入りきらない文字列が切り捨てられた
						return -1;
					}
				}
				else if(nBufCount!=MAX_BUF){
					ret=local_vsprintf_s(dst,buf_end-dst,field,tmp_v);
				}
				else{
					ret=local_vsprintf(dst,field,tmp_v);
				}

				//vを進める。自信なっしんぐ
				my_va_forward(v,field, prefix);

				//変換先ワークポインタを進める
				if(ret!=-1){
					dst+=ret;
				}
				src=field_end;
			}
			else{
				//有効な型フィールドではなかったので、そのまんま出力しちゃう
				*dst++ = *src++;
			}
		}
		else{
			//無変換
			*dst++ = *src++;
		}
	}
	//終端
	*dst = 0;

	if( truncate && *src != '\0' ){		//切り詰めありで、srcの処理が完了していない場合
		return -1;						//切り詰められた
	}
	return dst-buf;
}


int tchar_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<ACHAR>(buf,nBufCount,format,v,false);
}

int tchar_vsprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<WCHAR>(buf,nBufCount,format,v,false);
}




// vsprintfラップ
// ※bufに十分な容量があることに自信があるときだけ、使ってください。
//
int tchar_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s(buf,MAX_BUF,format,v);
}
int tchar_vsprintf(WCHAR* buf, const WCHAR* format, va_list& v)
{
	return tchar_vsprintf_s(buf,MAX_BUF,format,v);
}


// vsnprintf_sラップ
// バッファが出力文字列より小さい場合は可能な限り出力して末尾に\0を付け、戻り値-1で返ります。
//
int tchar_vsnprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<ACHAR>(buf,nBufCount,format,v,true);
}
int tchar_vsnprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v)
{
	return tchar_vsprintf_s_imp<WCHAR>(buf,nBufCount,format,v,true);
}


// sprintf_sラップ
//
// (実装について)
//     内容が同じなので、templateでも良かったのですが、
//     そうすると、ACHAR, WCHAR 以外の型からの暗黙で安全なキャストが
//     効かなくなり、コーディングが不便になるため、
//     あえて、ACHAR, WCHAR で関数をひとつずつ定義しています。
//
int tchar_sprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,nBufCount,format,v);
	va_end(v);
	return ret;
}
int tchar_sprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,nBufCount,format,v);
	va_end(v);
	return ret;
}


// sprintfラップ
// ※bufに十分な容量があることに自信があるときだけ、使ってください。
//
// (実装について)
//     内容が同じなので、templateでも良かったのですが、
//     そうすると、ACHAR, WCHAR 以外の型からの暗黙で安全なキャストが
//     効かなくなり、コーディングが不便になるため、
//     あえて、ACHAR, WCHAR で関数をひとつずつ定義しています。
//
int tchar_sprintf(ACHAR* buf, const ACHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,MAX_BUF,format,v);
	va_end(v);
	return ret;
}

int tchar_sprintf(WCHAR* buf, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,MAX_BUF,format,v);
	va_end(v);
	return ret;
}

// snprintf_sラップ
// バッファが出力文字列より小さい場合は可能な限り出力して末尾に\0を付け、戻り値-1で返ります。
//
int tchar_snprintf_s(ACHAR* buf, size_t count, const ACHAR* format, ...) 
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,count,format,v);
	va_end(v);
	return ret;
}
int tchar_snprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...)
{
	va_list v;
	va_start(v,format);
	int ret=tchar_vsprintf_s(buf,count,format,v);
	va_end(v);
	return ret;
}


