/*
	2007.09.30 kobake 作成

	++ 概要 ++

	enum と define の2種類の定数定義ヘッダを生成するためのモノ

	enum定数はソースコードから参照し、(デバッグがしやすくなる)
	define定数はリソースから参照する  (リソース内ではenum定数を利用できない)


	++ 依存 ++

	Visual Studio の cl.exe に依存。
	cl.exe のプリプロセッサ機能を利用しています。


	++ 使用方法 ++

	usage() を参照
*/

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
using namespace std;

enum EMode{
	MODE_INVALID,

	MODE_ENUM,
	MODE_DEFINE,
};

int usage()
{
	printf("HeaderMake -in=<入力ファイル> -out=<出力ファイル> -mode=<モード> [-enum=<enum名>]\n");
	printf("モード\n");
	printf("  define  … #define による定数定義ヘッダを生成n");
	printf("  enum    … enum による定数定義ヘッダを生成。<enum名>を指定すればそれが列挙型の名前になる\n");
	return 1;
}

inline bool is_token(char c)
{
	return __iscsym(c) || c=='-';
}

//! 後ろから検索し、c1 か c2 、先に見つかったほうのアドレスを返す
const char* strrchr2(const char* str, char c1, char c2)
{
	const char* p=strchr(str,'\0');
	//※whileを先行すると、c1,c2が'\0'のときにNULLを返してしまうので、do～whileとした。
	do{
		if(*p==c1 || *p==c2)return p;
	}while(--p>=str);
	return NULL;
}

const char* GetFileTitlePointer(const char* fpath)
{
	const char* p=strrchr2(fpath,'\\','/');
	if(p){
		return p+1;
	}
	else{
		return fpath;
	}
}

int main(int argc,char* argv[])
{
	//引数解釈
	const char* in_file  =NULL;
	const char* out_file =NULL;
	const char* mode_name=NULL;
	const char* enum_name="";
	for(int i=1;i<argc;i++){
		char* p=argv[i];
		if(*p=='/')*p='-';
		if(strncmp(p,"-in",3)==0){
			p+=3;
			if(*p!='\0'){ if(*p=='=')p++; in_file=p; }
			else in_file=argv[++i];
		}
		else if(strncmp(p,"-out",4)==0){
			p+=4;
			if(*p!='\0'){ if(*p=='=')p++; out_file=p; }
			else out_file=argv[++i];
		}
		else if(strncmp(p,"-mode",5)==0){
			p+=5;
			if(*p!='\0'){ if(*p=='=')p++; mode_name=p; }
			else mode_name=argv[++i];
		}
		else if(strncmp(p,"-enum",5)==0){
			p+=5;
			if(*p!='\0'){ if(*p=='=')p++; enum_name=p; }
			else enum_name=argv[++i];
		}
		else{
			printf("Error: 不明な引数[%s]\n",p);
			return 1;
		}
	}
	if(!in_file)  { printf("Error: 入力ファイルを指定してください\n\n"); return usage(); }
	if(!out_file) { printf("Error: 出力ファイルを指定してください\n\n"); return usage(); }
	if(!mode_name){ printf("Error: モードを指定してください\n\n");       return usage(); }


	//モード解釈
	EMode mode=MODE_INVALID;
	if(_stricmp(mode_name,"DEFINE")==0)mode=MODE_DEFINE;
	else if(_stricmp(mode_name,"ENUM")==0)mode=MODE_ENUM;
	else{
		printf("Error: 不明なモード[%s]です\n",mode_name);
	}


	//ファイル更新時刻比較 (なんか、VSのカスタムビルドがうまくいかないので、ここで判定(汗) )
	struct _stat in_stat;
	struct _stat out_stat;
	if(-1==_stat(in_file, &in_stat ))goto next;
	if(-1==_stat(out_file,&out_stat))goto next;
	//入力ファイルが新しければ処理を続行する
	if(in_stat.st_mtime>out_stat.st_mtime)goto next;
	else{ printf("出力ファイルは最新です\n"); return 0; }


next:
	//ファイルオープン
	char in_file2[_MAX_PATH];
	sprintf_s(in_file2,_countof(in_file2),"cl.exe /EP %s",in_file);
	FILE* in=_popen(in_file2,"rt");               if(!in ){ printf("Error: 入力ファイルを開けません\n"); return 1; } //※プリプロセス済みストリーム
	FILE* out=NULL; fopen_s(&out,out_file,"wt");  if(!out){ printf("Error: 出力ファイルを開けません\n"); return 1; }


	//処理
	char line[1024];
	fprintf(out,"#pragma once\n\n");
	fprintf(out,
		"//このファイルは HeaderMake により %s から生成されたファイルです。\n"
		"//直接編集しないでください。\n\n",
		GetFileTitlePointer(in_file)
	);
	if(mode==MODE_ENUM)fprintf(out,"enum %s{\n",enum_name); //enum開始
	while(NULL!=fgets(line,_countof(line),in))
	{
		//改行除去
		{
			char* p=strrchr(line,'\n');
			if(p && *(p+1)=='\0')*p='\0';
		}

		//トークン抽出開始
		const char* p=line;
		const char* q;

		//ID文字列 -> id
		char id[256];
		while(*p && !is_token(*p))p++;
		q=p;
		while(*q && is_token(*q))q++;
		strncpy_s(id,_countof(id),p,q-p);
		id[q-p]='\0';
		p=q;

		//値 -> value
		char value[256];
		while(*p && !is_token(*p))p++;
		q=p;
		while(*q && is_token(*q))q++;
		strncpy_s(value,_countof(value),p,q-p);
		value[q-p]='\0';
		p=q;

		//出力
		if(*id && *value){
			if(mode==MODE_ENUM){
				fprintf(out,"\t%s = %s,\n",id,value);
			}
			else if(mode==MODE_DEFINE){
				fprintf(out,"#define %s %s\n",id,value);
			}
		}
	}
	if(mode==MODE_ENUM)fprintf(out,"};\n"); //enum終了


	//後始末
	fclose(out);
	fclose(in);
	return 0;
}
