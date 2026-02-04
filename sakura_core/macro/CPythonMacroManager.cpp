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

#include "macro/CSMacroMgr.h"
#include "macro/CMacroFactory.h"
#include "_os/OleTypes.h"
#include "CMacro.h"
#include "util/tchar_convert.h"
#include "util/module.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"

namespace {

using Py_ssize_t = intptr_t;
using Py_hash_t = Py_ssize_t;

struct PyTypeObject;

struct PyObject
{
	Py_ssize_t ob_refcnt;
	PyTypeObject* ob_type;
};

#define PyObject_HEAD                   PyObject ob_base;

using PyCFunction = PyObject * (*)(PyObject*, PyObject*);

struct PyMethodDef
{
	const char* ml_name;
	PyCFunction ml_meth;
	int ml_flags;
	const char* ml_doc;
};

struct PyModuleDef_Base
{
	PyObject_HEAD
		PyObject* (*m_init)();
	Py_ssize_t m_index;
	PyObject* m_copy;
};

using visitproc = int (*)(PyObject*, void*);
using traverseproc = int (*)(PyObject*, visitproc, void*);
using freefunc = void (*)(void*);
using inquiry = int (*)(PyObject*);

struct PyModuleDef
{
	PyModuleDef_Base m_base;
	const char* m_name;
	const char* m_doc;
	Py_ssize_t m_size;
	PyMethodDef* m_methods;
	struct PyModuleDef_Slot* m_slots;
	traverseproc m_traverse;
	inquiry m_clear;
	freefunc m_free;
};

struct PyVarObject
{
	PyObject ob_base;
	Py_ssize_t ob_size;
};

#define PyObject_VAR_HEAD      PyVarObject ob_base;

struct PyTupleObject
{
	PyObject_VAR_HEAD
		PyObject* ob_item[1];
};

#if 0
#define _PyObject_EXTRA_INIT 0, 0,
#else
#define _PyObject_EXTRA_INIT
#endif

#define PyObject_HEAD_INIT(type)        \
    { _PyObject_EXTRA_INIT              \
    1, type },

#define PyModuleDef_HEAD_INIT { \
    PyObject_HEAD_INIT(NULL)    \
    NULL, /* m_init */          \
    0,    /* m_index */         \
    NULL, /* m_copy */          \
  }

inline Py_ssize_t Py_REFCNT(const void* ob) {
	return reinterpret_cast<const PyObject*>(ob)->ob_refcnt;
}

inline PyTypeObject* Py_TYPE(const void* ob) {
	return reinterpret_cast<const PyObject*>(ob)->ob_type;
}

inline Py_ssize_t Py_SIZE(const void* ob) {
	return reinterpret_cast<const PyVarObject*>(ob)->ob_size;
}

inline Py_ssize_t PyTuple_GET_SIZE(const void* op) {
	return Py_SIZE(op);
}

#define EXTERN

EXTERN void (*Py_Initialize)();
EXTERN void (*Py_InitializeEx)(int);
EXTERN void (*Py_Finalize)();
EXTERN int (*Py_FinalizeEx)(); // >= 3.6
EXTERN int (*Py_IsInitialized)();

EXTERN const char* (*Py_GetVersion)();
EXTERN const char* (*Py_GetCompiler)();
EXTERN wchar_t* (*Py_GetProgramFullPath)();

EXTERN void (*Py_IncRef)(PyObject*);
EXTERN void (*Py_DecRef)(PyObject*);

EXTERN void* (*PyCapsule_GetPointer)(PyObject* capsule, const char* name);
using PyCapsule_Destructor = void (*)(PyObject*);
EXTERN PyObject* (*PyCapsule_New)(void* pointer, const char* name, PyCapsule_Destructor destructor);

// Raw Memory Interface
EXTERN void (*PyMem_Free)(void* p);

// Unicode Objects
EXTERN wchar_t* (*PyUnicode_AsWideCharString)(PyObject* unicode, Py_ssize_t* size);

// Tuple Objects
EXTERN PyObject* (*PyTuple_New)(Py_ssize_t len);
EXTERN PyObject* (*PyTuple_Pack)(Py_ssize_t n, ...);
EXTERN Py_ssize_t(*PyTuple_Size)(PyObject* p);
EXTERN PyObject* (*PyTuple_GetItem)(PyObject*, Py_ssize_t);
EXTERN PyObject* (*PyTuple_GetSlice)(PyObject* p, Py_ssize_t low, Py_ssize_t high);
EXTERN int (*PyTuple_SetItem)(PyObject* p, Py_ssize_t pos, PyObject* o);

// Struct Sequence Objects
struct PyStructSequence_Desc;
EXTERN PyTypeObject* (*PyStructSequence_NewType)(PyStructSequence_Desc* desc);
EXTERN PyObject* (*PyStructSequence_New)(PyTypeObject* type);
EXTERN PyObject* (*PyStructSequence_GetItem)(PyObject* p, Py_ssize_t pos);
EXTERN void (*PyStructSequence_SetItem)(PyObject* p, Py_ssize_t pos, PyObject* o);

// List Objects
EXTERN PyObject* (*PyList_New)(Py_ssize_t len);
EXTERN Py_ssize_t(*PyList_Size)(PyObject* list);
EXTERN PyObject* (*PyList_GetItem)(PyObject* list, Py_ssize_t index);
EXTERN int (*PyList_SetItem)(PyObject* list, Py_ssize_t index, PyObject* item);
EXTERN int (*PyList_Insert)(PyObject* list, Py_ssize_t index, PyObject* item);
EXTERN int (*PyList_Append)(PyObject* list, PyObject* item);
EXTERN PyObject* (*PyList_GetSlice)(PyObject* list, Py_ssize_t low, Py_ssize_t high);
EXTERN int (*PyList_SetSlice)(PyObject* list, Py_ssize_t low, Py_ssize_t high, PyObject* itemlist);
EXTERN int (*PyList_Sort)(PyObject* list);
EXTERN int (*PyList_Reverse)(PyObject* list);
EXTERN PyObject* (*PyList_AsTuple)(PyObject* list);

// Dictionary Objects
EXTERN PyObject* (*PyDict_New)();
EXTERN void (*PyDict_Clear)(PyObject* p);
EXTERN int (*PyDict_Contains)(PyObject* p, PyObject* key);
EXTERN PyObject* (*PyDict_Copy)(PyObject* p);
EXTERN int (*PyDict_SetItem)(PyObject* p, PyObject* key, PyObject* val);
EXTERN int (*PyDict_SetItemString)(PyObject* p, const char* key, PyObject* val);
EXTERN int (*PyDict_DelItem)(PyObject* p, PyObject* key);
EXTERN int (*PyDict_DelItemString)(PyObject* p, const char* key);
EXTERN PyObject* (*PyDict_GetItem)(PyObject* p, PyObject* key);
EXTERN PyObject* (*PyDict_GetItemWithError)(PyObject* p, PyObject* key);
EXTERN PyObject* (*PyDict_GetItemString)(PyObject* p, const char* key);
EXTERN PyObject* (*PyDict_Items)(PyObject* p);
EXTERN PyObject* (*PyDict_Keys)(PyObject* p);
EXTERN PyObject* (*PyDict_Values)(PyObject* p);
EXTERN Py_ssize_t(*PyDict_Size)(PyObject* p);
EXTERN int (*PyDict_Next)(PyObject* p, Py_ssize_t* ppos, PyObject** pkey, PyObject** pvalue);
EXTERN int (*PyDict_Merge)(PyObject* a, PyObject* b, int override);
EXTERN int (*PyDict_Update)(PyObject* a, PyObject* b);
EXTERN int (*PyDict_MergeFromSeq2)(PyObject* a, PyObject* seq2, int override);

// Set Objects
EXTERN PyObject* (*PySet_New)(PyObject* iterable);
EXTERN PyObject* (*PyFrozenSet_New)(PyObject* iterable);
EXTERN Py_ssize_t(*PySet_Size)(PyObject* anyset);
EXTERN int (*PySet_Contains)(PyObject* anyset, PyObject* key);
EXTERN int (*PySet_Add)(PyObject* set, PyObject* key);
EXTERN int (*PySet_Discard)(PyObject* set, PyObject* key);
EXTERN PyObject* (*PySet_Pop)(PyObject* set);
EXTERN int (*PySet_Clear)(PyObject* set);

// File Objects
EXTERN PyObject* (*PyFile_FromFd)(int fd, const char* name, const char* mode, int buffering, const char* encoding, const char* errors, const char* newline, int closefd);
EXTERN int (*PyObject_AsFileDescriptor)(PyObject* p);
EXTERN PyObject* (*PyFile_GetLine)(PyObject* p, int n);
EXTERN int (*PyFile_WriteObject)(PyObject* obj, PyObject* p, int flags);
EXTERN int (*PyFile_WriteString)(const char* s, PyObject* p);

// Module Objects
EXTERN PyObject* (*PyModule_NewObject)(PyObject* name);
EXTERN PyObject* (*PyModule_New)(const char* name);
EXTERN PyObject* (*PyModule_GetDict)(PyObject* module);
EXTERN PyObject* (*PyModule_GetNameObject)(PyObject* module);
EXTERN const char* (*PyModule_GetName)(PyObject* module);
EXTERN void* (*PyModule_GetState)(PyObject* module);
EXTERN PyModuleDef* (*PyModule_GetDef)(PyObject* module);
EXTERN PyObject* (*PyModule_GetFilenameObject)(PyObject* module);
EXTERN const char* (*PyModule_GetFilename)(PyObject* module);
#define PYTHON_ABI_VERSION 3
EXTERN PyObject* (*PyModule_Create2)(struct PyModuleDef*, int apiver);
inline PyObject* PyModule_Create(struct PyModuleDef* def) {
	return PyModule_Create2(def, PYTHON_ABI_VERSION);
}
EXTERN PyObject* (*PyModuleDef_Init)(PyModuleDef* def);
EXTERN PyObject* (*PyModule_FromDefAndSpec2)(PyModuleDef* def, PyObject* spec, int module_api_version);
EXTERN int (*PyModule_ExecDef)(PyObject* module, PyModuleDef* def);
EXTERN int (*PyModule_SetDocString)(PyObject* module, const char* docstring);
EXTERN int (*PyModule_AddFunctions)(PyObject* module, PyMethodDef* functions);
EXTERN int (*PyModule_AddObjectRef)(PyObject* module, const char* name, PyObject* value);
EXTERN int (*PyModule_AddObject)(PyObject* module, const char* name, PyObject* value);
EXTERN int (*PyModule_AddIntConstant)(PyObject* module, const char* name, long value);
EXTERN int (*PyModule_AddStringConstant)(PyObject* module, const char* name, const char* value);
EXTERN int (*PyModule_AddType)(PyObject* module, PyTypeObject* type);

// Module lookup
EXTERN PyObject* (*PyState_FindModule)(PyModuleDef* def);
EXTERN int (*PyState_AddModule)(PyObject* module, PyModuleDef* def);
EXTERN int (*PyState_RemoveModule)(PyModuleDef* def);

EXTERN void (*PyErr_PrintEx)(int set_sys_last_vars);
EXTERN void (*PyErr_Print)();
EXTERN int (*PyErr_BadArgument)();
EXTERN PyObject* (*PyErr_Occurred)();
EXTERN void (*PyErr_Fetch)(PyObject** ptype, PyObject** pvalue, PyObject** ptraceback);
EXTERN void (*PyErr_Restore)(PyObject* type, PyObject* value, PyObject* traceback);
EXTERN void (*PyErr_NormalizeException)(PyObject** exc, PyObject** val, PyObject** tb);

EXTERN long (*PyLong_AsLong)(PyObject*);
EXTERN PyObject* (*PyLong_FromLong)(long);

EXTERN PyObject* (*PyUnicode_AsUTF8String)(PyObject* unicode);
EXTERN const char* (*PyUnicode_AsUTF8AndSize)(PyObject* unicode, Py_ssize_t* size);
EXTERN const char* (*PyUnicode_AsUTF8)(PyObject* unicode);
EXTERN PyObject* (*PyUnicode_FromWideChar)(const wchar_t* w, Py_ssize_t size);

// Operating System Utilities
EXTERN PyObject* (*PyOS_FSPath)(PyObject* path);
EXTERN void (*PyOS_BeforeFork)();
EXTERN void (*PyOS_AfterFork_Parent)();
EXTERN void (*PyOS_AfterFork_Child)();
EXTERN void (*PyOS_AfterFork)();
EXTERN int (*PyOS_CheckStack)();
using PyOS_sighandler_t = void (*)(int);
EXTERN PyOS_sighandler_t(*PyOS_getsig)(int i);
EXTERN PyOS_sighandler_t(*PyOS_setsig)(int i, PyOS_sighandler_t h);
EXTERN wchar_t* (*Py_DecodeLocale)(const char* arg, size_t* size);
EXTERN char* (*Py_EncodeLocale)(const wchar_t* text, size_t* error_pos);

// System Functions
EXTERN PyObject* (*PySys_GetObject)(const char* name);
EXTERN int (*PySys_SetObject)(const char* name, PyObject* v);
EXTERN void (*PySys_ResetWarnOptions)();
EXTERN void (*PySys_AddWarnOption)(const wchar_t* s);
EXTERN void (*PySys_AddWarnOptionUnicode)(PyObject* unicode);
EXTERN void (*PySys_SetPath)(const wchar_t* path);
EXTERN void (*PySys_WriteStdout)(const char* format, ...);
EXTERN void (*PySys_WriteStderr)(const char* format, ...);
EXTERN void (*PySys_FormatStdout)(const char* format, ...);
EXTERN void (*PySys_FormatStderr)(const char* format, ...);
EXTERN void (*PySys_AddXOption)(const wchar_t* s);
EXTERN PyObject* (*PySys_GetXOptions)();

// Process Control
EXTERN void (*Py_FatalError)(const char* message);
EXTERN void (*Py_Exit)(int status);
EXTERN int (*Py_AtExit)(void (*func)());

// Importing Modules
EXTERN PyObject* (*PyImport_ImportModule)(const char* name);
EXTERN PyObject* (*PyImport_ImportModuleNoBlock)(const char* name);
EXTERN PyObject* (*PyImport_ImportModuleLevelObject)(PyObject* name, PyObject* globals, PyObject* locals, PyObject* fromlist, int level);
EXTERN PyObject* (*PyImport_ImportModuleLevel)(const char* name, PyObject* globals, PyObject* locals, PyObject* fromlist, int level);
EXTERN PyObject* (*PyImport_Import)(PyObject* name);
EXTERN PyObject* (*PyImport_ReloadModule)(PyObject* m);
EXTERN PyObject* (*PyImport_AddModuleObject)(PyObject* name);
EXTERN PyObject* (*PyImport_AddModule)(const char* name);
EXTERN PyObject* (*PyImport_ExecCodeModule)(const char* name, PyObject* co);
EXTERN PyObject* (*PyImport_ExecCodeModuleEx)(const char* name, PyObject* co, const char* pathname);
EXTERN PyObject* (*PyImport_ExecCodeModuleObject)(PyObject* name, PyObject* co, PyObject* pathname, PyObject* cpathname);
EXTERN PyObject* (*PyImport_ExecCodeModuleWithPathnames)(const char* name, PyObject* co, const char* pathname, const char* cpathname);
EXTERN long (*PyImport_GetMagicNumber)();
EXTERN const char* (*PyImport_GetMagicTag)();
EXTERN PyObject* (*PyImport_GetModuleDict)();
EXTERN PyObject* (*PyImport_GetModule)(PyObject* name);
EXTERN PyObject* (*PyImport_GetImporter)(PyObject* path);
EXTERN int (*PyImport_ImportFrozenModuleObject)(PyObject* name);
EXTERN int (*PyImport_ImportFrozenModule)(const char* name);
EXTERN int (*PyImport_AppendInittab)(const char* name, PyObject* (*initfunc)());

// API Functions
EXTERN int (*PyArg_ParseTuple)(PyObject* args, const char* format, ...);
EXTERN int (*PyArg_VaParse)(PyObject* args, const char* format, va_list vargs);
EXTERN int (*PyArg_ParseTupleAndKeywords)(PyObject* args, PyObject* kw, const char* format, char* keywords[], ...);
EXTERN int (*PyArg_VaParseTupleAndKeywords)(PyObject* args, PyObject* kw, const char* format, char* keywords[], va_list vargs);
EXTERN int (*PyArg_ValidateKeywordArguments)(PyObject*);
EXTERN int (*PyArg_Parse)(PyObject* args, const char* format, ...);
EXTERN int (*PyArg_UnpackTuple)(PyObject* args, const char* name, Py_ssize_t min, Py_ssize_t max, ...);

// Building values
EXTERN PyObject* (*Py_BuildValue)(const char* format, ...);
EXTERN PyObject* (*Py_VaBuildValue)(const char* format, va_list vargs);

// String conversion and formatting
EXTERN int (*PyOS_snprintf)(char* str, size_t size, const char* format, ...);
EXTERN int (*PyOS_vsnprintf)(char* str, size_t size, const char* format, va_list va);
EXTERN double (*PyOS_string_to_double)(const char* s, char** endptr, PyObject* overflow_exception);
EXTERN char* (*PyOS_double_to_string)(double val, char format_code, int precision, int flags, int* ptype);

// Reflection
EXTERN PyObject* (*PyEval_GetBuiltins)();
EXTERN PyObject* (*PyEval_GetLocals)();
EXTERN PyObject* (*PyEval_GetGlobals)();
struct PyFrameObject;
EXTERN PyFrameObject* (*PyEval_GetFrame)();
EXTERN PyFrameObject* (*PyFrame_GetBack)(PyFrameObject* frame);
struct PyCodeObject;
EXTERN PyCodeObject* (*PyFrame_GetCode)(PyFrameObject* frame);
EXTERN int (*PyFrame_GetLineNumber)(PyFrameObject* frame);
EXTERN const char* (*PyEval_GetFuncName)(PyObject* func);
EXTERN const char* (*PyEval_GetFuncDesc)(PyObject* func);

// Codec registry and support functions
EXTERN int (*PyCodec_Register)(PyObject* search_function);
EXTERN int (*PyCodec_Unregister)(PyObject* search_function);
EXTERN int (*PyCodec_KnownEncoding)(const char* encoding);
EXTERN PyObject* (*PyCodec_Encode)(PyObject* object, const char* encoding, const char* errors);
EXTERN PyObject* (*PyCodec_Decode)(PyObject* object, const char* encoding, const char* errors);

// Codec lookup API
EXTERN PyObject* (*PyCodec_Encoder)(const char* encoding);
EXTERN PyObject* (*PyCodec_Decoder)(const char* encoding);
EXTERN PyObject* (*PyCodec_IncrementalEncoder)(const char* encoding, const char* errors);
EXTERN PyObject* (*PyCodec_IncrementalDecoder)(const char* encoding, const char* errors);
EXTERN PyObject* (*PyCodec_StreamReader)(const char* encoding, PyObject* stream, const char* errors);
EXTERN PyObject* (*PyCodec_StreamWriter)(const char* encoding, PyObject* stream, const char* errors);

// Registry API for Unicode encoding error handlers
EXTERN int (*PyCodec_RegisterError)(const char* name, PyObject* error);
EXTERN PyObject* (*PyCodec_LookupError)(const char* name);
EXTERN PyObject* (*PyCodec_StrictErrors)(PyObject* exc);
EXTERN PyObject* (*PyCodec_IgnoreErrors)(PyObject* exc);
EXTERN PyObject* (*PyCodec_ReplaceErrors)(PyObject* exc);
EXTERN PyObject* (*PyCodec_XMLCharRefReplaceErrors)(PyObject* exc);
EXTERN PyObject* (*PyCodec_BackslashReplaceErrors)(PyObject* exc);
EXTERN PyObject* (*PyCodec_NameReplaceErrors)(PyObject* exc);

// Object Protocol
EXTERN int (*PyObject_HasAttr)(PyObject* o, PyObject* attr_name);
EXTERN int (*PyObject_HasAttrString)(PyObject* o, const char* attr_name);
EXTERN PyObject* (*PyObject_GetAttr)(PyObject* o, PyObject* attr_name);
EXTERN PyObject* (*PyObject_GetAttrString)(PyObject* o, const char* attr_name);
EXTERN PyObject* (*PyObject_GenericGetAttr)(PyObject* o, PyObject* name);
EXTERN int (*PyObject_SetAttr)(PyObject* o, PyObject* attr_name, PyObject* v);
EXTERN int (*PyObject_SetAttrString)(PyObject* o, const char* attr_name, PyObject* v);
EXTERN int (*PyObject_GenericSetAttr)(PyObject* o, PyObject* name, PyObject* value);
EXTERN PyObject* (*PyObject_GenericGetDict)(PyObject* o, void* context);
EXTERN int (*PyObject_GenericSetDict)(PyObject* o, PyObject* value, void* context);
EXTERN PyObject* (*PyObject_RichCompare)(PyObject* o1, PyObject* o2, int opid);
EXTERN int (*PyObject_RichCompareBool)(PyObject* o1, PyObject* o2, int opid);
EXTERN PyObject* (*PyObject_Repr)(PyObject* o);
EXTERN PyObject* (*PyObject_ASCII)(PyObject* o);
EXTERN PyObject* (*PyObject_Str)(PyObject* o);
EXTERN PyObject* (*PyObject_Bytes)(PyObject* o);
EXTERN int (*PyObject_IsSubclass)(PyObject* derived, PyObject* cls);
EXTERN int (*PyObject_IsInstance)(PyObject* inst, PyObject* cls);
EXTERN Py_hash_t(*PyObject_Hash)(PyObject* o);
EXTERN Py_hash_t(*PyObject_HashNotImplemented)(PyObject* o);
EXTERN int (*PyObject_IsTrue)(PyObject* o);
EXTERN int (*PyObject_Not)(PyObject* o);
EXTERN PyObject* (*PyObject_Type)(PyObject* o);
EXTERN Py_ssize_t(*PyObject_Size)(PyObject* o);
EXTERN Py_ssize_t(*PyObject_Length)(PyObject* o);
EXTERN PyObject* (*PyObject_GetItem)(PyObject* o, PyObject* key);
EXTERN int (*PyObject_SetItem)(PyObject* o, PyObject* key, PyObject* v);
EXTERN int (*PyObject_DelItem)(PyObject* o, PyObject* key);
EXTERN PyObject* (*PyObject_Dir)(PyObject* o);
EXTERN PyObject* (*PyObject_GetIter)(PyObject* o);
EXTERN PyObject* (*PyObject_GetAIter)(PyObject* o);

// Object Calling API
EXTERN PyObject* (*PyObject_Call)(PyObject* callable, PyObject* args, PyObject* kwargs);
EXTERN PyObject* (*PyObject_CallNoArgs)(PyObject* callable);
EXTERN PyObject* (*PyObject_CallObject)(PyObject* callable, PyObject* args);
EXTERN PyObject* (*PyObject_CallFunction)(PyObject* callable, const char* format, ...);
EXTERN PyObject* (*PyObject_CallMethod)(PyObject* obj, const char* name, const char* format, ...);
EXTERN PyObject* (*PyObject_CallFunctionObjArgs)(PyObject* callable, ...);
EXTERN PyObject* (*PyObject_CallMethodObjArgs)(PyObject* obj, PyObject* name, ...);

// Call Support API
EXTERN int (*PyCallable_Check)(PyObject* o);

#define PyCFunction_New(ML, SELF) PyCFunction_NewEx((ML), (SELF), NULL)
EXTERN PyObject* (*PyCFunction_NewEx)(PyMethodDef*, PyObject*, PyObject*);

EXTERN int (*Py_Main)(int argc, wchar_t** argv);
EXTERN int (*Py_BytesMain)(int argc, char** argv);
EXTERN int (**PyOS_InputHook)(void);
EXTERN PyObject* (*Py_CompileString)(const char* str, const char* filename, int start);
struct PyCompilerFlags
{
	int cf_flags;
	int cf_feature_version;
};
EXTERN int (*PyRun_SimpleStringFlags)(const char*, PyCompilerFlags*);
#define PyRun_SimpleString(s) PyRun_SimpleStringFlags(s, NULL)
EXTERN PyObject* (*PyEval_EvalCode)(PyObject* co, PyObject* globals, PyObject* locals);
EXTERN PyObject* (*PyEval_EvalCodeEx)(PyObject* co, PyObject* globals, PyObject* locals, PyObject* const* args, int argcount, PyObject* const* kws, int kwcount, PyObject* const* defs, int defcount, PyObject* kwdefs, PyObject* closure);
EXTERN PyObject* (*PyEval_EvalFrame)(PyFrameObject* f);
EXTERN PyObject* (*PyEval_EvalFrameEx)(PyFrameObject* f, int throwflag);

inline void Py_XINCREF(PyObject* op) {
	if (op != nullptr) {
		Py_IncRef(op);
	}
}

inline void Py_XDECREF(PyObject* op) {
	if (op != nullptr) {
		Py_DecRef(op);
	}
}

#define METH_VARARGS  0x0001

#define Py_single_input 256
#define Py_file_input 257
#define Py_eval_input 258
#define Py_func_type_input 345

struct Symbol {
	void* ptr;
	const char* name;
};
#define X(name) {&name, #name,}
constexpr Symbol symbols[] = {
	X(Py_Initialize),
	X(Py_InitializeEx),
	X(Py_Finalize),
	X(Py_FinalizeEx),
	X(Py_IsInitialized),

	X(Py_GetVersion),
	X(Py_GetCompiler),
	X(Py_GetProgramFullPath),

	X(Py_IncRef),
	X(Py_DecRef),

	X(PyCapsule_GetPointer),
	X(PyCapsule_New),

	X(PyMem_Free),

	X(PyUnicode_AsWideCharString),

	X(PyTuple_New),
	X(PyTuple_Pack),
	X(PyTuple_Size),
	X(PyTuple_GetItem),
	X(PyTuple_GetSlice),
	X(PyTuple_SetItem),

	X(PyStructSequence_NewType),
	X(PyStructSequence_New),
	X(PyStructSequence_GetItem),
	X(PyStructSequence_SetItem),

	X(PyList_New),
	X(PyList_Size),
	X(PyList_GetItem),
	X(PyList_SetItem),
	X(PyList_Insert),
	X(PyList_Append),
	X(PyList_GetSlice),
	X(PyList_SetSlice),
	X(PyList_Sort),
	X(PyList_Reverse),
	X(PyList_AsTuple),

	X(PyDict_New),
	X(PyDict_Clear),
	X(PyDict_Contains),
	X(PyDict_Copy),
	X(PyDict_SetItem),
	X(PyDict_SetItemString),
	X(PyDict_DelItem),
	X(PyDict_DelItemString),
	X(PyDict_GetItem),
	X(PyDict_GetItemWithError),
	X(PyDict_GetItemString),
	X(PyDict_Items),
	X(PyDict_Keys),
	X(PyDict_Values),
	X(PyDict_Size),
	X(PyDict_Next),
	X(PyDict_Merge),
	X(PyDict_Update),
	X(PyDict_MergeFromSeq2),

	X(PySet_New),
	X(PyFrozenSet_New),
	X(PySet_Size),
	X(PySet_Contains),
	X(PySet_Add),
	X(PySet_Discard),
	X(PySet_Pop),
	X(PySet_Clear),

	X(PyFile_FromFd),
	X(PyObject_AsFileDescriptor),
	X(PyFile_GetLine),
	X(PyFile_WriteObject),
	X(PyFile_WriteString),

	X(PyModule_NewObject),
	X(PyModule_New),
	X(PyModule_GetDict),
	X(PyModule_GetNameObject),
	X(PyModule_GetName),
	X(PyModule_GetState),
	X(PyModule_GetDef),
	X(PyModule_GetFilenameObject),
	X(PyModule_GetFilename),
	X(PyModule_Create2),
	X(PyModuleDef_Init),
	X(PyModule_FromDefAndSpec2),
	X(PyModule_ExecDef),
	X(PyModule_SetDocString),
	X(PyModule_AddFunctions),
	X(PyModule_AddObjectRef),
	X(PyModule_AddObject),
	X(PyModule_AddIntConstant),
	X(PyModule_AddStringConstant),
	X(PyModule_AddType),

	X(PyState_FindModule),
	X(PyState_AddModule),
	X(PyState_RemoveModule),

	X(PyErr_PrintEx),
	X(PyErr_Print),
	X(PyErr_BadArgument),
	X(PyErr_Occurred),
	X(PyErr_Fetch),
	X(PyErr_Restore),
	X(PyErr_NormalizeException),

	X(PyLong_AsLong),
	X(PyLong_FromLong),

	X(PyUnicode_AsUTF8String),
	X(PyUnicode_AsUTF8AndSize),
	X(PyUnicode_AsUTF8),
	X(PyUnicode_FromWideChar),

	X(PyOS_FSPath),
	X(PyOS_BeforeFork),
	X(PyOS_AfterFork_Parent),
	X(PyOS_AfterFork_Child),
	X(PyOS_AfterFork),
	X(PyOS_CheckStack),
	X(PyOS_getsig),
	X(PyOS_setsig),
	X(Py_DecodeLocale),
	X(Py_EncodeLocale),

	X(PySys_GetObject),
	X(PySys_SetObject),
	X(PySys_ResetWarnOptions),
	X(PySys_AddWarnOption),
	X(PySys_AddWarnOptionUnicode),
	X(PySys_SetPath),
	X(PySys_WriteStdout),
	X(PySys_WriteStderr),
	X(PySys_FormatStdout),
	X(PySys_FormatStderr),
	X(PySys_AddXOption),
	X(PySys_GetXOptions),

	X(Py_FatalError),
	X(Py_Exit),
	X(Py_AtExit),

	X(PyImport_ImportModule),
	X(PyImport_ImportModuleNoBlock),
	X(PyImport_ImportModuleLevelObject),
	X(PyImport_ImportModuleLevel),
	X(PyImport_Import),
	X(PyImport_ReloadModule),
	X(PyImport_AddModuleObject),
	X(PyImport_AddModule),
	X(PyImport_ExecCodeModule),
	X(PyImport_ExecCodeModuleEx),
	X(PyImport_ExecCodeModuleObject),
	X(PyImport_ExecCodeModuleWithPathnames),
	X(PyImport_GetMagicNumber),
	X(PyImport_GetMagicTag),
	X(PyImport_GetModuleDict),
	X(PyImport_GetModule),
	X(PyImport_GetImporter),
	X(PyImport_ImportFrozenModuleObject),
	X(PyImport_ImportFrozenModule),
	X(PyImport_AppendInittab),

	X(PyArg_ParseTuple),
	X(PyArg_VaParse),
	X(PyArg_ParseTupleAndKeywords),
	X(PyArg_VaParseTupleAndKeywords),
	X(PyArg_ValidateKeywordArguments),
	X(PyArg_Parse),
	X(PyArg_UnpackTuple),

	X(Py_BuildValue),
	X(Py_VaBuildValue),

	X(PyOS_snprintf),
	X(PyOS_vsnprintf),
	X(PyOS_string_to_double),
	X(PyOS_double_to_string),

	X(PyEval_GetBuiltins),
	X(PyEval_GetLocals),
	X(PyEval_GetGlobals),
	X(PyEval_GetFrame),
	X(PyFrame_GetBack),
	X(PyFrame_GetCode),
	X(PyFrame_GetLineNumber),
	X(PyEval_GetFuncName),
	X(PyEval_GetFuncDesc),

	X(PyCodec_Register),
	X(PyCodec_Unregister),
	X(PyCodec_KnownEncoding),
	X(PyCodec_Encode),
	X(PyCodec_Decode),

	X(PyCodec_Encoder),
	X(PyCodec_Decoder),
	X(PyCodec_IncrementalEncoder),
	X(PyCodec_IncrementalDecoder),
	X(PyCodec_StreamReader),
	X(PyCodec_StreamWriter),

	X(PyCodec_RegisterError),
	X(PyCodec_LookupError),
	X(PyCodec_StrictErrors),
	X(PyCodec_IgnoreErrors),
	X(PyCodec_ReplaceErrors),
	X(PyCodec_XMLCharRefReplaceErrors),
	X(PyCodec_BackslashReplaceErrors),
	X(PyCodec_NameReplaceErrors),

	X(PyObject_HasAttr),
	X(PyObject_HasAttrString),
	X(PyObject_GetAttr),
	X(PyObject_GetAttrString),
	X(PyObject_GenericGetAttr),
	X(PyObject_SetAttr),
	X(PyObject_SetAttrString),
	X(PyObject_GenericSetAttr),
	X(PyObject_GenericGetDict),
	X(PyObject_GenericSetDict),
	X(PyObject_RichCompare),
	X(PyObject_RichCompareBool),
	X(PyObject_Repr),
	X(PyObject_ASCII),
	X(PyObject_Str),
	X(PyObject_Bytes),
	X(PyObject_IsSubclass),
	X(PyObject_IsInstance),
	X(PyObject_Hash),
	X(PyObject_HashNotImplemented),
	X(PyObject_IsTrue),
	X(PyObject_Not),
	X(PyObject_Type),
	X(PyObject_Size),
	X(PyObject_Length),
	X(PyObject_GetItem),
	X(PyObject_SetItem),
	X(PyObject_DelItem),
	X(PyObject_Dir),
	X(PyObject_GetIter),
	X(PyObject_GetAIter),

	X(PyObject_Call),
	X(PyObject_CallNoArgs),
	X(PyObject_CallObject),
	X(PyObject_CallFunction),
	X(PyObject_CallMethod),
	X(PyObject_CallFunctionObjArgs),
	X(PyObject_CallMethodObjArgs),

	X(PyCallable_Check),

	X(PyCFunction_NewEx),

	X(Py_Main),
	X(Py_BytesMain),
	X(PyOS_InputHook),
	X(Py_CompileString),
	X(PyRun_SimpleStringFlags),
	X(PyEval_EvalCode),
	X(PyEval_EvalCodeEx),
	X(PyEval_EvalFrame),
	X(PyEval_EvalFrameEx),
};
#undef X

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

PyObject* PyInit_SakuraEditor()
{
	auto module = PyModule_Create(&g_moduleDef);
	return module;
}

std::vector<std::string> g_commandNames;
std::vector<PyMethodDef> g_commandDescs;
std::vector<std::string> g_functionNames;
std::vector<PyMethodDef> g_functionDescs;

PyObject* handleCommand(PyObject* self, PyObject* args)
{
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
	PyObject* none = Py_BuildValue("");
	Py_IncRef(none);
	return none;
}

PyObject* handleFunction(PyObject* self, PyObject* args)
{
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

	for (const auto info : CSMacroMgr::GetCommandInfo()) {
		if (info.m_nFuncID == F_INVALID) {
			break;
		}
		g_commandNames.push_back(to_achar(info.m_pszFuncName));
	}
	for (const auto info : CSMacroMgr::GetFuncInfo()) {
		if (info.m_nFuncID == F_INVALID) {
			break;
		}
		g_functionNames.push_back(to_achar(info.m_pszFuncName));
	}
	for (auto& name : g_commandNames) {
		g_commandDescs.push_back({&name[0], (PyCFunction)handleCommand, METH_VARARGS, nullptr});
	}
	for (auto& name : g_functionNames) {
		g_functionDescs.push_back({&name[0], (PyCFunction)handleFunction, METH_VARARGS, nullptr});
	}

	s_initialized = true;
}

static
void ShowError(CEditView* pView, LPCWSTR lpCaption)
{
	PyObject* ptype, * pvalue, * ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
	if (pvalue) {
		PyObject* str = PyObject_Str(pvalue);
		if (str) {
			Py_ssize_t sz = 0;
			wchar_t* pMsg = PyUnicode_AsWideCharString(str, &sz);
			if (pMsg) {
				MessageBox(pView->GetHwnd(), pMsg, lpCaption, MB_ICONERROR);
				PyMem_Free(pMsg);
			}
			Py_DecRef(str);
		}
	}
	PyErr_Restore(ptype, pvalue, ptraceback);
}

struct PyObjectPtr final {
	PyObjectPtr(PyObject* op) : op(op) {}
	~PyObjectPtr() {
		Py_XDECREF(op);
	}

	PyObjectPtr(const PyObjectPtr&) = delete;
	PyObjectPtr& operator = (const PyObjectPtr&) = delete;

	PyObject* operator = (PyObject* param_op) {
		this->op = param_op;
		return this->op;
	}
	operator PyObject* () {
		return op;
	}
	PyObject* op;
};

bool CPythonMacroManager::ExecKeyMacro(CEditView *EditView, int flags [[maybe_unused]] ) const
{
	static HMODULE s_hModule;
	if (!s_hModule) {
		const wchar_t* dllname = L"python3.dll";
		std::wstring path = dllname;
		auto dir = GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory.c_str();
		if (::PathIsDirectoryW(dir)) {
			std::wstring path2 = std::format(L"{}/{}", dir, dllname);
			if (IsFileExists(path2.c_str())) {
				path = path2;
			}
		}
		s_hModule = LoadLibraryExedir(dllname);
		if (!s_hModule) {
			WCHAR* pMsg;
			::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr,
				::GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPWSTR)&pMsg,
				0,
				nullptr
			);
			CNativeW str(pMsg);
			::LocalFree((HLOCAL)pMsg);
			str.Replace(L"%1", dllname);
			ErrorMessage(nullptr, L"%s", str.GetStringPtr());
			return false;
		}
		for (size_t i = 0; i < _countof(symbols); ++i) {
			auto& s = symbols[i];
			auto sym = ::GetProcAddress(s_hModule, s.name);
			*(void**)s.ptr = (void*)sym;
		}
	}

	if (PyImport_AppendInittab("SakuraEditor", PyInit_SakuraEditor) == -1) {
		fprintf(stderr, "Error: could not extend in-built modules SakuraEditor\n");
		return false;
	}

	Py_InitializeEx(0);

	if (!Py_IsInitialized()) {
		return false;
	}

	bool bSuccess = false;
	do {
		//const char* version = Py_GetVersion();
		//const char* compiler = Py_GetCompiler();
		PyObjectPtr module = PyImport_ImportModule("SakuraEditor");
		if (!module) {
			break;
		}

		for (auto& desc : g_commandDescs) {
			auto cap = PyCapsule_New(&desc, nullptr, nullptr);
			auto fn = PyCFunction_New(&desc, cap);
			if (0 < PyModule_AddObject(module, desc.ml_name, fn)) {
				Py_XDECREF(fn);
				Py_XDECREF(cap);
			}
		}
		for (auto& desc : g_functionDescs) {
			auto cap = PyCapsule_New(&desc, nullptr, nullptr);
			auto fn = PyCFunction_New(&desc, cap);
			if (0 < PyModule_AddObject(module, desc.ml_name, fn)) {
				Py_XDECREF(fn);
				Py_XDECREF(cap);
			}
		}

		PyObjectPtr pCode = Py_CompileString(m_strMacro.c_str(), m_strPath.c_str(), Py_file_input);
		if (!pCode) {
			ShowError(EditView, m_wstrPath.c_str());
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

		PyObjectPtr pObj = PyEval_EvalCode(pCode, pGlobals, pLocals);
		if (!pObj) {
			ShowError(EditView, m_wstrPath.c_str());
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

