#include "stdafx.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �ϊ��e�[�u��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�S�p�J�i
static const wchar_t tableZenKana_Normal[]=
	L"�A�C�E�G�I�J�L�N�P�R�T�V�X�Z�\�^�`�c�e�g�i�j�k�l�m"	//����
	L"�n�q�t�w�z�}�~����������������������������"			//      ���u���v�u���v�͏��O�B(���p�����݂��Ȃ��̂�)
	L"�@�B�D�F�H�b������"									//��    ���u���v�u���v�u���v�͏��O�B(���p�����݂��Ȃ��̂�)
;
static const wchar_t tableZenKana_Dakuten[]=
	L"���K�M�O�Q�S�U�W�Y�[�]�_�a�d�f�h"						//���_
	L"�o�r�u�x�{" L"\u30f7\u30fa"							//���㔼2�����́u���J�v�u���J�v  ���u���J�v(\u30f8)�u���J�v(\u30f9)�͏��O�B(���p�����݂��Ȃ��̂�)
;
static const wchar_t tableZenKana_HanDakuten[]=
	L"�p�s�v�y�|"											//�����_
;
static const wchar_t tableZenKana_Kigo[]=
	L"�[�J�K�B�A�u�v�E"										//�L��
;

//���p�J�i
static const wchar_t tableHanKana_Normal[]=
	L"�������������������������"
	L"������������������ܦ�"
	L"���������"
;
static const wchar_t tableHanKana_Dakuten[]=
	L"����������������"
	L"�����" L"ܦ"
;
static const wchar_t tableHanKana_HanDakuten[]=
	L"�����"
;
static const wchar_t tableHanKana_Kigo[] =
	L"��ߡ����"
;

//�S�p�p�L���B�����̕��тɐ[���Ӗ��͂���܂���B�o�b�N�X���b�V���͖����B
static const wchar_t tableZenKigo[] =
	L"�{�|���^�����b��"
	L"�O�����G�F"
	L"�g�h�e�f�����i�j�o�p"
	L"�I�H�����`�Q"
;

//���p�p�L��
static const wchar_t tableHanKigo[] =
	L"+-*/%=|&"
	L"^\\@;:"
	L"\"\"''<>(){}"
	L"!?#$~_"
;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	wcschr �̌��ʂ��C���f�b�N�X�Ŏ󂯎��ŁB
	c �����������Ƃ��́A���̈ʒu�� idx �Ɋi�[���Atrue ��Ԃ��B
	������Ȃ��Ƃ��� idx ��ύX������ false ��Ԃ��B
*/
bool wcschr_idx(const wchar_t* str, wchar_t c, int* idx)
{
	const wchar_t* p=wcschr(str,c);
	if(p)*idx=(int)(p-str);
	return p!=NULL;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �ϊ�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//! �S�p�Ђ炪�ȁ��S�p�J�^�J�i (�������͕s��)
void Convert_ZenhiraToZenkana(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		wchar_t& c=*p;

		if(c>=L'��' && c<=L'\u3096'){ c=L'�@'+(c-L'��'); } //��\3096�́u���v�̂Ђ炪�ȔŁB

		p++;
	}
}

//! �S�p�J�^�J�i���S�p�Ђ炪�� (�������͕s��)
void Convert_ZenkanaToZenhira(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		wchar_t& c=*p;

		if(c>=L'�@' && c<=L'��'){ c=L'��'+(c-L'�@'); }

		p++;
	}
}


//! �S�p�p�������p�p�� (�������͕s��)
void Convert_ZenEisuu_To_HanEisuu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	int n;
	while(p<q){
		wchar_t& c=*p;

		if(0){}
		else if(c>=L'�`' && c<=L'�y'){ c=L'A'+(c-L'�`'); }
		else if(c>=L'��' && c<=L'��'){ c=L'a'+(c-L'��'); }
		else if(c>=L'�O' && c<=L'�X'){ c=L'0'+(c-L'�O'); }
		//�ꕔ�̋L�����ϊ�����
		else if(wcschr_idx(tableZenKigo,c,&n)){ c=tableHanKigo[n]; }

		p++;
	}
}


//! ���p�p�����S�p�p�� (�������͕s��)
void Convert_HanEisuu_To_ZenEisuu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	int n;
	while(p<q){
		wchar_t& c=*p;

		if(0){}
		else if(c>=L'A' && c<=L'Z'){ c=L'�`'+(c-L'A'); }
		else if(c>=L'a' && c<=L'z'){ c=L'��'+(c-L'a'); }
		else if(c>=L'0' && c<=L'9'){ c=L'�O'+(c-L'0'); }
		//�ꕔ�̋L�����ϊ�����
		else if(wcschr_idx(tableHanKigo,c,&n)){ c=tableZenKigo[n]; }

		p++;
	}
}


/*!
	�S�p�J�^�J�i�����p�J�^�J�i
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ZenKana_To_HanKana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		if(0){}
		//�q�b�g���镶��������Εϊ����s��
		else if(wcschr_idx(tableZenKana_Normal    ,c,&n)){ *dst++=tableHanKana_Normal[n];                  }
		else if(wcschr_idx(tableZenKana_Dakuten   ,c,&n)){ *dst++=tableHanKana_Dakuten[n];    *dst++=L'�'; }
		else if(wcschr_idx(tableZenKana_HanDakuten,c,&n)){ *dst++=tableHanKana_HanDakuten[n]; *dst++=L'�'; }
		else if(wcschr_idx(tableZenKana_Kigo      ,c,&n)){ *dst++=tableHanKana_Kigo[n];                    }
		//���ϊ�
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
}

/*!
	���p�J�^�J�i���S�p�J�^�J�i
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankanaToZenkana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		wchar_t next=(src+1<src_end)?*(src+1):0; //����1�������ǂ�
		if(0){}
		//���_�A�����_�̃`�F�b�N���s���čs��
		else if(next==L'�' && wcschr_idx(tableHanKana_Dakuten   ,c,&n)){ *dst++=tableZenKana_Dakuten[n];    src++; }
		else if(next==L'�' && wcschr_idx(tableHanKana_HanDakuten,c,&n)){ *dst++=tableZenKana_HanDakuten[n]; src++; }
		//����ȊO�̕����`�F�b�N���s��
		else if(              wcschr_idx(tableHanKana_Normal    ,c,&n)){ *dst++=tableZenKana_Normal[n];            }
		else if(              wcschr_idx(tableHanKana_Kigo      ,c,&n)){ *dst++=tableZenKana_Kigo[n];              }
		//���ϊ�
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
}
