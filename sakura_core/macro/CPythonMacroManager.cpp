/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
#include <vector>
#include <string>
#include "CPythonMacroManager.h"

#include "macro/CSMacroMgr.h"
#include "macro/CMacroFactory.h"
#include "_os/OleTypes.h"
#include "CMacro.h"

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

namespace {

PyMethodDef g_moduleMethods[] = {
	{NULL, NULL, 0, NULL}
};

PyModuleDef g_moduleDef = {
	PyModuleDef_HEAD_INIT,
	"SakuraEditor",		// name of module
	"",					// module documentation, may be NULL
	-1,					// size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
	g_moduleMethods,
};

PyMODINIT_FUNC
PyInit_SakuraEditor(void)
{
	auto module = PyModule_Create(&g_moduleDef);
	return module;
}

std::vector<std::string> g_commandNames;
std::vector<PyMethodDef> g_commandDescs;
std::vector<std::string> g_functionNames;
std::vector<PyMethodDef> g_functionDescs;
CEditView* g_pEditView;

std::wstring utf8_to_utf16le(const char* src)
{
	int newLen = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
	std::wstring ret;
	ret.resize(newLen + 1);
	MultiByteToWideChar(CP_UTF8, 0, src, -1, &ret[0], newLen);
	return ret;
}

PyObject* handleCommand(PyObject* self, PyObject* args)
{
	const PyMethodDef* pDesc = (const PyMethodDef*) PyCapsule_GetPointer(self, nullptr);
	ptrdiff_t idx = pDesc - &g_commandDescs[0];
	assert(idx >= 0);
	assert(idx < g_commandNames.size());
	const MacroFuncInfo* info = &CSMacroMgr::m_MacroFuncInfoCommandArr[idx];

	size_t nArgs = PyTuple_GET_SIZE(args);
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
			if (!info->m_pData || info->m_pData->m_nArgMaxSize >= i) {
				varType = VT_EMPTY;
			}else {
				varType = info->m_pData->m_pVarArgEx[i - 4];
			}
		}
		if (varType == VT_EMPTY) {
			PyErr_BadArgument();
			return NULL;
		}

		if (varType == VT_BSTR) {
			wchar_t* str = PyUnicode_AsWideCharString(arg, NULL);
			strArguments[i] = str;
			PyMem_Free(str);
		}else if (varType == VT_I4) {
			if (PyLong_Check(arg)) {
				long value = PyLong_AsLong(arg);
				strArguments[i] = std::to_wstring(value);
			}else {
				PyErr_BadArgument();
				return NULL;
			}
		}else {
			assert(false);
		}
	}

	const wchar_t* arguments[8]{};
	int argLengths[8]{};
	for (size_t i=0; i<nArgs; ++i) {
		arguments[i] = strArguments[i].c_str();
		argLengths[i] = strArguments[i].size();
	}

	bool ret = CMacro::HandleCommand(g_pEditView, (EFunctionCode)info->m_nFuncID, arguments, argLengths, nArgs);

	PyObject* none = Py_BuildValue("");
	Py_INCREF(none);
	return none;
}

PyObject* handleFunction(PyObject* self, PyObject* args)
{
	const PyMethodDef* pDesc = (const PyMethodDef*) PyCapsule_GetPointer(self, nullptr);
	ptrdiff_t idx = pDesc - &g_functionDescs[0];
	assert(idx >= 0);
	assert(idx < g_functionNames.size());
	const MacroFuncInfo* info = &CSMacroMgr::m_MacroFuncInfoArr[idx];

	VARIANT vtArgs[8];
	size_t nArgs = PyTuple_GET_SIZE(args);
	size_t i;
	for (i=0; i<nArgs; ++i) {
		PyObject* arg = PyTuple_GetItem(args, i);
		if (!arg)
			break;
		VARTYPE varType;
		if (i < 4) {
			varType = info->m_varArguments[i];
		}else {
			if (!info->m_pData || info->m_pData->m_nArgMaxSize >= i) {
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
			const char* str = PyUnicode_AsUTF8(arg);
			SysString S(str, strlen(str));
			Wrap(&vtArgs[i])->Receive(S);
		}else if (varType == VT_I4) {
			vtArgs[i].vt = VT_I4;
			vtArgs[i].lVal = PyLong_AsLong(arg);
		}else {
			assert(false);
		}
	}

	PyObject* retObj = NULL;
	if (i == nArgs) {
		VARIANT vtResult;
		::VariantInit(&vtResult);
		bool ret = CMacro::HandleFunction(g_pEditView, (EFunctionCode)info->m_nFuncID, vtArgs, nArgs, vtResult);
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

	for (const MacroFuncInfo* info = &CSMacroMgr::m_MacroFuncInfoCommandArr[0]; info->m_nFuncID != F_INVALID; ++info) {
		g_commandNames.push_back(to_achar(info->m_pszFuncName));
	}
	for (const MacroFuncInfo* info = &CSMacroMgr::m_MacroFuncInfoArr[0]; info->m_nFuncID != F_INVALID; ++info) {
		g_functionNames.push_back(to_achar(info->m_pszFuncName));
	}
	for (auto& name : g_commandNames) {
		g_commandDescs.push_back({&name[0], (PyCFunction)handleCommand, METH_VARARGS, NULL});
	}
	for (auto& name : g_functionNames) {
		g_functionDescs.push_back({&name[0], (PyCFunction)handleFunction, METH_VARARGS, NULL});
	}

	s_initialized = true;
}

// virtual
CPythonMacroManager::~CPythonMacroManager()
{
}

bool CPythonMacroManager::ExecKeyMacro(CEditView *EditView, int flags) const
{
	static HMODULE s_hModule;
	if (!s_hModule) {
		s_hModule = LoadLibrary(L"python38.dll");
		if (!s_hModule) {
			return false;
		}
	}

	if (PyImport_AppendInittab("SakuraEditor", PyInit_SakuraEditor) == -1) {
		fprintf(stderr, "Error: could not extend in-built modules SakuraEditor\n");
		exit(1);
	}

	Py_InitializeEx(0);

	if (!Py_IsInitialized()) {
		return 0;
	}

	const char* version = Py_GetVersion();
	const char* compiler = Py_GetCompiler();

	PyObject* module = PyImport_ImportModule("SakuraEditor");
	if (!module) {
		PyErr_Print();
		fprintf(stderr, "Error: could not import module 'SakuraEditor'\n");
		assert(false);
	}

	for (auto& desc : g_commandDescs) {
		int ret = PyModule_AddObject(module, desc.ml_name, PyCFunction_New(&desc, PyCapsule_New(&desc, nullptr, nullptr)));
	}
	for (auto& desc : g_functionDescs) {
		int ret = PyModule_AddObject(module, desc.ml_name, PyCFunction_New(&desc, PyCapsule_New(&desc, nullptr, nullptr)));
	}

	g_pEditView = EditView;

#if 0
	auto mainModule = PyImport_AddModule("__main__");
	auto globals = PyModule_GetDict(mainModule);
	auto locals = PyDict_New();
	auto result = PyRun_String(m_str.c_str(), Py_file_input, globals, locals);
	if (result == NULL) {
		PyObject *pType, *pValue, *pTraceback;
		PyErr_Fetch(&pType, &pValue, &pTraceback);
		if (pType) {
			PyObject* pRepr = PyObject_Repr(pType);
			const char* str = PyUnicode_AsUTF8(pRepr);
			Py_DecRef(pRepr);
			Py_DecRef(pType);
		}
		if (pValue) {
			PyObject* pRepr = PyObject_Repr(pValue);
			const char* str = PyUnicode_AsUTF8(pRepr);
			Py_DecRef(pRepr);
			Py_DecRef(pValue);
		}
		if (pTraceback) {
			PyObject* pRepr = PyObject_Repr(pTraceback);
			const char* str = PyUnicode_AsUTF8(pRepr);
			Py_DecRef(pRepr);
			Py_DecRef(pTraceback);
		}
	}
	Py_XDECREF(result);
	Py_XDECREF(locals);
#else
	PyRun_SimpleString(m_str.c_str());
#endif

	if (Py_FinalizeEx() < 0) {
		return false;
	}

	return true;
}

BOOL CPythonMacroManager::LoadKeyMacro(HINSTANCE hInstance, const WCHAR* pszPath)
{
	FILE* f = _wfopen(pszPath, L"rb");
	long sz = _filelength(_fileno(f));
	m_str.resize(sz);
	fread(&m_str[0], 1, sz, f);
	fclose(f);
	return TRUE;
}

BOOL CPythonMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const WCHAR* pszCode)
{
	int newLen = WideCharToMultiByte(CP_UTF8, 0, pszCode, -1, NULL, 0, NULL, NULL);
	m_str.resize(newLen);
	WideCharToMultiByte(CP_UTF8, 0, pszCode, -1, &m_str[0], newLen, NULL, NULL);
	return TRUE;
}

// static
CMacroManagerBase* CPythonMacroManager::Creator(const WCHAR* FileExt)
{
	if (wcscmp( FileExt, L"py" ) == 0) {
		return new CPythonMacroManager;
	}
	return NULL;
}

// static
void CPythonMacroManager::declare()
{
	CMacroFactory::getInstance()->RegisterCreator( Creator );
}

