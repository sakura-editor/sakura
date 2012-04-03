/*!	@file
	@brief �C���|�[�g�A�G�N�X�|�[�g�}�l�[�W��

	@author Uchi
	@date 2010/4/22 �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi, Moca

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

#ifndef SAKURA_CIMPEXPMANAGER_H_
#define SAKURA_CIMPEXPMANAGER_H_

#include "CDataProfile.h"
#include "env/DLLSHAREDATA.h"

using std::wstring;

class CImpExpManager
{
public:
	bool ImportUI( HINSTANCE, HWND );
	bool ExportUI( HINSTANCE, HWND );
	virtual bool ImportAscertain( HINSTANCE, HWND, const wstring&, wstring& );
	virtual bool Import( const wstring&, wstring& ) = 0;
	virtual bool Export( const wstring&, wstring& ) = 0;
	// �t�@�C�����̏����l��ݒ�
	void SetBaseName( const wstring& );
	// �t���p�X�����擾
	inline wstring GetFullPath()
	{
		return to_wchar( GetDllShareData().m_sHistory.m_szIMPORTFOLDER ) + m_sOriginName;
	}
	// �t���p�X�����擾
	inline wstring MakeFullPath( wstring sFileName )
	{
		return to_wchar( GetDllShareData().m_sHistory.m_szIMPORTFOLDER ) + sFileName;
	}
	// �t�@�C�������擾
	inline wstring GetFileName()	{ return m_sOriginName; }

protected:
	// Import Folder�̐ݒ�
	inline void SetImportFolder( const TCHAR* szPath ) 
	{
		/* �t�@�C���̃t���p�X���t�H���_�ƃt�@�C�����ɕ��� */
		/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
		::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
		_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
	}

	// �f�t�H���g�g���q�̎擾(�u*.txt�v�`��)
	virtual const TCHAR* GetDefaultExtension();
	// �f�t�H���g�g���q�̎擾(�utxt�v�`��)
	virtual const wchar_t* GetOriginExtension();

protected:
	wstring		m_sBase;
	wstring		m_sOriginName;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �^�C�v�ʐݒ�                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpType : public CImpExpManager
{
public:
	// Constructor
	CImpExpType( int nIdx, STypeConfig& types, HWND hwndList )
		: m_nIdx( nIdx )
		, m_Types( types )
		, m_hwndList( hwndList )
	{
		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
		m_pShareData = &GetDllShareData();
	}

public:
	bool ImportAscertain( HINSTANCE, HWND, const wstring&, wstring& );
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	// �C���^�[�t�F�[�X�p
	int 			m_nIdx;
	STypeConfig&	m_Types;
	HWND			m_hwndList;

	// �����g�p
	DLLSHAREDATA*	m_pShareData;
	int				m_nColorType;
	wstring 		m_sColorFile;
	CDataProfile	m_cProfile;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpColors : public CImpExpManager
{
public:
	// Constructor
	CImpExpColors( ColorInfo * psColorInfoArr )
		: m_ColorInfoArr( psColorInfoArr )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.col"); }
	const wchar_t* GetOriginExtension()	{ return L"col"; }

private:
	ColorInfo*		m_ColorInfoArr;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpRegex : public CImpExpManager
{
public:
	// Constructor
	CImpExpRegex( STypeConfig& types )
		: m_Types( types )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.rkw"); }
	const wchar_t* GetOriginExtension()	{ return L"rkw"; }

private:
	STypeConfig&	m_Types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeyHelp : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeyHelp( STypeConfig& types )
		: m_Types( types )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.txt"); }
	const wchar_t* GetOriginExtension()	{ return L"txt"; }

private:
	STypeConfig&	m_Types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���蓖��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeybind : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeybind( CommonSetting& common )
		: m_Common( common )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.key"); }
	const wchar_t* GetOriginExtension()	{ return L"key"; }

private:
	CommonSetting&		m_Common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpCustMenu : public CImpExpManager
{
public:
	// Constructor
	CImpExpCustMenu( CommonSetting& common )
		: m_Common( common )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.mnu"); }
	const wchar_t* GetOriginExtension()	{ return L"mnu"; }

private:
	CommonSetting&		m_Common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �����L�[���[�h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeyWord : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeyWord( CommonSetting& common, int nKeyWordSetIdx, bool& bCase )
		: m_Common( common )
		, m_nIdx( nKeyWordSetIdx )
		, m_bCase( bCase )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.kwd"); }
	const wchar_t* GetOriginExtension()	{ return L"kwd"; }

private:
	CommonSetting&		m_Common;
	int 				m_nIdx;
	bool&				m_bCase;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���C�����j���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpMainMenu : public CImpExpManager
{
public:
	// Constructor
	CImpExpMainMenu( CommonSetting& common )
		: m_Common( common )
	{
	}

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	CommonSetting&		m_Common;
};


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CIMPEXPMANAGER_H_ */
