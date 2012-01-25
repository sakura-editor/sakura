#include "StdAfx.h"
#include "CConvert_SpaceToTab.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

//! �󔒁�TAB�ϊ��B�P�Ƃ̃X�y�[�X�͕ϊ����Ȃ�
bool CConvert_SpaceToTab::DoConvert(CNativeW* pcData)
{
	using namespace WCODE;

	const wchar_t*	pLine;
	int			nLineLen;
	wchar_t*	pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
	int			nPosX;
	CEol		cEol;

	BOOL		bSpace = FALSE;	//�X�y�[�X�̏��������ǂ���
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	/* �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ� */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return false;
	}
	pDes = new wchar_t[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			// �擪�s�ɂ��Ă͊J�n���ʒu���l������i����ɐ܂�Ԃ��֘A�̑΍􂪕K�v�H�j
			nPosX = (pcData->GetStringPtr() == pLine)? m_nStartColumn: 0;	// ��������i�ɑΉ�����\�����ʒu
			bSpace = FALSE;	//���O���X�y�[�X��
			nStartPos = 0;	// �X�y�[�X�̐擪
			for( i = 0; i < nLineLen; ++i ){
				if( SPACE == pLine[i] || TAB == pLine[i] ){
					if( bSpace == FALSE ){
						nStartPos = nPosX;
					}
					bSpace = TRUE;
					if( SPACE == pLine[i] ){
						nPosX++;
					}else if( TAB == pLine[i] ){
						nPosX += m_nTabWidth - (nPosX % m_nTabWidth);
					}
				}else{
					if( bSpace ){
						if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
							pDes[nPosDes] = SPACE;
							nPosDes++;
						} else{
							for( j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); j++ ){
								pDes[nPosDes] = TAB;
								nPosDes++;
								nStartPos += m_nTabWidth - ( nStartPos % m_nTabWidth );
							}
							//	2003.08.05 Moca
							//	�ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
							//	�o�b�t�@���͂ݏo���̂��C��
							for( j = nStartPos; j < nPosX; j++ ){
								pDes[nPosDes] = SPACE;
								nPosDes++;
							}
						}
					}
					nPosX++;
					if(WCODE::IsZenkaku(pLine[i])) nPosX++;		//�S�p��������Ή� 2008.10.17 matsumo
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					bSpace = FALSE;
				}
			}
			//for( ; i < nLineLen; ++i ){
			//	pDes[nPosDes] = pLine[i];
			//	nPosDes++;
			//}
			if( bSpace ){
				if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
					pDes[nPosDes] = SPACE;
					nPosDes++;
				} else{
					//for( j = nStartPos - 1; (j + m_nTabWidth) <= nPosX + 1; j+=m_nTabWidth ){
					for( j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); j++ ){
						pDes[nPosDes] = TAB;
						nPosDes++;
						nStartPos += m_nTabWidth - ( nStartPos % m_nTabWidth );
					}
					//	2003.08.05 Moca
					//	�ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
					//	�o�b�t�@���͂ݏo���̂��C��
					for( j = nStartPos; j < nPosX; j++ ){
						pDes[nPosDes] = SPACE;
						nPosDes++;
					}
				}
			}
		}

		/* �s���̏��� */
		auto_memcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}

