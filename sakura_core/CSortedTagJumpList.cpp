/*!	@file
	@brief タグジャンプリスト

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta

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

#include "StdAfx.h"
#include "CSortedTagJumpList.h"

/*!
	@date 2005.04.23 genta 管理数の最大値を指定する引数追加
*/
CSortedTagJumpList::CSortedTagJumpList(int max)
    : m_pTagjump(NULL)
    , m_nCount(0)
    , m_bOverflow(false)
    , m_MAX_TAGJUMPLIST(max)
{
    // id==0 を 空文字列にする
    m_baseDirArr.push_back(L"");
}

CSortedTagJumpList::~CSortedTagJumpList()
{
    Empty();
}

/*
	指定されたアイテムのメモリを解放する。

	@param[in] item 削除するアイテム
*/
void CSortedTagJumpList::Free(TagJumpInfo *item)
{
    free(item->keyword);
    free(item->filename);
    free(item->note);
    free(item);
    return;
}

/*
	リストをすべて解放する。
*/
void CSortedTagJumpList::Empty(void)
{
    TagJumpInfo *p;
    TagJumpInfo *next;
    for (p = m_pTagjump; p; p = next)
    {
        next = p->next;
        Free(p);
    }
    m_pTagjump  = NULL;
    m_nCount    = 0;
    m_bOverflow = false;
    m_baseDirArr.clear();
    m_baseDirArr.push_back(L"");
}

/*
	基準フォルダを登録し、基準フォルダIDを取得
	@date 2010.07.23 Moca 新規追加
*/
int CSortedTagJumpList::AddBaseDir(const WCHAR *baseDir)
{
    m_baseDirArr.push_back(baseDir);
    return m_baseDirArr.size() - 1;
}

/*
	アイテムをソートされた状態でリストに追加する。
	アイテムが最大数を超える場合は、超えるアイテムを削除する。
	文字列はコピーを作成するので、呼び出し側は文字列のアドレス先を保持する必要はない。
	
	@param[in] keyword	キーワード
	@param[in] filename	ファイル名
	@param[in] no		行番号
	@param[in] type		種類
	@param[in] note		備考
	@param[in] depth	(さかのぼる)階層
	@param[in] baseDirId	基準フォルダID。0で空文字列指定 (AddBaseDirの戻り値)
	@retval TRUE  追加した
	@retval FALSE 追加失敗
	@date 2010.07.23 Moca baseDirId 追加
*/
BOOL CSortedTagJumpList::AddParamA(const ACHAR *keyword, const ACHAR *filename, int no, ACHAR type, const ACHAR *note, int depth, int baseDirId)
{
    //アイテムを作成する。
    TagJumpInfo *item = (TagJumpInfo *)malloc(sizeof(TagJumpInfo));
    if (NULL == item)
        return FALSE;
    item->keyword   = _wcsdup(to_wchar(keyword));
    item->filename  = _wcsdup(to_wchar(filename));
    item->no        = no;
    item->type      = type;
    item->note      = _wcsdup(to_wchar(note));
    item->depth     = depth;
    item->next      = NULL;
    item->baseDirId = baseDirId;

    //文字列長ガード
    if (wcslen(item->keyword) >= MAX_TAG_STRING_LENGTH)
        item->keyword[MAX_TAG_STRING_LENGTH - 1] = 0;
    if (wcslen(item->filename) >= MAX_TAG_STRING_LENGTH)
        item->filename[MAX_TAG_STRING_LENGTH - 1] = 0;
    if (wcslen(item->note) >= MAX_TAG_STRING_LENGTH)
        item->note[MAX_TAG_STRING_LENGTH - 1] = 0;

    TagJumpInfo *p;
    TagJumpInfo *prev = NULL;

    //アイテムをリストの適当な位置に追加する。
    for (p = m_pTagjump; p; p = p->next)
    {
        if (wcscmp(p->keyword, item->keyword) > 0)
            break;
        prev = p;
    }
    item->next = p;
    if (prev)
        prev->next = item;
    else
        m_pTagjump = item;
    m_nCount++;

    //最大数を超えたら最後のアイテムを削除する。
    if (m_nCount > m_MAX_TAGJUMPLIST)
    {
        prev = NULL;
        for (p = m_pTagjump; p->next; p = p->next)
            prev = p;
        if (prev)
            prev->next = NULL;
        else
            m_pTagjump = NULL;
        Free(p);
        m_nCount--;
        m_bOverflow = true;
    }
    return TRUE;
}

/*
	指定の情報を取得する。

	@param[out] keyword		キーワード
	@param[out] filename	ファイル名
	@param[out] no			行番号
	@param[out] type		種類
	@param[out] note		備考
	@param[out] depth		(さかのぼる)階層
	@param[out] baseDir		ファイル名の基準フォルダ
	@return 処理結果

	@note 不要な情報の場合は引数に NULL を指定する。
*/
BOOL CSortedTagJumpList::GetParam(int index, WCHAR *keyword, WCHAR *filename, int *no, WCHAR *type, WCHAR *note, int *depth, WCHAR *baseDir)
{
    if (keyword)
        keyword[0] = L'\0';
    if (filename)
        filename[0] = L'\0';
    if (no)
        *no = 0;
    if (type)
        *type = 0;
    if (note)
        note[0] = L'\0';
    if (depth)
        *depth = 0;
    if (baseDir)
        baseDir[0] = L'\0';

    CSortedTagJumpList::TagJumpInfo *p;
    p = GetPtr(index);
    if (NULL != p)
    {
        if (keyword)
            wcscpy(keyword, p->keyword);
        if (filename)
            wcscpy(filename, p->filename);
        if (no)
            *no = p->no;
        if (type)
            *type = p->type;
        if (note)
            wcscpy(note, p->note);
        if (depth)
            *depth = p->depth;
        if (baseDir)
        {
            if (0 <= p->baseDirId && (size_t)p->baseDirId < m_baseDirArr.size())
            {
                wcscpy(baseDir, m_baseDirArr[p->baseDirId].c_str());
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*
	指定の情報を構造体ポインタで取得する。
	取得した情報は参照なので解放してはならない。

	@param[in] index 要素番号
	@return タグジャンプ情報
*/
CSortedTagJumpList::TagJumpInfo *CSortedTagJumpList::GetPtr(int index)
{
    TagJumpInfo *p;
    int i;
    i = 0;
    for (p = m_pTagjump; p; p = p->next)
    {
        if (index == i)
            return p;
        i++;
    }
    return NULL;
}
