/*! @file */
#include "StdAfx.h"
#include "CColor_CPreprocessor.h"

extern "C" {
int* mcpp_ifdef_false_lines;
extern int mcpp_lib_main( int argc, char ** argv);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    C プリプロセッサ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColor_CPreprocessor::~CColor_CPreprocessor()
{
	delete[] mcpp_ifdef_false_lines;
}

void CColor_CPreprocessor::Update(void)
{
	const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
	m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	if (!Disp())
		return;

	wchar_t szPathName[_MAX_PATH];
	wchar_t szTempFileName[_MAX_PATH];
	GetTempPath( _MAX_PATH, szPathName );
	GetTempFileName( szPathName, L"skr_", 0, szTempFileName );

	// 一時ファイル出力
	EConvertResult eRet = CWriteManager().WriteFile_From_CDocLineMgr(
		pCEditDoc->m_cDocLineMgr,
		SSaveInfo(
			szTempFileName,
			CODE_UTF8,
			EOL_NONE,
			true
		)
	);
	if (eRet != RESULT_FAILURE) {
		delete[] mcpp_ifdef_false_lines;
		mcpp_ifdef_false_lines = new int[pCEditDoc->m_cDocLineMgr.GetLineCount() + 3]();
		const char* args[5];

		int nArgs = 0;
		std::string includeDir = "-I";
		args[nArgs++] = NULL;
		args[nArgs++] = "-z"; // no output of included files
		args[nArgs++] = "-P"; // no #line output
		if (pCEditDoc->m_cDocFile.IsFileExist()) {
			includeDir += to_achar(pCEditDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str());
			args[nArgs++] = includeDir.c_str();
		}
		args[nArgs++] = to_achar(szTempFileName);
		int ret = mcpp_lib_main(nArgs, (char**)args);
		++ret;
	}
	::DeleteFile(szPathName);
}

extern int g_CColorStrategy_nCurLine;

bool CColor_CPreprocessor::BeginColor(const CStringRef& cStr, int nPos)
{
	return mcpp_ifdef_false_lines[g_CColorStrategy_nCurLine + 1] != 0;
}

bool CColor_CPreprocessor::EndColor(const CStringRef& cStr, int nPos)
{
	return mcpp_ifdef_false_lines[g_CColorStrategy_nCurLine + 1] == 0;;
}

