/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_REGKEY_9BB6B5D5_D652_4227_9726_ADB94F2CA44E9_H_
#define SAKURA_REGKEY_9BB6B5D5_D652_4227_9726_ADB94F2CA44E9_H_

class CRegKey
{
protected:
	HKEY _root;
	HKEY _key;
public:
	CRegKey()
	{
		_root = NULL;
		_key = NULL;
	}

	virtual ~CRegKey()
	{
		Close();
	}

	static bool ExistsKey(HKEY root, const TCHAR* path, unsigned int access = KEY_READ)
	{
		CRegKey test;
		return (test.Open(root, path, access) == 0);
	}
	bool IsOpend() const
	{
		return (_key != NULL);
	}
	int Create(HKEY root, const TCHAR* path, unsigned int access = (KEY_READ | KEY_WRITE))
	{
		LONG error = RegCreateKeyEx(root, path, 0, NULL, 0, access, NULL, &_key, NULL);
		if(error != ERROR_SUCCESS)
		{
			return error;
		}
		_root = root;
		return ERROR_SUCCESS;
	}
	int Open(HKEY root, const TCHAR* path, unsigned int access = KEY_READ)
	{
		LONG error = RegOpenKeyEx(root, path, 0, access, &_key);
		if(error != ERROR_SUCCESS)
		{
			return error;
		}
		_root = root;
		return ERROR_SUCCESS;
	}
	void Close()
	{
		if(_key != NULL)
		{
			RegCloseKey(_key);
			_key = NULL;
			_root = NULL;
		}
	}
	int GetValue(const TCHAR* valueName, TCHAR* buffer, unsigned int nMaxChar, int* pGetChars = NULL) const
	{
		DWORD dwType = REG_SZ;
		DWORD nError = 0;
		DWORD getChars = nMaxChar;
		if ((nError = RegQueryValueEx(_key, valueName, NULL, &dwType, (LPBYTE)buffer, &getChars)) != 0)
		{
			return nError;
		}
		if(pGetChars)
		{
			*pGetChars = getChars;
		}
		return ERROR_SUCCESS;
	}
	int GetValueBINARY(const TCHAR* valueName, BYTE* buffer, unsigned int nMaxChar, int* pGetChars = NULL) const
	{
		DWORD dwType = REG_BINARY;
		DWORD nError = 0;
		DWORD getChars = nMaxChar;
		if ((nError = RegQueryValueEx(_key, valueName, NULL, &dwType, (LPBYTE)buffer, &getChars)) != 0)
		{
			return nError;
		}
		if(pGetChars)
		{
			*pGetChars = getChars;
		}
		return ERROR_SUCCESS;
	}
	int SetValue(const TCHAR* valueName, const TCHAR* buffer, int nMaxChar = -1)
	{
		if(nMaxChar == -1)
		{
			nMaxChar = (DWORD)_tcslen(buffer) * sizeof(TCHAR);
		}
		DWORD nError = 0;
		if ((nError = RegSetValueEx(_key, valueName, 0, REG_SZ, (LPBYTE)buffer, nMaxChar)) != 0)
		{
			return nError;
		}
		return ERROR_SUCCESS;
	}
	int SetValue(const TCHAR* valueName, const BYTE* buffer, int nMaxChar, DWORD dwType)
	{
		DWORD nError = 0;
		if ((nError = RegSetValueEx(_key, valueName, 0, dwType, (LPBYTE)buffer, nMaxChar)) != 0)
		{
			return nError;
		}
		return ERROR_SUCCESS;
	}

	int DeleteValue(const TCHAR* valueName)
	{
		return RegDeleteValue(_key, valueName);
	}
	int DeleteSubKey(const TCHAR* path)
	{
		return RegDeleteKey(_key, path);
	}

	int EnumKey(int &index, TCHAR* pNameBuffer, int nMaxChar, int* pGetChar = NULL) const
	{
		if(index < 0)
		{
			return ERROR_NO_MORE_ITEMS;
		}
		DWORD nSize = nMaxChar;
		int nError = RegEnumKeyEx(_key, (DWORD)index, pNameBuffer, &nSize, NULL, NULL, NULL, NULL);
		if(nError != ERROR_SUCCESS)
		{
			index = -1;
			return nError;
		}
		if(pGetChar)
		{
			*pGetChar = nMaxChar;
		}
		return nError;
	}

	int EnumValue(int &index, TCHAR* pNameBuffer, int nMaxChar, DWORD *lpType, BYTE *lpData, int nMaxData, DWORD* pDataLen) const
	{
		if(index < 0)
		{
			return ERROR_NO_MORE_ITEMS;
		}
		DWORD nValueSize = nMaxChar;
		DWORD nDataSize = nMaxChar;
		int nError = RegEnumValue(_key, (DWORD)index, pNameBuffer, &nValueSize, NULL, lpType, lpData, &nDataSize);
		if(nError != ERROR_SUCCESS)
		{
			index = -1;
			return nError;
		}
		if( pDataLen )
		{
			*pDataLen = nDataSize;
		}
		return nError;
	}

	static int DeleteKey(HKEY root, const TCHAR* path)
	{
		return RegDeleteKey(root, path);
	}
};

#endif /* SAKURA_REGKEY_9BB6B5D5_D652_4227_9726_ADB94F2CA44E9_H_ */
/*[EOF]*/
