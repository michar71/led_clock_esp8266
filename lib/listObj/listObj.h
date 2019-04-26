//
//  listObj.hpp
//  
//
//  Created by Michael Hartmann on 11/27/15.
//
//

#ifndef listObj_h
#define listObj_h

#define MAXSTR 64

class listObj{
public:
    typedef struct sEntry{
        int num;
        char pStr[MAXSTR];
        void* pPtr;
        sEntry* pNext;
    }tEntry;

    typedef tEntry* pEntry;
    
    typedef enum{
        LIST_OK = 0,
        LIST_NOT_FOUND,
        LIST_INVALID_INDEX,
        LIST_COULDNT_ADD,
        LIST_EMPTY
    }tListError;
    
    listObj();
    ~listObj();
    tListError clear(void);
    tListError add(tEntry entry);
    tListError insert(int pos, tEntry entry);
    tListError addData(int num, char* pStr);
    tListError insertData(int pos,int num, char* pStr);
    tListError remove(int pos);
    tListError insertByAlpha(int num, char* pStr);
    tListError getByIndex(int pos,pEntry* ppEntry);
    tListError getByNum(int num,pEntry* ppEntry);
    tListError getNext(pEntry* ppEntry);
    tListError getFirst(pEntry* ppEntry);
    tListError getLast(pEntry* ppEntry);
    tListError swap(int posA,int posB);
    unsigned int getIndex(void);
    unsigned int getSize(void);
private:
    tListError checkValidIndex(int pos);
    pEntry pRoot;
    int currentIndex;
    int size;
};


#endif /* listObj_h */
