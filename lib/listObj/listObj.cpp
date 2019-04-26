//
//  listObj.cpp
//  
//
//  Created by Michael Hartmann on 11/27/15.
//
//

#include "listObj.h"
#include <stdio.h>
#include <string.h>


listObj::listObj() :
pRoot(NULL),currentIndex(0),size(0)
{
}

listObj::~listObj()
{
    clear();
}

listObj::tListError listObj::checkValidIndex(int pos)
{
    if ((pos>0) && (pos<size))
        return LIST_OK;    
    else
        return LIST_INVALID_INDEX;   
           
}

listObj::tListError listObj::clear(void)
{
    //Check for Empty List
    if (pRoot == NULL)
        return LIST_EMPTY;

    //Remove entries
    for(int ii=1;ii<=size;ii++)
        remove(1);

    //Reset Everything
    size = 0;
    currentIndex = 0;
    pRoot = NULL;
    return LIST_OK;
}

listObj::tListError listObj::add(tEntry entry)
{
    return insert(size, entry);
}

listObj::tListError listObj::insert(int pos, tEntry entry)
{
    tEntry* pIndex;
    tEntry* pNew;

    getByIndex(pos,&pIndex);

    pNew = new tEntry;
    if (pNew)
    {
        //Copy data
        pNew->num = entry.num;
        strncpy(pNew->pStr, entry.pStr, MAXSTR);
        pNew->pPtr = entry.pPtr;
        pNew->pNext = NULL;

        //Case 1: List is empty, create root
        if(pRoot == NULL)
        {
            pRoot = pNew; 
        }        
        //Case 2: Insert/add
        else
        {
            //Link List insert
            pNew->pNext = pIndex->pNext;
            pIndex->pNext = pNew;   
        }
        
        //update reference
        size++;
        currentIndex++;
        return LIST_OK;
    }
    else
    {
        return LIST_COULDNT_ADD;
    }
}


listObj::tListError listObj::addData(int num, char* pStr)
{
    tEntry entry;

    entry.num = num;
    strncpy(entry.pStr,pStr,sizeof(entry.pStr));
    return add(entry);
}


listObj::tListError listObj::insertData(int pos, int num, char* pStr)
{
    tEntry entry;

    entry.num = num;
    strncpy(entry.pStr,pStr,MAXSTR);
    return insert(pos,entry);
}


//Check!!!
listObj::tListError listObj::remove(int pos)
{
    tEntry* pPrev = pRoot;
    tEntry* pNext = pRoot;
    currentIndex = 0;
    if (checkValidIndex(pos) != LIST_OK)
        return LIST_INVALID_INDEX;
    if (pRoot == NULL)
        return LIST_EMPTY;

    currentIndex = 1;
    for(int ii = 1;ii<pos;ii++)
    {
    	pPrev = pNext;
    	getNext(&pNext);
    	if(pNext == NULL)
    	    return LIST_INVALID_INDEX;
    }
    pPrev->pNext = pNext->pNext;
    delete pNext;
    size--;
    return LIST_OK;
}


listObj::tListError listObj::insertByAlpha(int num, char* pStr)
{
    tEntry* pNext = pRoot;
    tEntry entry;

    entry.num = num;
    strncpy(entry.pStr,pStr,MAXSTR);
    currentIndex = 0;

    while(pNext != NULL)
    {
        if (strcmp(pStr,pNext->pStr) < 0)
        {
        	return insert(currentIndex-1,entry);
        }
        getNext(&pNext);
    }
    return add(entry);
}

listObj::tListError listObj::getByIndex(int pos,pEntry* ppEntry)
{
    currentIndex = 0;
    *ppEntry = pRoot;
    if (pRoot == NULL)
        return LIST_EMPTY;

    if (checkValidIndex(pos) != LIST_OK)
        return LIST_INVALID_INDEX;

    currentIndex = 1;
    while (currentIndex != pos)
    {
        currentIndex++;
        if((*ppEntry)->pNext == NULL)
        {
            return LIST_OK;
        }
        *ppEntry = (*ppEntry)->pNext;
    }
    return LIST_OK;

}

listObj::tListError listObj::getByNum(int num,pEntry* ppEntry)
{
    currentIndex = 0;
    *ppEntry = pRoot;
    if (pRoot == NULL)
        return LIST_EMPTY;

    currentIndex = 1;

    while (*ppEntry != NULL)
    {
        if ((*ppEntry)->num == num)
        {
            return LIST_OK;
        }
        getNext(ppEntry);
    }
    return LIST_NOT_FOUND;

}

listObj::tListError listObj::getNext(pEntry* ppEntry)
{
    if (currentIndex < size)
    {
        currentIndex++;
        return getByIndex(currentIndex,ppEntry);
    }
    else
    {
        *ppEntry = NULL;
        return LIST_INVALID_INDEX;
    }
}

listObj::tListError listObj::getFirst(pEntry* ppEntry)
{
    *ppEntry = pRoot;
    if (pRoot == NULL)
    {
        currentIndex = 0;
        return LIST_EMPTY;
    }
    else
    {
        currentIndex = 1;
        return LIST_OK;
    }
}

listObj::tListError listObj::getLast(pEntry* ppEntry)
{
    *ppEntry = pRoot;
    if (pRoot ==NULL)
    {
        currentIndex = 0;
        return LIST_EMPTY;
    }
    return getByIndex(size,ppEntry);
}


unsigned int listObj::getIndex(void)
{
    return currentIndex;
}

unsigned int listObj::getSize(void)
{
    return size;
}

listObj::tListError listObj::swap(int posA,int posB)
{
    pEntry* pA;
    pEntry* pB;
    pEntry* temp;

    if (checkValidIndex(posA) != LIST_OK)
        return LIST_INVALID_INDEX;
    if (checkValidIndex(posB) != LIST_OK)
        return LIST_INVALID_INDEX;

    if (LIST_OK != getByIndex(posA,pA))
        return LIST_INVALID_INDEX;
    if (LIST_OK != getByIndex(posB,pB))
        return LIST_INVALID_INDEX;
        
    temp = pA;
    pA = pB;
    pB = temp;

    return LIST_OK;
}