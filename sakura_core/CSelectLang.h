/*!	@file
	@brief �e���ꃁ�b�Z�[�W���\�[�X�Ή�

	@author nasukoji
	@date 2011.04.10	�V�K�쐬
*/
/*
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CSELECTLANG_H_
#define _CSELECTLANG_H_

#include <windows.h>
#include <vector>

#define MAX_SELLANG_NAME_STR	128		// ���b�Z�[�W���\�[�X�̌��ꖼ�̍ő啶���񒷁i�T�C�Y�͓K���j

class CSelectLang
{
public:
	// ���b�Z�[�W���\�[�X�p�\����
	struct SSelLangInfo {
		TCHAR szDllName[MAX_PATH];		// ���b�Z�[�W���\�[�XDLL�̃t�@�C����
		TCHAR szLangName[MAX_SELLANG_NAME_STR];		// ���ꖼ
		HINSTANCE hInstance;			// �ǂݍ��񂾃��\�[�X�̃C���X�^���X�n���h��
		WORD wLangId;					// ����ID
		BOOL bValid;					// ���b�Z�[�W���\�[�XDLL�Ƃ��ėL��
	};

protected:
	//static LPTSTR m_szDefaultLang;					// ���b�Z�[�W���\�[�XDLL���ǂݍ��ݎ��̃f�t�H���g����
	static SSelLangInfo* m_psLangInfo;				// ���b�Z�[�W���\�[�X�̏��
public:
	typedef std::vector<SSelLangInfo*> PSSelLangInfoList;
	static PSSelLangInfoList m_psLangInfoList;

public:
	/*
	||  Constructors
	*/
	CSelectLang(){}
	~CSelectLang();

	/*
	||  Attributes & Operations
	*/
	static HINSTANCE getLangRsrcInstance( void );			// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h����Ԃ�
	static LPCTSTR getDefaultLangString( void );			// ���b�Z�[�W���\�[�XDLL���ǂݍ��ݎ��̃f�t�H���g����i"(Japanese)" or "(English(United States))"�j

	static HINSTANCE InitializeLanguageEnvironment(void);		// �����������������
	static HINSTANCE LoadLangRsrcLibrary( SSelLangInfo& lang );	// ���b�Z�[�W�p���\�[�XDLL�����[�h����
	static void ChangeLang( TCHAR* pszDllName );	// �����ύX����

protected:
	/*
	||  �����w���p�֐�
	*/
	static HINSTANCE ChangeLang( UINT nSelIndex );	// �����ύX����

private:
};




/*!
	@brief �����񃊃\�[�X�ǂݍ��݃N���X

	@date 2011.06.01 nasukoji	�V�K�쐬
*/

#define LOADSTR_ADD_SIZE		256			// �����񃊃\�[�X�p�o�b�t�@�̏����܂��͒ǉ��T�C�Y�iTCHAR�P�ʁj

class CLoadString
{
protected:
	// �����񃊃\�[�X�ǂݍ��ݗp�o�b�t�@�N���X
	class CLoadStrBuffer
	{
	public:
		CLoadStrBuffer()
		{
			m_pszString   = m_szString;				// �ϐ����ɏ��������o�b�t�@��ڑ�
			m_nBufferSize = _countof(m_szString);	// �z���
			m_nLength     = 0;
			m_szString[0] = 0;
		}

		/*virtual*/ ~CLoadStrBuffer()
		{
			// �o�b�t�@���擾���Ă����ꍇ�͉������B
			if( m_pszString && m_pszString != m_szString ){
				delete[] m_pszString;
			}
		}

		/*virtual*/ LPCTSTR GetStringPtr() const { return m_pszString; }	// �ǂݍ��񂾕�����̃|�C���^��Ԃ�
		/*virtual*/ int GetBufferSize() const { return m_nBufferSize; }		// �ǂݍ��݃o�b�t�@�̃T�C�Y�iTCHAR�P�ʁj��Ԃ�
		/*virtual*/ int GetStringLength() const { return m_nLength; }		// �ǂݍ��񂾕������iTCHAR�P�ʁj��Ԃ�

		/*virtual*/ int LoadString( UINT uid );								// �����񃊃\�[�X��ǂݍ��ށi�ǂݍ��ݎ��s���j

	protected:
		LPTSTR m_pszString;						// ������ǂݍ��݃o�b�t�@�̃|�C���^
		int m_nBufferSize;						// �擾�z����iTCHAR�P�ʁj
		int m_nLength;							// �擾�������iTCHAR�P�ʁj
		TCHAR m_szString[LOADSTR_ADD_SIZE];		// ������ǂݍ��݃o�b�t�@�i�o�b�t�@�g����͎g�p����Ȃ��j

	private:
		CLoadStrBuffer( const CLoadStrBuffer& );					// �R�s�[�֎~�Ƃ���
		CLoadStrBuffer operator = ( const CLoadStrBuffer& );		// ����֎~�Ƃ���
	};

	static CLoadStrBuffer m_acLoadStrBufferTemp[4];		// ������ǂݍ��݃o�b�t�@�̔z��iCLoadString::LoadStringSt() ���g�p����j
	static int m_nDataTempArrayIndex;					// �Ō�Ɏg�p�����o�b�t�@�̃C���f�b�N�X�iCLoadString::LoadStringSt() ���g�p����j
	CLoadStrBuffer m_cLoadStrBuffer;					// ������ǂݍ��݃o�b�t�@�iCLoadString::LoadString() ���g�p����j

public:
	/*
	||  Constructors
	*/
	CLoadString(){}
	CLoadString( UINT uid ){ LoadString( uid ); }		// ������ǂݍ��ݕt���R���X�g���N�^
	/*virtual*/ ~CLoadString(){}


	/*
	||  Attributes & Operations
	*/
	/*virtual*/ LPCTSTR GetStringPtr() const { return m_cLoadStrBuffer.GetStringPtr(); }	// �ǂݍ��񂾕�����̃|�C���^��Ԃ�
//	/*virtual*/ int GetBufferSize() const { return m_cLoadStrBuffer.GetBufferSize(); }		// �ǂݍ��݃o�b�t�@�̃T�C�Y�iTCHAR�P�ʁj��Ԃ�
	/*virtual*/ int GetStringLength() const { return m_cLoadStrBuffer.GetStringLength(); }	// �ǂݍ��񂾕������iTCHAR�P�ʁj��Ԃ�

	static LPCTSTR LoadStringSt( UINT uid );			// �ÓI�o�b�t�@�ɕ����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j
	/*virtual*/ LPCTSTR LoadString( UINT uid );			// �����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j

protected:

private:
	CLoadString( const CLoadString& );					// �R�s�[�֎~�Ƃ���
	CLoadString operator = ( const CLoadString& );		// ����֎~�Ƃ���
};

// �����񃍁[�h�ȈՉ��}�N��
#define LS( id ) ( CLoadString::LoadStringSt( id ) )
#define LSW( id ) to_wchar( CLoadString::LoadStringSt( id ) )

///////////////////////////////////////////////////////////////////////
#endif /* _CSELECTLANG_H_ */

/*[EOF]*/
