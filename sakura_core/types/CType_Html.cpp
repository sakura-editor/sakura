#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/CColorStrategy.h"

/* HTML */
//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂݍ��߂�悤�ɂ���
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++��ǉ�	//Mar. 15, 2001 JEPRO .hm��ǉ�
//Feb. 2, 2005 genta �������̂ŃV���O���N�H�[�g�̐F������HTML�ł͍s��Ȃ�
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	_tcscpy( pType->m_szTypeName, _T("HTML") );
	_tcscpy( pType->m_szTypeExts, _T("html,htm,shtml,plg") );

	//�ݒ�
	pType->m_cBlockComments[0].SetBlockCommentRule( L"<!--", L"-->" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nStringType = 0;											/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pType->m_nKeyWordSetIdx[0] = 1;										/* �L�[���[�h�Z�b�g */
	pType->m_eDefaultOutline = OUTLINE_HTML;							/* �A�E�g���C����͕��@ */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;			//�V���O���N�H�[�g�̐F����OFF
}




/*! HTML �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2004.04.19 zenryaku ��v�f�𔻒�
	@date 2004.04.20 Moca �R�����g�����ƁA�s���ȏI���^�O�𖳎����鏈����ǉ�
	@date 2008.08.15 aroka ���o���ƒi���̐[�������ǉ� 2008.09.07�C��
*/
void CDocOutline::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLineBuf;	//	pLineBuf �͍s�S�̂��w���A
	const wchar_t*	pLine;		//	pLine �͏������̕����ȍ~�̕������w���܂��B
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	bool			bEndTag;
	bool			bCommentTag = false;
	bool			bParaTag = false;	//	2008.08.15 aroka

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int		nMaxStack = 32;	//	�l�X�g�̍Ő[
	int				nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t			pszStack[nMaxStack][32];
	wchar_t			szTitle[32];			//	�ꎞ�̈�
	wchar_t			szTag[32];				//	�ꎞ�̈�  �������ŕێ����č��������Ă��܂��B

	enum ELabelType {						//	�񋓑́F���x���̎��
		LT_DEFAULT,		LT_INLINE,		LT_IGNORE,		LT_EMPTY,
		LT_BLOCK,		LT_PARAGRAPH,	LT_HEADING
	};
	enum ELabelType	nLabelType;				// ���x���̎��
	CLogicInt		nLineCount;
	/*	�������o���v�f�ihy�j�����ɏ�ʃ��x���̌��o��(hx)�������܂œ����[���ɂ��낦�܂��B
		���̂��߁A���o���̐[�����L�����Ă����܂��B
		���ʃ��x���̌��o���̐[���͌����܂ŕs��ŁA�O�̏͐߂ł̐[���͉e�����܂���B 2008.08.15 aroka
	*/
	int				nHeadDepth[6+1];		// [0]�� �󂯂Ă���
	for(k=0;k<=6;k++){
		nHeadDepth[k] = -1;
	}
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLineBuf	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( !pLineBuf )
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			pLine = &pLineBuf[i];
			// 2004.04.20 Moca �R�����g����������
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!=L'<' || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine==L'/' )
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1; )
			{
				// �^�O����؂�o��
				// �X�y�[�X�A�^�u�A�u_:-.�p���v�ȊO�̔��p�����A�P�����ڂ́u-.�����v�͔F�߂Ȃ��B
				if( (pLine[j]==L' ' || pLine[j]==L'\t') ||
					(pLine[j]<0x80 && !wcschr(L"_:-.",pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>=L'0' && pLine[j]<=L'9') || pLine[j]==L'-' || pLine[j]==L'.' )) )
				{
					break;
				}
				int nCharSize = CNativeW::GetSizeOfChar(pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize * sizeof(wchar_t));
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here �R�����g����������
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j] = '\0';
			/*	�^�O�̎�ނ��Ƃɏ�����ς���K�v�����邪�A
				�s�x��r����̂̓R�X�g�������̂ŁA�ŏ��ɕ��ނ��Ă����B 2008.08.15 aroka
				��r�̉񐔂��������߁A�������ɕϊ����Ă�����strcmp���g���B
			*/
			wcscpy( szTag, szTitle );
			_wcslwr( szTag );
			
			nLabelType = LT_DEFAULT;
			// �����v�f�i�����ڂ�ς��邽�߂̃^�O�j�͍\����͂��Ȃ��B
			if( !wcscmp(szTag,L"b") || !wcscmp(szTag,L"big") || !wcscmp(szTag,L"blink")
			 || !wcscmp(szTag,L"font") || !wcscmp(szTag,L"i") || !wcscmp(szTag,L"marquee")
			 || !wcscmp(szTag,L"nobr") || !wcscmp(szTag,L"s") || !wcscmp(szTag,L"small")
			 || !wcscmp(szTag,L"strike") || !wcscmp(szTag,L"tt") || !wcscmp(szTag,L"u")
			 || !wcscmp(szTag,L"bdo") || !wcscmp(szTag,L"sub") || !wcscmp(szTag,L"sup") )
 			{
				nLabelType = LT_INLINE;
			}
			// �C�����C���e�L�X�g�v�f�i�e�L�X�g���C������^�O�j�͍\����͂��Ȃ�?
//			if( !wcscmp(szTag,L"abbr") || !wcscmp(szTag,L"acronym") || !wcscmp(szTag,L"dfn")
//			 || !wcscmp(szTag,L"em") || !wcscmp(szTag,L"strong") || !wcscmp(szTag,L"span")
//			 || !wcscmp(szTag,L"code") || !wcscmp(szTag,L"samp") || !wcscmp(szTag,L"kbd")
//			 || !wcscmp(szTag,L"var") || !wcscmp(szTag,L"cite") || !wcscmp(szTag,L"q") )
//			{
//				nLabelType = LT_INLINE;
//			}
			// ���r�v�f�iXHTML1.1�j�͍\����͂��Ȃ��B
			if( !wcscmp(szTag,L"rbc") || !wcscmp(szTag,L"rtc") || !wcscmp(szTag,L"ruby")
			 || !wcscmp(szTag,L"rb") || !wcscmp(szTag,L"rt") || !wcscmp(szTag,L"rp") )
			{
				nLabelType = LT_INLINE;
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W�Ȃ����͍̂\����͂��Ȃ��B
			if( !wcscmp(szTag,L"br") || !wcscmp(szTag,L"base") || !wcscmp(szTag,L"basefont")
			 || !wcscmp(szTag,L"frame") )
			{
				nLabelType = LT_IGNORE;
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W������́B
			if( !wcscmp(szTag,L"area") || !wcscmp(szTag,L"hr") || !wcscmp(szTag,L"img")
			 || !wcscmp(szTag,L"input") || !wcscmp(szTag,L"link") || !wcscmp(szTag,L"meta")
			 || !wcscmp(szTag,L"param") )
			{
				nLabelType = LT_EMPTY;
			}
			if( !wcscmp(szTag,L"div") || !wcscmp(szTag,L"center")
			 || !wcscmp(szTag,L"address") || !wcscmp(szTag,L"blockquote")
			 || !wcscmp(szTag,L"noscript") || !wcscmp(szTag,L"noframes")
			 || !wcscmp(szTag,L"ol") || !wcscmp(szTag,L"ul") || !wcscmp(szTag,L"dl")
			 || !wcscmp(szTag,L"dir") || !wcscmp(szTag,L"menu")
			 || !wcscmp(szTag,L"pre") || !wcscmp(szTag,L"table")
			 || !wcscmp(szTag,L"form") || !wcscmp(szTag,L"fieldset") || !wcscmp(szTag,L"isindex") )
			{
				nLabelType = LT_BLOCK;
			}
			if( !wcscmp(szTag,L"p") )
			{
				nLabelType = LT_PARAGRAPH;
			}
			if( (szTag[0]==L'h') && (L'1'<=szTitle[1]&&szTitle[1]<=L'6') ){
				nLabelType = LT_HEADING;
			}

			if( bEndTag ) // �I���^�O
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca �ǉ�
				while(nDepth>0)
				{
					nDepth--;
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca �c���[���ƈ�v���Ȃ��Ƃ��́A���̏I���^�O�͖���
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}else{
					if( nLabelType==LT_HEADING ){	//	���o���̏I���
						nHeadDepth[szTitle[1]-L'0'] = nDepth;
						nDepth++;
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = false;
					}
				}
			} else { // �J�n�^�O
				if( nLabelType!=LT_INLINE && nLabelType!=LT_IGNORE ){
					// p�̒��Ńu���b�N�v�f��������A�����I��p�����B 2008.09.07 aroka
					if( bParaTag ){
						if( nLabelType==LT_HEADING || nLabelType==LT_PARAGRAPH || nLabelType==LT_BLOCK ){
							nDepth--;
						}
					}
					if( nLabelType==LT_HEADING ){
						if( nHeadDepth[szTitle[1]-L'0'] != -1 ) // �����o��:���o
						{
							nDepth = nHeadDepth[szTitle[1]-L'0'];
							for(k=szTitle[1]-L'0';k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-L'0'] = nDepth;
							bParaTag = false;
						}
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = true;
					}
					if( nLabelType==LT_BLOCK ){
						bParaTag = false;
					}

					CLayoutPoint ptPos;

					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if( nLabelType!=LT_EMPTY ){
						// �I���^�O�Ȃ��������S�Ẵ^�O�炵�����̂𔻒�
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if( pLine[j]==L'/' && pLine[j+1]==L'>' )
								{
									bEndTag = true;
									break;
								}
								else if( pLine[j]==L'>' )
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if( pLine[j+k]==L'<' || pLine[j+k]==L'\r' || pLine[j+k]==L'\n' )
									{
										break;
									}
									szTitle[k] = pLine[j+k];
								}
								j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptPos.GetY2()+CLayoutInt(1), szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[j]==L'>')
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}



const wchar_t* g_ppszKeywordsHTML[] = {
	L"_blank",
	L"_parent",
	L"_self",
	L"_top",
	L"A",
	L"ABBR",
	L"ABOVE",
	L"absbottom",
	L"absmiddle",
	L"ACCESSKEY",
	L"ACRONYM",
	L"ACTION",
	L"ADDRESS",
	L"ALIGN",
	L"all",
	L"APPLET",
	L"AREA",
	L"AUTOPLAY",
	L"AUTOSTART",
	L"B",
	L"BACKGROUND",
	L"BASE",
	L"BASEFONT",
	L"baseline",
	L"BEHAVIOR",
	L"BELOW",
	L"BGCOLOR",
	L"BGSOUND",
	L"BIG",
	L"BLINK",
	L"BLOCKQUOTE",
	L"BODY",
	L"BORDER",
	L"BORDERCOLOR",
	L"BORDERCOLORDARK",
	L"BORDERCOLORLIGHT",
	L"BOTTOM",
	L"box",
	L"BR",
	L"BUTTON",
	L"CAPTION",
	L"CELLPADDING",
	L"CELLSPACING",
	L"CENTER",
	L"CHALLENGE",
	L"char",
	L"checkbox",
	L"CHECKED",
	L"CITE",
	L"CLEAR",
	L"CLIP",
	L"CODE",
	L"CODEBASE",
	L"CODETYPE",
	L"COL",
	L"COLGROUP",
	L"COLOR",
	L"COLS",
	L"COLSPAN",
	L"COMMENT",
	L"CONTROLS",
	L"DATA",
	L"DD",
	L"DECLARE",
	L"DEFER",
	L"DEL",
	L"DELAY",
	L"DFN",
	L"DIR",
	L"DIRECTION",
	L"DISABLED",
	L"DIV",
	L"DL",
	L"DOCTYPE",
	L"DT",
	L"EM",
	L"EMBED",
	L"ENCTYPE",
	L"FACE",
	L"FIELDSET",
	L"file",
	L"FONT",
	L"FOR",
	L"FORM",
	L"FRAME",
	L"FRAMEBORDER",
	L"FRAMESET",
	L"GET",
	L"groups",
	L"GROUPS",
	L"GUTTER",
	L"H1",
	L"H2",
	L"H3",
	L"H4",
	L"H5",
	L"H6",
	L"H7",
	L"HEAD",
	L"HEIGHT",
	L"HIDDEN",
	L"Hn",
	L"HR",
	L"HREF",
	L"hsides",
	L"HSPACE",
	L"HTML",
	L"I",
	L"ID",
	L"IFRAME",
	L"ILAYER",
	L"image",
	L"IMG",
	L"INDEX",
	L"inherit",
	L"INPUT",
	L"INS",
	L"ISINDEX",
	L"JavaScript",
	L"justify",
	L"KBD",
	L"KEYGEN",
	L"LABEL",
	L"LANGUAGE",
	L"LAYER",
	L"LEFT",
	L"LEGEND",
	L"lhs",
	L"LI",
	L"LINK",
	L"LISTING",
	L"LOOP",
	L"MAP",
	L"MARQUEE",
	L"MAXLENGTH",
	L"MENU",
	L"META",
	L"METHOD",
	L"METHODS",
	L"MIDDLE",
	L"MULTICOL",
	L"MULTIPLE",
	L"NAME",
	L"NEXT",
	L"NEXTID",
	L"NOBR",
	L"NOEMBED",
	L"NOFRAMES",
	L"NOLAYER",
	L"none",
	L"NOSAVE",
	L"NOSCRIPT",
	L"NOTAB",
	L"NOWRAP",
	L"OBJECT",
	L"OL",
	L"onBlur",
	L"onChange",
	L"onClick",
	L"onFocus",
	L"onLoad",
	L"onMouseOut",
	L"onMouseOver",
	L"onReset",
	L"onSelect",
	L"onSubmit",
	L"OPTION",
	L"P",
	L"PAGEX",
	L"PAGEY",
	L"PALETTE",
	L"PANEL",
	L"PARAM",
	L"PARENT",
	L"password",
	L"PLAINTEXT",
	L"PLUGINSPAGE",
	L"POST",
	L"PRE",
	L"PREVIOUS",
	L"Q",
	L"radio",
	L"REL",
	L"REPEAT",
	L"reset",
	L"REV",
	L"rhs",
	L"RIGHT",
	L"rows",
	L"ROWSPAN",
	L"RULES",
	L"S",
	L"SAMP",
	L"SAVE",
	L"SCRIPT",
	L"SCROLLAMOUNT",
	L"SCROLLDELAY",
	L"SELECT",
	L"SELECTED",
	L"SERVER",
	L"SHAPES",
	L"show",
	L"SIZE",
	L"SMALL",
	L"SONG",
	L"SPACER",
	L"SPAN",
	L"SRC",
	L"STANDBY",
	L"STRIKE",
	L"STRONG",
	L"STYLE",
	L"SUB",
	L"submit",
	L"SUMMARY",
	L"SUP",
	L"TABINDEX",
	L"TABLE",
	L"TARGET",
	L"TBODY",
	L"TD",
	L"TEXT",
	L"TEXTAREA",
	L"textbottom",
	L"TEXTFOCUS",
	L"textmiddle",
	L"texttop",
	L"TFOOT",
	L"TH",
	L"THEAD",
	L"TITLE",
	L"TOP",
	L"TR",
	L"TT",
	L"TXTCOLOR",
	L"TYPE",
	L"U",
	L"UL",
	L"URN",
	L"USEMAP",
	L"VALIGN",
	L"VALUE",
	L"VALUETYPE",
	L"VAR",
	L"VISIBILITY",
	L"void",
	L"vsides",
	L"VSPACE",
	L"WBR",
	L"WIDTH",
	L"WRAP",
	L"XMP"
};
int g_nKeywordsHTML = _countof(g_ppszKeywordsHTML);
