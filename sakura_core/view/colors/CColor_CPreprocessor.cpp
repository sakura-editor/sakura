/*! @file */
#include "StdAfx.h"
#include "CColor_CPreprocessor.h"

#include <vld.h>

extern "C" {
extern int mcpp_lib_main( int argc, char ** argv);
extern void mcpp_set_in_func( FILE* (* func_fopen) ( char const* fileName,char const* mode),
							  char* (* func_fgets) ( char * str, int num, FILE * stream ),
							  int   (* func_fclose) ( FILE* stream),
                              int   (* func_ferror) ( FILE * stream )
							 );

static CDocLine* g_pcDocLine;
static CDocLine* g_pcDocLineNext;
void skr_SetDocLineExcludedByCPreprocessor()
{
	g_pcDocLine->m_sMark.m_bExcludedByCPreprocessor = true;
}

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
		if (!g_pcDocLineNext)
			return NULL;
		g_pcDocLine = g_pcDocLineNext;
		int len;
		g_pcDocLine->m_sMark.m_bExcludedByCPreprocessor = false;
		const wchar_t* line = g_pcDocLine->GetDocLineStrWithEOL(&len);
		g_pcDocLineNext = g_pcDocLine->GetNextLine();
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
}

void CColor_CPreprocessor::Update(void)
{
	CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
	m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	if (pCEditDoc->m_cDocLineMgr.GetLineCount() == 0)
		return;
	if (!Disp())
		return;
	auto& docFile = pCEditDoc->m_cDocFile;
	const auto& filePath = docFile.GetFilePathClass();
	const auto& dirPath = filePath.GetDirPath();
	if (docFile.IsFileExist()) {
		g_mcpp_input_filename = to_achar(filePath);
	}
	else {
		g_mcpp_input_filename = to_achar((dirPath + L"tmp.c").c_str());
	}
	mcpp_set_in_func(func_fopen, func_fgets, func_fclose, func_ferror);
	g_pcDocLine = g_pcDocLineNext = pCEditDoc->m_cDocLineMgr.GetDocLineTop();

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

extern const CDocLine* g_pDocLineDrawing;

bool CColor_CPreprocessor::BeginColor(const CStringRef& cStr, int nPos)
{
	return g_pDocLineDrawing && g_pDocLineDrawing->m_sMark.m_bExcludedByCPreprocessor;
}

bool CColor_CPreprocessor::EndColor(const CStringRef& cStr, int nPos)
{
	return !g_pDocLineDrawing || !g_pDocLineDrawing->m_sMark.m_bExcludedByCPreprocessor;
}

