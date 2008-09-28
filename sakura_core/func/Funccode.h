/*!	@file
	@brief �@�\�ԍ���`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, MIK, Stonee, asa-o, Misaka, hor
	Copyright (C) 2002, aroka, YAZAKI, minfu, MIK, ai, genta
	Copyright (C) 2003, �S, genta, MIK, Moca
	Copyright (C) 2004, genta, zenryaku, kazika, Moca, isearch
	Copyright (C) 2005, genta, MIK, maru
	Copyright (C) 2006, aroka, �����, fon, ryoji
	Copyright (C) 2007, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _FUNCCODE_H_
#define _FUNCCODE_H_

//Oct. 17, 2000 jepro  F_FILECLOSE:�u�t�@�C�������v�Ƃ����L���v�V������ύX
//Feb. 28, 2004 genta  F_FILESAVECLOSE
//Jan. 24, 2005 genta  F_FILESAVEALL
//Jan. 24, 2005 genta  F_FILESAVE_QUIET
//2007.02.13    ryoji  F_EXITALLEDITORS�ǉ�
//Sep. 14, 2000 jepro  F_PRINT_PAGESETUP:�u����̃y�[�W���C�A�E�g�̐ݒ�v����u����y�[�W�ݒ�v�ɕύX 
//Feb.  9, 2001 jepro  F_OPEN_HHPP�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
//Feb.  9, 2001 jepro  F_OPEN_CCPP�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
//Feb.  7, 2001 JEPRO  F_OPEN_HfromtoC �ǉ�
//Sep. 17, 2000 jepro  F_PLSQL_COMPILE_ON_SQLPLUS �����́u�R���p�C���v���u���s�v�ɓ���
//Dec. 27, 2000 JEPRO  F_EXITALL�ǉ�
//2007.09.02 kobake �p�����[�^��wchar_t�ŗ��邱�Ƃ���������悤�A���O�� F_CHAR ���� F_WCHAR �ɕύX
//2001.12.03 hor    F_LTRIM
//2001.12.03 hor    F_RTRIM
//2001.12.06 hor    F_SORT_ASC
//2001.12.06 hor    F_SORT_DESC
//2001.12.06 hor    F_MERGE
//2002.04.09 minfu  F_RECONVERT
//Dec. 4, 2002 genta F_FILE_REOPEN�`
//Oct. 10, 2000 JEPRO F_ROLLDOWN     ���̔��y�[�W�A�b�v�ɖ��̕ύX
//Oct. 10, 2000 JEPRO F_ROLLUP       ���̔��y�[�W�_�E���ɖ��̕ύX
//Oct.  6, 2000 JEPRO F_HalfPageUp   ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//Oct.  6, 2000 JEPRO F_HalfPageDown ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//Oct. 10, 2000 JEPRO F_1PageUp      �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
//Oct. 10, 2000 JEPRO F_1PageDown    �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
//2001/06/20 asa-o F_WndScrollDown
//2001/06/20 asa-o F_WndScrollUp
//Oct. 10, 2000 JEPRO F_ROLLDOWN_SEL     ���̔��y�[�W�A�b�v�ɖ��̕ύX
//Oct. 10, 2000 JEPRO F_ROLLUP_SEL       ���̔��y�[�W�_�E���ɖ��̕ύX
//Oct.  6, 2000 JEPRO F_HalfPageUp_Sel   ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//Oct.  6, 2000 JEPRO F_HalfPageDown_Sel ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//Oct. 10, 2000 JEPRO F_1PageUp_Sel      �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
//Oct. 10, 2000 JEPRO F_1PageDown_Sel    �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
//2002/2/3 aroka F_COPYFNAME
//Sept. 15, 2000 JEPRO F_CREATEKEYBINDLIST ��̍s��IDM_TEST�̂܂܂ł͂��܂������Ȃ��̂�F�ɕς��ēo�^
//2002.06.02     MIK    F_CTRL_CODE_DIALOG			
//Aug. 29, 2002  ai     F_TOHANKATA				 	
//Sep. 17, 2000  jepro  F_TOZENKAKUKATA �������u���p���S�p�J�^�J�i�v����ύX
//Sep. 17, 2000  jepro  F_TOZENKAKUHIRA �������u���p���S�p�Ђ炪�ȁv����ύX
//Jly. 30, 2001  Misaka F_TOZENEI					
//2002.2.11      YAZAKI F_TOHANEI							
//2001/05/27     Stonee F_SPACETOTAB		
//Oct. 17, 2000  jepro  F_UUDECODE �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX
//02/06/26 ai F_JUMP_SRCHSTARTPOS		
//2002.06.15 MIK F_SEARCH_BOX			
//20060201 aroka F_OUTLINE_TOGGLE	 
//2003.04.13 MIK F_TAGJUMP_LIST			
//2003.04.13 MIK F_TAGS_MAKE			
//2003.04.15 MIK F_DIRECT_TAGJUMP		
//Apr. 03. 2003 genta F_TAGJUMP_CLOSE		 
//2005.03.31 MIK F_TAGJUMP_KEYWORD		
//2002.05.25 MIK F_DIFF_DIALOG			
//2002.05.25 MIK F_DIFF					
//2002.05.25 MIK F_DIFF_NEXT			
//2002.05.25 MIK F_DIFF_PREV			
//2002.05.25 MIK F_DIFF_RESET		
//2004.10.13 isearch F_ISEARCH_NEXT	    
//2004.10.13 isearch F_ISEARCH_PREV	 
//2004.10.13 isearch F_ISEARCH_REGEXP_NEXT   
//2004.10.13 isearch F_ISEARCH_REGEXP_PREV  
//2004.10.13 isearch F_ISEARCH_MIGEMO_NEXT   
//2004.10.13 isearch F_ISEARCH_MIGEMO_PREV    
//2003.06.08 Moca F_CHGMOD_EOL_CRLF
//2003.06.08 Moca F_CHGMOD_EOL_LF
//2003.06.08 Moca F_CHGMOD_EOL_CR
//2003.06.23 Moca F_CHGMOD_EOL           
//2005.01.29 genta F_SET_QUOTESTRING     	
//2006.04.26 maru F_TRACEOUT            	
//2003.06.10 MIK F_SHOWTAB			 	
//2003.04.08 MIK F_FAVORITE				
//Stonee, 2001/05/18 �ݒ�_�C�A���O�p�̋@�\�ԍ���p�� F_TYPE_SCREEN�`F_OPTION_HELPER
//Jul. 03, 2001 JEPRO �ǉ� F_TYPE_HELPER		 	
//2001.11.17 add MIK F_TYPE_REGEX_KEYWORD	 	
//2006.10.06 ryoji �ǉ� F_TYPE_KEYHELP		 	
//Feb. 11, 2007 genta �ǉ� F_OPTION_TAB		 
//Oct. 7, 2000 JEPRO  F_WRAPWINDOWWIDTH  WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX F_SPLIT_V				
//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX F_SPLIT_H				
//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ� F_SPLIT_VH				
//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL) F_WIN_CLOSEALL			
//2004.07.14 kazika F_BIND_WINDOW			
//2004-09-21 Moca F_TOPMOST			
//2006.03.23 fon F_WINLIST			
//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ��� F_MINIMIZE_ALL				
//2001.02.10 by MIK F_MAXIMIZE_H			
//2007.06.20 ryoji F_GROUPCLOSE	
//2007.06.20 ryoji F_NEXTGROUP			
//2007.06.20 ryoji F_PREVGROUP			
//2007.06.20 ryoji F_TAB_MOVERIGHT	
//2007.06.20 ryoji F_TAB_MOVELEFT	
//2007.06.20 ryoji F_TAB_SEPARATE		
//2007.06.20 ryoji F_TAB_JOINTNEXT			
//2007.06.20 ryoji F_TAB_JOINTPREV			
//2006.03.24 fon F_TOGGLE_KEY_SEARCH		
//Nov. 25, 2000 JEPRO �ǉ� F_HELP_CONTENTS			
//Nov. 25, 2000 JEPRO �ǉ� F_HELP_SEARCH			
//Dec. 24, 2000 JEPRO �ǉ� F_ABOUT					
//Oct. 19, 2002 genta F_GETSELECTED
//2003-02-21 �S F_EXPANDPARAMETER 
//2003.06.25 Moca F_GETLINESTR       
//2003.06.25 Moca F_GETLINECOUNT    
//2004.03.16 zenryaku F_CHGTABWIDTH     
//2005.7.30 maru F_ISTEXTSELECTED  
//2005.7.30 maru F_GETSELLINEFROM  
//2005.7.30 maru F_GETSELCOLMFROM   
//2005.7.30 maru F_GETSELLINETO    
//2005.7.30 maru F_GETSELCOLMTO    
//2005.7.30 maru F_ISINSMODE        
//2005.07.31 maru F_GETCHARCODE      
//2005.08.05 maru F_GETLINECODE      
//2005.08.05 maru F_ISPOSSIBLEUNDO   
//2005.08.05 maru F_ISPOSSIBLEREDO   

/*
�����̈�����
  CEditView::HandleCommand
  CMacro::HandleFunction
  MacroFuncInfo CSMacroMgr::m_MacroFuncInfoNotCommandArr[]
���Q��
*/

//2007.09.30 kobake �@�\�ԍ��萔��񋓌^�ɕύX�B(�f�o�b�O�����₷�����邽��)
#include "Funccode_enum.h"

// �@�\�ꗗ�Ɋւ���f�[�^�錾
namespace nsFuncCode{
	extern const TCHAR*			ppszFuncKind[];
	extern const int			nFuncKindNum;
	extern const int			pnFuncListNumArr[];
	extern const EFunctionCode*	ppnFuncListArr[];
	extern const int			nFincListNumArrNum;
};
///////////////////////////////////////////////////////////////////////


/*�@�\�ԍ��ɑΉ������w���v�g�s�b�NID��Ԃ�*/
SAKURA_CORE_API int FuncID_To_HelpContextID( EFunctionCode nFuncID );	//Stonee, 2001/02/23

class CEditDoc;
struct DLLSHAREDATA;

//2007.10.30 kobake �@�\�`�F�b�N��CEditWnd���炱���ֈړ�
bool IsFuncEnable( CEditDoc*, DLLSHAREDATA*, EFunctionCode );	/* �@�\�����p�\�����ׂ� */
bool IsFuncChecked( CEditDoc*, DLLSHAREDATA*, EFunctionCode );	/* �@�\���`�F�b�N��Ԃ����ׂ� */

#endif // _FUNCCODE_H_
