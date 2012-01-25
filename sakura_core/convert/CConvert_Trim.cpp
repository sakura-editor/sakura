#include "StdAfx.h"
#include "CConvert_Trim.h"
#include "convert_util.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TRIM Step2
	ConvMemory ���� �߂��Ă�����̏����D
	CMemory.cpp�̂Ȃ��ɒu���Ȃ��ق����ǂ����ȂƎv���Ă�����ɒu���܂����D
	
	@author hor
	@date 2001.12.03 hor    �V�K�쐬
	@date 2007.10.18 kobake CConvert_Trim�Ɉړ�
*/
bool CConvert_Trim::DoConvert(CNativeW* pcData)
{
	const wchar_t*	pLine;
	int			nLineLen;
	wchar_t*	pDes;
	int			nBgn;
	int			i,j;
	int			nPosDes;
	CEol		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ� */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return true;
	}
	pDes = new wchar_t[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	// LTRIM
	if( m_bLeft ){
		while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				for( i = 0; i <= nLineLen; ++i ){
					if( WCODE::IsBlank(pLine[i]) ){
						continue;
					}else{
						break;
					}
				}
				if(nLineLen-i>0){
					wmemcpy( &pDes[nPosDes], &pLine[i], nLineLen );
					nPosDes+=nLineLen-i;
				}
			}
			wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	// RTRIM
	else{
		while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
				i = j = 0;
				while( i < nLineLen ){
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
					if( !WCODE::IsBlank(pLine[i]) ){
						j = i + nCharChars;
					}
					i += nCharChars;
				}
				if(j>0){
					wmemcpy( &pDes[nPosDes], &pLine[0], j );
					nPosDes+=j;
				}
			}
			wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}


