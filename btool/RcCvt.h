////
//	RcCvt		: convert MSVC type resource file to Brc32 type.
//		20010820 aroka

#include <stdio.h>

class RcCvt {
public:
	RcCvt();
	~RcCvt();
	bool init(char *infile, char*outfile);
	bool convert(void);

private:
	bool font(char*linestr);
	bool include(char*linestr);
	//! エラー文字列の追加
	bool setErr(char*errstr);

private:
	FILE *fpi;	//!<変換元(入力)ファイルのファイルポインタ
	FILE *fpo;	//!<変換先(出力)ファイルのファイルポインタ
	int nLine;	//!<1行の最大長．内部バッファの初期化時にバッファサイズとして使われる．
	char *strerr;	//!< エラー文字列へのポインタ
	char *buffer;	//!< 読み込みバッファへのポインタ．
};
