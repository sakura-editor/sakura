#pragma once

bool fexist(LPCTSTR pszPath); //!< ファイルまたはディレクトリが存在すればtrue

SAKURA_CORE_API bool IsFilePath( const wchar_t*, int*, int*, bool = true );
SAKURA_CORE_API bool IsFileExists(const TCHAR* path, bool bFileOnly = false);
bool IsDirectory(LPCTSTR pszPath);	// 2009.08.20 ryoji

//	Apr. 30, 2003 genta
//	ディレクトリの深さを調べる
int CalcDirectoryDepth(const TCHAR* path);

// 2005.11.26 aroka
bool IsLocalDrive( const TCHAR* pszDrive );

//※サクラ依存
FILE *_tfopen_absexe(LPCTSTR fname, LPCTSTR mode); // 2003.06.23 Moca
FILE *_tfopen_absini(LPCTSTR fname, LPCTSTR mode, BOOL bOrExedir = TRUE); // 2007.05.19 ryoji

//パス文字列処理
SAKURA_CORE_API void CutLastYenFromDirectoryPath( TCHAR* );						/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
SAKURA_CORE_API void AddLastYenFromDirectoryPath( TCHAR* );						/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
SAKURA_CORE_API void SplitPath_FolderAndFile( const TCHAR*, TCHAR*, TCHAR* );	/* ファイルのフルパスを、フォルダとファイル名に分割 */
SAKURA_CORE_API void Concat_FolderAndFile( const TCHAR*, const TCHAR*, TCHAR* );/* フォルダ、ファイル名から、結合したパスを作成 */
SAKURA_CORE_API BOOL GetLongFileName( const TCHAR*, TCHAR* );					/* ロングファイル名を取得する */
SAKURA_CORE_API BOOL CheckEXT( const TCHAR*, const TCHAR* );					/* 拡張子を調べる */
const TCHAR* GetFileTitlePointer(const TCHAR* tszPath);							//!< ファイルフルパス内のファイル名を指すポインタを取得。2007.09.20 kobake 作成
bool _IS_REL_PATH(const TCHAR* path);											//!< 相対パスか判定する。2003.06.23 Moca

//ファイル時刻
class CFileTime{
public:
	CFileTime(){ ClearFILETIME(); }
	CFileTime(const FILETIME& ftime){ SetFILETIME(ftime); }
	//設定
	void ClearFILETIME(){ m_ftime.dwLowDateTime = m_ftime.dwHighDateTime = 0; m_bModified = true; }
	void SetFILETIME(const FILETIME& ftime){ m_ftime = ftime; m_bModified = true; }
	//取得
	const FILETIME& GetFILETIME() const{ return m_ftime; }
	const SYSTEMTIME& GetSYSTEMTIME() const
	{
		//キャッシュ更新 -> m_systime, m_bModified
		if(m_bModified){
			m_bModified = false;
			FILETIME ftimeLocal;
			if(!::FileTimeToLocalFileTime( &m_ftime, &ftimeLocal ) || !::FileTimeToSystemTime( &ftimeLocal, &m_systime )){
				memset(&m_systime,0,sizeof(m_systime)); //失敗時ゼロクリア
			}
		}
		return m_systime;
	}
	const SYSTEMTIME* operator->() const{ return &GetSYSTEMTIME(); }
	//判定
	bool IsZero() const
	{
		return m_ftime.dwLowDateTime == 0 && m_ftime.dwHighDateTime == 0;
	}
protected:
private:
	FILETIME m_ftime;
	//キャッシュ
	mutable SYSTEMTIME	m_systime;
	mutable bool		m_bModified;
};
bool GetLastWriteTimestamp( const TCHAR* filename, CFileTime* pcFileTime ); //	Oct. 22, 2005 genta

//文字列分割
void my_splitpath ( const char *comln , char *drv,char *dir,char *fnm,char *ext );
void my_splitpath_w ( const wchar_t *comln , wchar_t *drv,wchar_t *dir,wchar_t *fnm,wchar_t *ext );
void my_splitpath_t ( const TCHAR *comln , TCHAR *drv,TCHAR *dir,TCHAR *fnm,TCHAR *ext );
#ifdef _UNICODE
#define my_splitpath_t my_splitpath_w
#else
#define my_splitpath_t my_splitpath
#endif
