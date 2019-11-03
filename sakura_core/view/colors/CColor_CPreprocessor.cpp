/*! @file */
#include "StdAfx.h"
#include "CColor_CPreprocessor.h"

#include <vld.h>

extern "C" {
char* mcpp_ifdef_false_lines;
extern int mcpp_lib_main( int argc, char ** argv);
extern void mcpp_set_in_func( FILE* (* func_fopen) ( char const* fileName,char const* mode),
							  char* (* func_fgets) ( char * str, int num, FILE * stream ),
							  int   (* func_fclose) ( FILE* stream),
                              int   (* func_ferror) ( FILE * stream )
							 );

static const CDocLine* g_pcDocLine;
static std::string g_mcpp_input_filename;
static FILE* g_mcpp_input_file = (FILE*)0xdeadbeef;

static
FILE* func_fopen(char const* fileName, char const* mode)
{
	if (fileName == g_mcpp_input_filename) {
		return g_mcpp_input_file;
	}
	else {
		return fopen(fileName, mode);
	}
}

static
char* func_fgets(char * str, int num, FILE * stream)
{
	if (stream == g_mcpp_input_file) {
		if (!g_pcDocLine)
			return NULL;
		int len;
		const wchar_t* line = g_pcDocLine->GetDocLineStrWithEOL(&len);
		g_pcDocLine = g_pcDocLine->GetNextLine();
		return strncpy(str, to_achar(line), num);
	}
	else
		return fgets(str, num, stream);
}

static
int func_fclose(FILE* stream)
{
	if (stream == g_mcpp_input_file)
		return 0;
	else
		return fclose(stream);
}

static
int func_ferror(FILE* stream)
{
	if (stream == g_mcpp_input_file)
		return 0;
	else
		return ferror(stream);
}

} // extern "C"

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
	const auto& docFile = pCEditDoc->m_cDocFile;
	const auto& filePath = docFile.GetFilePathClass();
	const auto& dirPath = filePath.GetDirPath();
	if (docFile.IsFileExist()) {
		g_mcpp_input_filename = to_achar(filePath);
	}
	else {
		g_mcpp_input_filename = to_achar((dirPath + L"tmp.c").c_str());
	}
	mcpp_set_in_func(func_fopen, func_fgets, func_fclose, func_ferror);
	g_pcDocLine = pCEditDoc->m_cDocLineMgr.GetDocLineTop();

	delete[] mcpp_ifdef_false_lines;
	mcpp_ifdef_false_lines = new char[pCEditDoc->m_cDocLineMgr.GetLineCount() + 3]();
	const char* args[5];

	int nArgs = 0;
	std::string includeDir = "-I";
	args[nArgs++] = NULL;
	args[nArgs++] = "-z"; // no output of included files
	args[nArgs++] = "-P"; // no #line output
	if (pCEditDoc->m_cDocFile.IsFileExist()) {
		includeDir += to_achar(filePath.GetDirPath().c_str());
		args[nArgs++] = includeDir.c_str();
	}
	args[nArgs++] = g_mcpp_input_filename.c_str();
	int ret = mcpp_lib_main(nArgs, (char**)args);
	++ret;
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

