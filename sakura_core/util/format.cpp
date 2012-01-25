#include "StdAfx.h"
#include "format.h"


/*!	�������t�H�[�}�b�g

	@param[out] �����ϊ���̕�����
	@param[in] �o�b�t�@�T�C�Y
	@param[in] format ����
	@param[in] systime ����������������
	@return bool true

	@note  %Y %y %m %d %H %M %S �̕ϊ��ɑΉ�

	@author aroka
	@date 2005.11.21 �V�K
	
	@todo �o�̓o�b�t�@�̃T�C�Y�`�F�b�N���s��
*/
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime )
{
	TCHAR szTime[10];
	const TCHAR *p = format;
	TCHAR *q = szResult;
	int len;
	
	while( *p ){
		if( *p == _T('%') ){
			++p;
			switch(*p){
			case _T('Y'):
				len = wsprintf(szTime,_T("%d"),systime.wYear);
				_tcscpy( q, szTime );
				break;
			case _T('y'):
				len = wsprintf(szTime,_T("%02d"),(systime.wYear%100));
				_tcscpy( q, szTime );
				break;
			case _T('m'):
				len = wsprintf(szTime,_T("%02d"),systime.wMonth);
				_tcscpy( q, szTime );
				break;
			case _T('d'):
				len = wsprintf(szTime,_T("%02d"),systime.wDay);
				_tcscpy( q, szTime );
				break;
			case _T('H'):
				len = wsprintf(szTime,_T("%02d"),systime.wHour);
				_tcscpy( q, szTime );
				break;
			case _T('M'):
				len = wsprintf(szTime,_T("%02d"),systime.wMinute);
				_tcscpy( q, szTime );
				break;
			case _T('S'):
				len = wsprintf(szTime,_T("%02d"),systime.wSecond);
				_tcscpy( q, szTime );
				break;
				// A Z
			case _T('%'):
			default:
				*q = *p;
				len = 1;
				break;
			}
			q+=len;//q += strlen(szTime);
			++p;
			
		}
		else{
			*q = *p;
			q++;
			p++;
		}
	}
	*q = *p;
	return true;
}

/*!	�o�[�W�����ԍ��̉��

	@param[in] �o�[�W�����ԍ�������
	@return UINT32 8bit�i����1bit+���l7bit�j�����W���[�A�}�C�i�[�A�r���h�A���r�W�������i�[

	@author syat
	@date 2011.03.18 �V�K
	@note �Q�l PHP version_compare http://php.s3.to/man/function.version-compare.html
*/
UINT32 ParseVersion( const TCHAR* sVer )
{
	int nVer;
	int nShift = 0;	//���ʂȕ�����ɂ�鉺��
	int nDigit = 0;	//�A�����鐔���̐�
	UINT32 ret = 0;

	const TCHAR *p = sVer;
	int i;

	for( i=0; *p && i<4; i++){
		//���ʂȕ�����̏���
		if( *p == _T('a') ){
			if( _tcsncmp( _T("alpha"), p, 5 ) == 0 )p += 5;
			else p++;
			nShift = -0x60;
		}
		else if( *p == _T('b') ){
			if( _tcsncmp( _T("beta"), p, 4 ) == 0 )p += 4;
			else p++;
			nShift = -0x40;
		}
		else if( *p == _T('r') || *p == _T('R') ){
			if( _tcsnicmp( _T("rc"), p, 2 ) == 0 )p += 2;
			else p++;
			nShift = -0x20;
		}
		else if( *p == _T('p') ){
			if( _tcsncmp( _T("pl"), p, 2 ) == 0 )p += 2;
			else p++;
			nShift = 0x20;
		}
		else if( !_istdigit(*p) ){
			nShift = -0x80;
		}
		else{
			nShift = 0;
		}
		while( *p && !_istdigit(*p) ){ p++; }
		//���l�̒��o
		for( nVer = 0, nDigit = 0; _istdigit(*p); p++ ){
			if( ++nDigit > 2 )break;	//������2���܂łŎ~�߂�
			nVer = nVer * 10 + *p - _T('0');
		}
		//��؂蕶���̏���
		while( *p && _tcschr( _T(".-_+"), *p ) ){ p++; }

		DebugOut(_T("  VersionPart%d: ver=%d,shift=%d\n"), i, nVer, nShift);
		ret |= ( (nShift + nVer + 128) << (24-8*i) );
	}
	for( ; i<4; i++ ){	//�c��̕�����signed 0 (=0x80)�𖄂߂�
		ret |= ( 128 << (24-8*i) );
	}

#ifdef _UNICODE
	DebugOut(_T("ParseVersion %ls -> %08x\n"), sVer, ret);
#endif
	return ret;
}

/*!	�o�[�W�����ԍ��̔�r

	@param[in] �o�[�W����A
	@param[in] �o�[�W����B
	@return int 0: �o�[�W�����͓������A1�ȏ�: A���V�����A-1�ȉ�: B���V����

	@author syat
	@date 2011.03.18 �V�K
*/
int CompareVersion( const TCHAR* verA, const TCHAR* verB )
{
	UINT32 nVerA = ParseVersion(verA);
	UINT32 nVerB = ParseVersion(verB);

	return nVerA - nVerB;
}
