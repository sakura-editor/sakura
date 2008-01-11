/*!	@file
	@brief �����R�[�h�������ێ��N���X

	@author Sakura-Editor collaborators
	@date 2006/12/10 �V�K�쐬
*/
/*
	Copyright (C) 2006, rastiv

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

#include "stdafx.h"
#include "charset/CESI.h"
#include "charcode.h"

/*!
	�e�L�X�g�̕����R�[�h���X�L��������D
*/
bool CESI::ScanEncoding( const char* pS, const int nLen )
{
	if( NULL == pS ){
		return false;
	}
	Charcode::GetEncdInf_SJis( pS, nLen, &m_pEI[0] );
	Charcode::GetEncdInf_EucJp( pS, nLen, &m_pEI[1] );
	Charcode::GetEncdInf_Jis( pS, nLen, &m_pEI[2] );
	Charcode::GetEncdInf_Utf8( pS, nLen, &m_pEI[3] );
	Charcode::GetEncdInf_Utf7( pS, nLen, &m_pEI[4] );
	Charcode::GetEncdInf_Uni( pS, nLen, &m_WEI );
	
	return true;
}



/*!
	Unicode �𔻒�.
	
	@retval ���̐��F Unicode �炵���Ɣ��f���ꂽ�D
	        ���̐��F Unicode BE �炵���Ɣ��f���ꂽ�D
	        �[���F   �ǂ���Ƃ������Ȃ��D
	        *pEI_result�F �߂�l���[���ȊO�̏ꍇ�Ɍ���C�ڍ׏���Ԃ��D
*/
int CESI::DetectUnicode( WCCODE_INFO* pEI_result )
{
	int nCRorLF_Uni;	// ���C�h���� Unicode �̉��s�̌�
	int nCRorLF_UniBe;	// ���C�h���� UnicodeBE �̉��s�̌�
	int nCRorLF_mb;		// �}���`�o�C�g�����̉��s�̌�
	int nCRorLF_wc;		// := MAX( nCRorLF_Uni, nCRorLF_UniBe )
	int nUniType;
	int d;
	
	/* �}���`�o�C�g�����̉��s�i0x0d,0x0a�j�̌����`�F�b�N */
	nCRorLF_mb = m_WEI.nCRorLF_ascii;
	if( nCRorLF_mb < 1 ){
		return 0;
	}
	
	/* ���j�R�[�h BOM �̎�ނ𔻒� */
	// nUniType > 0 : ���g���G���f�B�A���炵���D
	// nUniType < 0 : �r�b�O�G���f�B�A���炵��.
	nCRorLF_Uni = m_WEI.Uni.nCRorLF;
	nCRorLF_UniBe = m_WEI.UniBe.nCRorLF;
	d = nUniType = nCRorLF_Uni - nCRorLF_UniBe;
	
	/* ���C�h�����̉��s�i0x0d00,0x0a00 or 0x000d,0x000a�j�̌����擾 */
	nCRorLF_wc = nCRorLF_Uni;
	// d := nCRorLF_Uni - nCRorLF_UniBe
	if( nUniType < 0 ){
		nCRorLF_wc += -d;
	}
	
	/*
		���؃X�e�b�v
	*/
	
	d = nCRorLF_mb - nCRorLF_wc;
	// �����ŁC�K��  nCRorLF_mb > 0 && nCRorLF_mb >= nCRorLF_wc.
	// ������Cd > 0.
	
	if( d < nCRorLF_wc ){
		/* �}���`�o�C�g�����̉��s�����C�h�����̉��s��菭�Ȃ��ꍇ. */
		// BOM �̔��茋�ʂɏ]���D
		if( 0 < nUniType ){
			*pEI_result = m_WEI.Uni;
		}else if( nUniType < 0 ){
			*pEI_result = m_WEI.UniBe;
		}
		return nUniType;
	}else{ // 0 <= nCRorLF_wc <= d
		/* �}���`�o�C�g�����̉��s�����C�h�����̉��s�Ɠ�������葽���ꍇ. */
		// BOM �̔����j������.
		return 0;
	}
}



/*!
	�}���`�o�C�g�n�����R�[�h�𔻒�.
	
	@retval   ����|�C���g�i���L�o�C�g�� �| �s���o�C�g���j�̍ł��傫�����̂�Ԃ��D
	        *pEI_result: ����|�C���g���ł��������������R�[�h�̏ڍׁD
*/
int CESI::DetectMultibyte( MBCODE_INFO* pEI_result )
{
	MBCODE_INFO* ppEI_MostDiff[NUM_OF_MBCODE];
	MBCODE_INFO* pei_tmp;
	int i, j;
	
	/*
		�u���L�o�C�g�� �| �s���o�C�g���v�̐��̑傫�����Ƀ\�[�g�i�o�u���\�[�g�j
	*/
	for( i = 0; i < NUM_OF_MBCODE; i++ ){
		ppEI_MostDiff[i] = &m_pEI[i];
	}
	for( i = 1; i < NUM_OF_MBCODE; i++ ){
		for( j = 0; j < NUM_OF_MBCODE - i; j++ ){
			if( ppEI_MostDiff[j]->nDiff < ppEI_MostDiff[j+1]->nDiff ){
				pei_tmp = ppEI_MostDiff[j+1];
				ppEI_MostDiff[j+1] = ppEI_MostDiff[j];
				ppEI_MostDiff[j] = pei_tmp;
			}
		}
	}
	*pEI_result = *ppEI_MostDiff[0];
	return ppEI_MostDiff[0]->nDiff;
}
