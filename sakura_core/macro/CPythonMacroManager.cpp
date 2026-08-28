/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2022 Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <vector>
#include <string>
#include <io.h>
#include <shlwapi.h>
#include "CPythonMacroManager.h"

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

namespace {

/*!
 * @brief Python APIの関数ポインタを保持するシングルトン
 */
struct PythonApi final : public TSakuraSingleton<PythonApi> {
	using DllSearchPathHolder = cxx::ResourceHolder<&::RemoveDllDirectory>;

	~PythonApi() override = default;

	template<class Function>
	void LoadSymbol(HMODULE hModule, Function& function, LPCSTR name, const std::filesystem::path& dllPath) const
	{
		const auto addr = ::GetProcAddress(hModule, name);

		if (!addr) {
			throw std::domain_error(std::format("missing symbol '{}' in '{}'", name, cxx::to_string(dllPath.native(), CP_UTF8)));
		}

		function = std::bit_cast<Function>(addr);
	}

	HMODULE	LoadModule(std::wstring_view pythonDir);

	DllSearchPathHolder m_DllSearchPathHolder;

	decltype(&::Py_CompileString) Py_CompileString = nullptr;
	decltype(&::Py_DecRef) Py_DecRef = nullptr;
	decltype(&::Py_FinalizeEx) Py_FinalizeEx = nullptr;
	decltype(&::Py_GetConstant) Py_GetConstant = nullptr;
	decltype(&::Py_InitializeEx) Py_InitializeEx = nullptr;
	decltype(&::Py_IsInitialized) Py_IsInitialized = nullptr;
	decltype(&::PyCapsule_GetPointer) PyCapsule_GetPointer = nullptr;
	decltype(&::PyCapsule_New) PyCapsule_New = nullptr;
	decltype(&::PyCMethod_New) PyCMethod_New = nullptr;
	decltype(&::PyDict_New) PyDict_New = nullptr;
	decltype(&::PyErr_BadArgument) PyErr_BadArgument = nullptr;
	decltype(&::PyErr_Fetch) PyErr_Fetch = nullptr;
	decltype(&::PyErr_NormalizeException) PyErr_NormalizeException = nullptr;
	decltype(&::PyErr_Restore) PyErr_Restore = nullptr;
	decltype(&::PyEval_EvalCode) PyEval_EvalCode = nullptr;
	decltype(&::PyImport_AddModule) PyImport_AddModule = nullptr;
	decltype(&::PyImport_AppendInittab) PyImport_AppendInittab = nullptr;
	decltype(&::PyImport_ImportModule) PyImport_ImportModule = nullptr;
	decltype(&::PyLong_AsLong) PyLong_AsLong = nullptr;
	decltype(&::PyLong_FromLong) PyLong_FromLong = nullptr;
	decltype(&::PyMem_Free) PyMem_Free = nullptr;
	decltype(&::PyModule_AddObject) PyModule_AddObject = nullptr;
	decltype(&::PyModule_Create2) PyModule_Create2 = nullptr;
	decltype(&::PyModule_GetDict) PyModule_GetDict = nullptr;
	decltype(&::PyObject_Str) PyObject_Str = nullptr;
	decltype(&::PyTuple_GetItem) PyTuple_GetItem = nullptr;
	decltype(&::PyTuple_Size) PyTuple_Size = nullptr;
	decltype(&::PyUnicode_AsWideCharString) PyUnicode_AsWideCharString = nullptr;
	decltype(&::PyUnicode_FromWideChar) PyUnicode_FromWideChar = nullptr;
};

HMODULE PythonApi::LoadModule(std::wstring_view pythonDir)
{
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

#pragma push_macro("LOAD_SYMBOL")

#define LOAD_SYMBOL(name) \
	LoadSymbol(hModule, name, #name, dllPath); \
	0 /* ←セミコロンを付けないとエラーになるように。 */

	LOAD_SYMBOL(Py_CompileString);
	LOAD_SYMBOL(Py_DecRef);
	LOAD_SYMBOL(Py_FinalizeEx);
	LOAD_SYMBOL(Py_GetConstant);
	LOAD_SYMBOL(Py_InitializeEx);
	LOAD_SYMBOL(Py_IsInitialized);
	LOAD_SYMBOL(PyCapsule_GetPointer);
	LOAD_SYMBOL(PyCapsule_New);
	LOAD_SYMBOL(PyCMethod_New);
	LOAD_SYMBOL(PyDict_New);
	LOAD_SYMBOL(PyErr_BadArgument);
	LOAD_SYMBOL(PyErr_Fetch);
	LOAD_SYMBOL(PyErr_NormalizeException);
	LOAD_SYMBOL(PyErr_Restore);
	LOAD_SYMBOL(PyEval_EvalCode);
	LOAD_SYMBOL(PyImport_AddModule);
	LOAD_SYMBOL(PyImport_AppendInittab);
	LOAD_SYMBOL(PyImport_ImportModule);
	LOAD_SYMBOL(PyLong_AsLong);
	LOAD_SYMBOL(PyLong_FromLong);
	LOAD_SYMBOL(PyMem_Free);
	LOAD_SYMBOL(PyModule_AddObject);
	LOAD_SYMBOL(PyModule_Create2);
	LOAD_SYMBOL(PyModule_GetDict);
	LOAD_SYMBOL(PyObject_Str);
	LOAD_SYMBOL(PyTuple_GetItem);
	LOAD_SYMBOL(PyTuple_Size);
	LOAD_SYMBOL(PyUnicode_AsWideCharString);
	LOAD_SYMBOL(PyUnicode_FromWideChar);

#pragma pop_macro("LOAD_SYMBOL")

	return hModule.release();
}

/*
 * @note マクロ関数を独自定義したもの。
 */
PyObject* PyXCFunctionNew(PyMethodDef* ml, PyObject* self)
{
	const auto& PyCMethod_New = PythonApi::getInstance()->PyCMethod_New;
	return PyCMethod_New(ml, self, nullptr, nullptr);
}

/*
 * @note inline関数を独自定義したもの。
 */
void PyXDecRef(PyObject* object)
{
	const auto& Py_DecRef = PythonApi::getInstance()->Py_DecRef;
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
	const auto& PyModule_Create2 = PythonApi::getInstance()->PyModule_Create2;
	return PyModule_Create2(&g_moduleDef, PYTHON_API_VERSION);
}

std::vector<std::string> g_commandNames;
std::vector<PyMethodDef> g_commandDescs;
std::vector<std::string> g_functionNames;
std::vector<PyMethodDef> g_functionDescs;

PyObject* handleCommand(PyObject* self, PyObject* args)
{
	const auto& Py_GetConstant = PythonApi::getInstance()->Py_GetConstant;
	const auto& PyCapsule_GetPointer = PythonApi::getInstance()->PyCapsule_GetPointer;
	const auto& PyErr_BadArgument = PythonApi::getInstance()->PyErr_BadArgument;
	const auto& PyLong_AsLong = PythonApi::getInstance()->PyLong_AsLong;
	const auto& PyMem_Free = PythonApi::getInstance()->PyMem_Free;
	const auto& PyTuple_GetItem = PythonApi::getInstance()->PyTuple_GetItem;
	const auto& PyTuple_Size = PythonApi::getInstance()->PyTuple_Size;
	const auto& PyUnicode_AsWideCharString = PythonApi::getInstance()->PyUnicode_AsWideCharString;

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
	const auto& PyCapsule_GetPointer = PythonApi::getInstance()->PyCapsule_GetPointer;
	const auto& PyErr_BadArgument = PythonApi::getInstance()->PyErr_BadArgument;
	const auto& PyLong_AsLong = PythonApi::getInstance()->PyLong_AsLong;
	const auto& PyLong_FromLong = PythonApi::getInstance()->PyLong_FromLong;
	const auto& PyMem_Free = PythonApi::getInstance()->PyMem_Free;
	const auto& PyTuple_GetItem = PythonApi::getInstance()->PyTuple_GetItem;
	const auto& PyTuple_Size = PythonApi::getInstance()->PyTuple_Size;
	const auto& PyUnicode_AsWideCharString = PythonApi::getInstance()->PyUnicode_AsWideCharString;
	const auto& PyUnicode_FromWideChar = PythonApi::getInstance()->PyUnicode_FromWideChar;

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
	const auto& PyErr_Fetch = PythonApi::getInstance()->PyErr_Fetch;
	const auto& PyErr_NormalizeException = PythonApi::getInstance()->PyErr_NormalizeException;
	const auto& PyErr_Restore = PythonApi::getInstance()->PyErr_Restore;
	const auto& PyMem_Free = PythonApi::getInstance()->PyMem_Free;
	const auto& PyObject_Str = PythonApi::getInstance()->PyObject_Str;
	const auto& PyUnicode_AsWideCharString = PythonApi::getInstance()->PyUnicode_AsWideCharString;

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
		if (!s_hModule) {
			s_hModule = PythonApi::getInstance()->LoadModule(GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory);

			if (!s_hModule) return false;
		}
	}
	catch (const std::domain_error& e) {
		ErrorMessage(nullptr, L"%ls", cxx::to_wstring(e.what(), CP_UTF8).c_str());
		PythonApi::resetInstance();
		return false;
	}

	const auto& Py_CompileString = PythonApi::getInstance()->Py_CompileString;
	const auto& Py_FinalizeEx = PythonApi::getInstance()->Py_FinalizeEx;
	const auto& Py_InitializeEx = PythonApi::getInstance()->Py_InitializeEx;
	const auto& Py_IsInitialized = PythonApi::getInstance()->Py_IsInitialized;
	const auto& PyCapsule_New = PythonApi::getInstance()->PyCapsule_New;
	const auto& PyDict_New = PythonApi::getInstance()->PyDict_New;
	const auto& PyEval_EvalCode = PythonApi::getInstance()->PyEval_EvalCode;
	const auto& PyImport_AddModule = PythonApi::getInstance()->PyImport_AddModule;
	const auto& PyImport_AppendInittab = PythonApi::getInstance()->PyImport_AppendInittab;
	const auto& PyImport_ImportModule = PythonApi::getInstance()->PyImport_ImportModule;
	const auto& PyModule_AddObject = PythonApi::getInstance()->PyModule_AddObject;
	const auto& PyModule_GetDict = PythonApi::getInstance()->PyModule_GetDict;

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

		PyObjectPtr pMain = PyImport_AddModule("__main__");
		if (!pMain) {
			break;
		}

		PyObject* pGlobals = PyModule_GetDict(pMain); // borrowed reference
		if (!pGlobals) {
			break;
		}

		PyObjectPtr pLocals = PyDict_New();
		if (!pLocals) {
			break;
		}

		if (PyObjectPtr pObj = PyEval_EvalCode(pCode, pGlobals, pLocals); !pObj) {
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

BOOL CPythonMacroManager::LoadKeyMacro(HINSTANCE hInstance [[maybe_unused]], const WCHAR* pszPath)
{
	FILE* f = _wfopen(pszPath, L"rb");
	if (!f) {
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
	return TRUE;
}

BOOL CPythonMacroManager::LoadKeyMacroStr(HINSTANCE hInstance [[maybe_unused]], const WCHAR* pszCode)
{
	m_strPath.clear();
	m_wstrPath.clear();
	return wide2utf8(m_strMacro, pszCode) ? TRUE : FALSE;
}

// static
CMacroManagerBase* CPythonMacroManager::Creator(const WCHAR* FileExt)
{
	if (_wcsicmp( FileExt, L"py" ) == 0) {
		return new CPythonMacroManager;
	}
	return nullptr;
}

// static
void CPythonMacroManager::declare()
{
	CMacroFactory::getInstance()->RegisterCreator( Creator );
}

