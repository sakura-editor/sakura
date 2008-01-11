#pragma once

SAKURA_CORE_API bool IsFilePath( const wchar_t*, int*, int*, bool = true );
SAKURA_CORE_API bool IsFileExists(const TCHAR* path, bool bFileOnly = false);

//	Apr. 30, 2003 genta
//	�f�B���N�g���̐[���𒲂ׂ�
int CalcDirectoryDepth(const TCHAR* path);

// 2005.11.26 aroka
bool IsLocalDrive( const TCHAR* pszDrive );

//���T�N���ˑ�
FILE *_tfopen_absexe(LPCTSTR fname, LPCTSTR mode); // 2003.06.23 Moca
FILE *_tfopen_absini(LPCTSTR fname, LPCTSTR mode, BOOL bOrExedir = TRUE); // 2007.05.19 ryoji

//�p�X�����񏈗�
SAKURA_CORE_API void CutLastYenFromDirectoryPath( TCHAR* );						/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
SAKURA_CORE_API void AddLastYenFromDirectoryPath( TCHAR* );						/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
SAKURA_CORE_API void SplitPath_FolderAndFile( const TCHAR*, TCHAR*, TCHAR* );	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
SAKURA_CORE_API BOOL GetLongFileName( const TCHAR*, TCHAR* );					/* �����O�t�@�C�������擾���� */
SAKURA_CORE_API BOOL CheckEXT( const TCHAR*, const TCHAR* );					/* �g���q�𒲂ׂ� */
const TCHAR* GetFileTitlePointer(const TCHAR* tszPath);							//!< �t�@�C���t���p�X���̃t�@�C�������w���|�C���^���擾�B2007.09.20 kobake �쐬
bool _IS_REL_PATH(const TCHAR* path);											//!< ���΃p�X�����肷��B2003.06.23 Moca

//	Oct. 22, 2005 genta
bool GetLastWriteTimestamp( const TCHAR* filename, FILETIME& ftime );

void my_splitpath ( const char *comln , char *drv,char *dir,char *fnm,char *ext );
void my_splitpath_w ( const wchar_t *comln , wchar_t *drv,wchar_t *dir,wchar_t *fnm,wchar_t *ext );
void my_splitpath_t ( const TCHAR *comln , TCHAR *drv,TCHAR *dir,TCHAR *fnm,TCHAR *ext );
#ifdef _UNICODE
#define my_splitpath_t my_splitpath_w
#else
#define my_splitpath_t my_splitpath
#endif
