/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2026 Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "macro/CPythonMacroManager.h"

#include "cxx/ResourceHolder.hpp"
#include "macro/CSMacroMgr.h"
#include "macro/CMacroFactory.h"
#include "_os/OleTypes.h"
#include "CMacro.h"
#include "util/tchar_convert.h"
#include "util/module.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"

#include "python3_config.h"

#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <io.h>

#include <Shlwapi.h>

namespace {

// ローカルマクロを定義するため、既存マクロを退避する
#pragma push_macro("DECLARE_API_FUNCTION")

/*!
 * @brief API関数を宣言するためのマクロ
 *
 * @param name API関数名
 */
#define DECLARE_API_FUNCTION(name) \
	#name

/*!
 * @brief 利用する DbgHelp APIの関数のリスト
 */
constexpr std::array g_PythonFunctions{
	DECLARE_API_FUNCTION(Py_CompileString),
	DECLARE_API_FUNCTION(Py_DecRef),
	DECLARE_API_FUNCTION(Py_FinalizeEx),
	DECLARE_API_FUNCTION(Py_GetConstant),
	DECLARE_API_FUNCTION(Py_InitializeEx),
	DECLARE_API_FUNCTION(Py_IsInitialized),
	DECLARE_API_FUNCTION(PyCapsule_GetPointer),
	DECLARE_API_FUNCTION(PyCapsule_New),
	DECLARE_API_FUNCTION(PyCMethod_New),
	DECLARE_API_FUNCTION(PyDict_New),
	DECLARE_API_FUNCTION(PyErr_BadArgument),
	DECLARE_API_FUNCTION(PyErr_Fetch),
	DECLARE_API_FUNCTION(PyErr_NormalizeException),
	DECLARE_API_FUNCTION(PyErr_Restore),
	DECLARE_API_FUNCTION(PyEval_EvalCode),
	DECLARE_API_FUNCTION(PyImport_AddModule),
	DECLARE_API_FUNCTION(PyImport_AppendInittab),
	DECLARE_API_FUNCTION(PyImport_ImportModule),
	DECLARE_API_FUNCTION(PyLong_AsLong),
	DECLARE_API_FUNCTION(PyLong_FromLong),
	DECLARE_API_FUNCTION(PyMem_Free),
	DECLARE_API_FUNCTION(PyModule_AddObject),
	DECLARE_API_FUNCTION(PyModule_Create2),
	DECLARE_API_FUNCTION(PyModule_GetDict),
	DECLARE_API_FUNCTION(PyObject_Str),
	DECLARE_API_FUNCTION(PyTuple_GetItem),
	DECLARE_API_FUNCTION(PyTuple_Size),
	DECLARE_API_FUNCTION(PyUnicode_AsWideCharString),
	DECLARE_API_FUNCTION(PyUnicode_FromWideChar),
};

// 既存マクロを復元する
#pragma pop_macro("DECLARE_API_FUNCTION")

/*!
 * @brief Python APIの関数ポインタを保持するシングルトン
 */
struct PythonApi final : public TSakuraSingleton<PythonApi> {
	using DllSearchPathHolder = cxx::ResourceHolder<&::RemoveDllDirectory>;

	~PythonApi() override = default;

	template <basis::NullTerminatedStringConstructible<WCHAR> A>
	HMODULE	LoadModule(const A& pythonDir);

	DllSearchPathHolder m_DllSearchPathHolder;

	std::map<std::string, FARPROC, std::less<>> m_Functions{};
};

template <basis::NullTerminatedStringConstructible<WCHAR> A>
HMODULE PythonApi::LoadModule(const A& arg)
{
	// 入力元をNUL終端文字列とみなす
	const auto szPythonDir = cxx::NullTerminatedString<WCHAR>{ arg };

	// 入力元を文字列として扱う
	std::wstring_view pythonDir{ szPythonDir };

	// python3.dllの名前は環境別で固定
	constexpr auto& dllName =
#ifdef __MINGW32__
		L"libpython3.dll"
#else
		L"python3.dll"
#endif
	;

	// 読み込み対象DLLのパスを組み立てる
	const auto pythonDirectory = std::filesystem::path(pythonDir);
	const auto dllPath = std::filesystem::absolute(pythonDirectory / dllName).lexically_normal();

	// pythonパスが空でなく存在している場合、DLL検索パスに追加する
	if (const auto& dllSearchPath = pythonDirectory;
		!dllSearchPath.empty() && fexist(dllSearchPath))
	{
		m_DllSearchPathHolder = ::AddDllDirectory(dllSearchPath.c_str());
	}

	// 追加検索パスを有効にしてDLLをロードする
	using DllHolder = cxx::ResourceHolder<&::FreeLibrary>;
	DllHolder hModule = ::LoadLibraryExW(
		dllPath.c_str(),
		nullptr,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
		LOAD_LIBRARY_SEARCH_SYSTEM32 |
		LOAD_LIBRARY_SEARCH_USER_DIRS
	);

	// 読み込み失敗時は例外を投げる
	if (!hModule) {
		const auto code = ::GetLastError();
		LPWSTR pMsg = nullptr;
		::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			LPWSTR(&pMsg),
			0,
			nullptr
		);
		std::wstring str = std::regex_replace(pMsg, std::wregex(L"%1"), dllPath.native());
		::LocalFree(HLOCAL(pMsg));
		throw std::domain_error(cxx::to_string(str, CP_UTF8));
	}

	// Python APIの関数ポインタをロードする
	std::ranges::for_each(
		g_PythonFunctions,
		[this, &hModule] (const auto& name)
	{
		const auto addr = ::GetProcAddress(
			hModule.get(),
			name
		);

		if (!addr) throw std::system_error(std::make_error_code(std::errc::not_supported), std::format("missing symbol '{}'", name));

		m_Functions.try_emplace(name, addr);
	});

	return hModule.release();
}

#pragma push_macro("USE_PYTHON_API")

#define USE_PYTHON_API(name) \
	const auto name = std::bit_cast<decltype(&:: name )>( PythonApi::getInstance()->m_Functions.at( #name ) ); \
	0 /* ←セミコロンを付けないとエラーになるように。 */

/*
 * @note マクロ関数を独自定義したもの。
 */
PyObject* PyXCFunctionNew(PyMethodDef* ml, PyObject* self)
{
	USE_PYTHON_API(PyCMethod_New);
	return PyCMethod_New(ml, self, nullptr, nullptr);
}

/*
 * @note inline関数を独自定義したもの。
 */
void PyXDecRef(
	PyObject* object
) noexcept
{
	USE_PYTHON_API(Py_DecRef);
	if (object != _Py_NULL) {
		Py_DecRef(object);
	}
}

using PyObjectPtr = cxx::ResourceHolder<&PyXDecRef>;

PyMethodDef g_moduleMethods[] = {
	{nullptr, nullptr, 0, nullptr}
};

PyModuleDef g_moduleDef = {
	PyModuleDef_HEAD_INIT,
	"SakuraEditor",		// name of module
	"",					// module documentation, may be NULL
	-1,					// size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
	g_moduleMethods,
};

PyObject* PyInit_SakuraEditor(void)
{
	USE_PYTHON_API(PyModule_Create2);
	return PyModule_Create2(&g_moduleDef, PYTHON_API_VERSION);
}

std::vector<std::string> g_commandNames;
std::vector<PyMethodDef> g_commandDescs;
std::vector<std::string> g_functionNames;
std::vector<PyMethodDef> g_functionDescs;

PyObject* handleCommand(PyObject* self, PyObject* args)
{
	USE_PYTHON_API(Py_GetConstant);
	USE_PYTHON_API(PyCapsule_GetPointer);
	USE_PYTHON_API(PyErr_BadArgument);
	USE_PYTHON_API(PyLong_AsLong);
	USE_PYTHON_API(PyMem_Free);
	USE_PYTHON_API(PyTuple_GetItem);
	USE_PYTHON_API(PyTuple_Size);
	USE_PYTHON_API(PyUnicode_AsWideCharString);

	const PyMethodDef* pDesc = (const PyMethodDef*) PyCapsule_GetPointer(self, nullptr);
	ptrdiff_t idx = pDesc - &g_commandDescs[0];
	assert(idx >= 0);
	assert((size_t)idx < g_commandNames.size());
	const MacroFuncInfo* info = &CSMacroMgr::GetCommandInfo()[idx];

	size_t nArgs = PyTuple_Size(args);
	std::vector<std::wstring> strArguments(nArgs);
	for (size_t i=0; i<nArgs; ++i) {
		PyObject* arg = PyTuple_GetItem(args, i);
		if (!arg) {
			break;
		}
		VARTYPE varType;
		if (i < 4) {
			varType = info->m_varArguments[i];
		}else {
			if (!info->m_pData || info->m_pData->m_nArgMaxSize >= (int)i) {
				varType = VT_EMPTY;
			}else {
				varType = info->m_pData->m_pVarArgEx[i - 4];
			}
		}
		if (varType == VT_EMPTY) {
			PyErr_BadArgument();
			return nullptr;
		}

		if (varType == VT_BSTR) {
			Py_ssize_t sz = 0;
			wchar_t* str = PyUnicode_AsWideCharString(arg, &sz);
			strArguments[i].assign(str, sz);
			PyMem_Free(str);
		}else if (varType == VT_I4) {
			long value = PyLong_AsLong(arg);
			strArguments[i] = std::to_wstring(value);
		}else {
			assert(false);
		}
	}

	const wchar_t* arguments[8]{};
	int argLengths[8]{};
	for (size_t i=0; i<nArgs; ++i) {
		arguments[i] = strArguments[i].c_str();
		argLengths[i] = (int)strArguments[i].size();
	}

	bool ret = CMacro::HandleCommand(
		&GetEditWnd().GetActiveView(),
		(EFunctionCode)info->m_nFuncID, arguments, argLengths, (int)nArgs);
	if (!ret) {
		DEBUG_TRACE(L"CMacro::HandleCommand failed, EFunctionCode: %d\n", info->m_nFuncID);
	}
	return Py_GetConstant(Py_CONSTANT_NONE);
}

PyObject* handleFunction(PyObject* self, PyObject* args)
{
	USE_PYTHON_API(PyCapsule_GetPointer);
	USE_PYTHON_API(PyErr_BadArgument);
	USE_PYTHON_API(PyLong_AsLong);
	USE_PYTHON_API(PyLong_FromLong);
	USE_PYTHON_API(PyMem_Free);
	USE_PYTHON_API(PyTuple_GetItem);
	USE_PYTHON_API(PyTuple_Size);
	USE_PYTHON_API(PyUnicode_AsWideCharString);
	USE_PYTHON_API(PyUnicode_FromWideChar);

	const PyMethodDef* pDesc = (const PyMethodDef*) PyCapsule_GetPointer(self, nullptr);
	ptrdiff_t idx = pDesc - &g_functionDescs[0];
	assert(idx >= 0);
	assert((size_t)idx < g_functionNames.size());
	const MacroFuncInfo* info = &CSMacroMgr::GetFuncInfo()[idx];

	VARIANT vtArgs[8];
	size_t nArgs = PyTuple_Size(args);
	size_t i;
	for (i=0; i<nArgs; ++i) {
		PyObject* arg = PyTuple_GetItem(args, i);
		if (!arg)
			break;
		VARTYPE varType;
		if (i < 4) {
			varType = info->m_varArguments[i];
		}else {
			if (!info->m_pData || info->m_pData->m_nArgMaxSize >= (int)i) {
				varType = VT_EMPTY;
			}else {
				varType = info->m_pData->m_pVarArgEx[i - 4];
			}
		}
		if (varType == VT_EMPTY) {
			PyErr_BadArgument();
			break;
		}

		::VariantInit(&vtArgs[i]);
		if (varType == VT_BSTR) {
			Py_ssize_t sz = 0;
			wchar_t* str = PyUnicode_AsWideCharString(arg, &sz);
			assert(str);
			SysString S(str, (int)sz);
			Wrap(&vtArgs[i])->Receive(S);
			PyMem_Free(str);
		}else if (varType == VT_I4) {
			vtArgs[i].vt = VT_I4;
			vtArgs[i].lVal = PyLong_AsLong(arg);
		}else {
			assert(false);
		}
	}

	PyObject* retObj = nullptr;
	if (i == nArgs) {
		VARIANT vtResult;
		::VariantInit(&vtResult);
		bool ret = CMacro::HandleFunction(
			&GetEditWnd().GetActiveView(),
			(EFunctionCode)info->m_nFuncID, vtArgs, (int)nArgs, vtResult);
		if (!ret) {
			DEBUG_TRACE(L"CMacro::HandleFunction failed, EFunctionCode: %d\n", info->m_nFuncID);
		}
		std::wstring str;
		switch (vtResult.vt) {
		case VT_I4:
			retObj = PyLong_FromLong(vtResult.lVal);
			break;
		case VT_BSTR:
			Wrap(&vtResult.bstrVal)->GetW(&str);
			retObj = PyUnicode_FromWideChar(str.c_str(), str.size());
			break;
		default:
			assert(false);
			break;
		}
		::VariantClear(&vtResult);
	}
	for (size_t j = 0; j < i; ++j) {
		::VariantClear(&vtArgs[j]);
	}

	return retObj;
}

} // namespace

CPythonMacroManager::CPythonMacroManager()
{
	static bool s_initialized = false;
	if (s_initialized)
		return;

	const auto commandCount = CSMacroMgr::GetCommandInfo().size();
	g_commandNames.reserve(commandCount);
	g_commandDescs.reserve(commandCount);
	for (const auto& info : CSMacroMgr::GetCommandInfo()) {
		if (info.m_nFuncID == F_INVALID) {
			break;
		}
		g_commandNames.emplace_back(cxx::to_string(info.m_pszFuncName));
		const auto name = std::data(g_commandNames.back());
		g_commandDescs.emplace_back(name, &handleCommand, METH_VARARGS, nullptr);
	}
	const auto functionCount = CSMacroMgr::GetFuncInfo().size();
	g_functionNames.reserve(functionCount);
	g_functionDescs.reserve(functionCount);
	for (const auto& info : CSMacroMgr::GetFuncInfo()) {
		if (info.m_nFuncID == F_INVALID) {
			break;
		}
		g_functionNames.emplace_back(cxx::to_string(info.m_pszFuncName));
		const auto name = std::data(g_functionNames.back());
		g_functionDescs.emplace_back(name, &handleFunction, METH_VARARGS, nullptr);
	}

	s_initialized = true;
}

static
void ShowError(HWND hWndMessageOwner, std::wstring_view caption)
{
	USE_PYTHON_API(PyErr_Fetch);
	USE_PYTHON_API(PyErr_NormalizeException);
	USE_PYTHON_API(PyErr_Restore);
	USE_PYTHON_API(PyMem_Free);
	USE_PYTHON_API(PyObject_Str);
	USE_PYTHON_API(PyUnicode_AsWideCharString);

	PyObject* ptype = nullptr;
	PyObject* pvalue = nullptr;
	PyObject* ptraceback = nullptr;

	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	if (pvalue) {
		if (PyObjectPtr str = PyObject_Str(pvalue)) {
			Py_ssize_t sz = 0;
			if (const auto pMsg = PyUnicode_AsWideCharString(str, &sz)) {
				::MessageBoxW(hWndMessageOwner, pMsg, std::data(caption), MB_ICONERROR);
				PyMem_Free(pMsg);
			}
		}
	}

	PyErr_Restore(ptype, pvalue, ptraceback);
}

bool CPythonMacroManager::ExecKeyMacro(CEditView *EditView, int flags [[maybe_unused]] ) const
{
	try {
		SFilePath dir = GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory;

		if (!s_hModule || std::wstring_view(dir) != std::wstring_view(prev_dir)) {
			prev_dir = dir;
			if (s_hModule) {
				::FreeLibrary(s_hModule);
				s_hModule = nullptr;
			}
			s_hModule = PythonApi::getInstance()->LoadModule(dir);

			if (!s_hModule) return false;
		}
	}
	catch (const std::domain_error& e) {
		ErrorMessage(nullptr, L"%ls", cxx::to_wstring(e.what(), CP_UTF8).c_str());
		PythonApi::resetInstance();
		return false;
	}

	USE_PYTHON_API(Py_CompileString);
	USE_PYTHON_API(Py_FinalizeEx);
	USE_PYTHON_API(Py_InitializeEx);
	USE_PYTHON_API(Py_IsInitialized);
	USE_PYTHON_API(PyCapsule_New);
	USE_PYTHON_API(PyDict_New);
	USE_PYTHON_API(PyEval_EvalCode);
	USE_PYTHON_API(PyImport_AppendInittab);
	USE_PYTHON_API(PyImport_ImportModule);
	USE_PYTHON_API(PyModule_AddObject);

	const auto& Py_XDECREF = PyXDecRef;

	if (PyImport_AppendInittab("SakuraEditor", PyInit_SakuraEditor) == -1) {
		fprintf(stderr, "Error: could not extend in-built modules SakuraEditor\n");
		return false;
	}

	Py_InitializeEx(0);

	if (!Py_IsInitialized()) {
		return false;
	}

	const auto hWndMessageOwner = EditView->GetHwnd();

	bool bSuccess = false;
	do {
		PyObjectPtr sakuraModule = PyImport_ImportModule("SakuraEditor");
		if (!sakuraModule) {
			break;
		}

		for (auto& desc : g_commandDescs) {
			auto cap = PyCapsule_New(&desc, nullptr, nullptr);
			auto fn = PyXCFunctionNew(&desc, cap);
			if (0 < PyModule_AddObject(sakuraModule, desc.ml_name, fn)) {
				Py_XDECREF(fn);
				Py_XDECREF(cap);
			}
		}
		for (auto& desc : g_functionDescs) {
			auto cap = PyCapsule_New(&desc, nullptr, nullptr);
			auto fn = PyXCFunctionNew(&desc, cap);
			if (0 < PyModule_AddObject(sakuraModule, desc.ml_name, fn)) {
				Py_XDECREF(fn);
				Py_XDECREF(cap);
			}
		}

		PyObjectPtr pCode = Py_CompileString(m_strMacro.c_str(), m_strPath.c_str(), Py_file_input);
		if (!pCode) {
			ShowError(hWndMessageOwner, m_wstrPath);
			break;
		}

		PyObjectPtr dict = PyDict_New();
		if (!dict) {
			break;
		}

		if (PyObjectPtr pObj = PyEval_EvalCode(pCode, dict, dict); !pObj) {
			ShowError(hWndMessageOwner, m_wstrPath);
			break;
		}
		bSuccess = true;
	} while (false);

	if (Py_FinalizeEx() < 0) {
		return false;
	}
	return bSuccess;
}

inline
bool wide2utf8(std::string& utf8, const WCHAR* psz)
{
	int nbytes = WideCharToMultiByte(CP_UTF8, 0, psz, -1, nullptr, 0, nullptr, nullptr);
	if (nbytes == 0)
		return false;
	utf8.resize(nbytes);
	nbytes = WideCharToMultiByte(CP_UTF8, 0, psz, -1, &utf8[0], nbytes, nullptr, nullptr);
	if (nbytes == 0)
		return false;
	return true;
}

BOOL CPythonMacroManager::LoadKeyMacro(
	HINSTANCE hInstance [[maybe_unused]],
	const WCHAR* pszPath
)
{
	// CTextInputStream は UTF-8 BOM がないファイルを SJIS とみなしてしまうので使わない
	FILE* f = nullptr;
	if (::_wfopen_s(&f, pszPath, L"rb")) {
		ResetReady();
		return FALSE;
	}

	m_wstrPath = pszPath;
	wide2utf8(m_strPath, pszPath);
	long sz = _filelength(_fileno(f));
	m_strMacro.resize(sz);
	fread(&m_strMacro[0], 1, sz, f);
	fclose(f);

	// detect and erase UTF-8 BOM
	constexpr const BYTE utf8BOM[]{ 0xef, 0xbb, 0xbf };
	if (sz >= 3 && memcmp(&m_strMacro[0], utf8BOM, 3) == 0) {
		m_strMacro.erase(0, 3);
	}

	SetReady();

	return TRUE;
}

BOOL CPythonMacroManager::LoadKeyMacroStr(
	HINSTANCE hInstance [[maybe_unused]],
	const WCHAR* pszCode
)
{
	m_strPath.clear();
	m_wstrPath.clear();
	const auto bRet = wide2utf8(m_strMacro, pszCode) ? TRUE : FALSE;

	SetReady();

	return bRet;
}

// static
CMacroManagerBase* CPythonMacroManager::Creator(const WCHAR* FileExt)
{
	if (_wcsicmp( FileExt, L"py" ) == 0) {
		return new CPythonMacroManager;
	}
	return nullptr;
}

#pragma pop_macro("USE_PYTHON_API")

// static
void CPythonMacroManager::declare()
{
	CMacroFactory::getInstance()->RegisterCreator( Creator );
}

