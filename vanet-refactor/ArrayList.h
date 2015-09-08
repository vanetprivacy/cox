#pragma once
#include <iostream>   // for endl, fixed
using namespace std;

template<class C> class ArrayList
{
    long int _Count;
    long int AllocatedSize;
    C* Storage;
public:
    ArrayList()
    {
        Storage=NULL;
        Clear();
    }
    ArrayList(int PreAllocateSize)
    {
        Storage=NULL;
        Clear();
        EnsureMinimumSize(PreAllocateSize);
    }
    ~ArrayList(){Clear();}
    ArrayList& operator=(ArrayList& Copy)
    {
        Storage=NULL;
        Clear();
        int NewCount=Copy.Count();
        EnsureMinimumSize(NewCount);
        for(int i=0;i<NewCount;i++){Storage[i]=Copy[i];}
        return *this;
    }
    ArrayList(ArrayList& Copy)
    {
        this->operator=(Copy);
    }
    void EnsureMinimumSize(int NewCount)
    {
        if(NewCount>AllocatedSize)
        {
            C* NewStorage=new C[NewCount];
            for(int i=0;i<AllocatedSize;i++){NewStorage[i]=Storage[i];}
            if(Storage!=NULL){delete[] Storage;}
            Storage=NewStorage;
            AllocatedSize=NewCount;
        }
    }
    long int Count(){return _Count;}
    int PushObj(C Obj)
    {
        EnsureMinimumSize(_Count+1);
        Storage[_Count++]=Obj;
        return _Count-1;
    }
    C PopObj()
    {
        C RetVal=(_Count==0)?(C)NULL:Storage[--_Count];
        Storage[_Count]=(C)NULL;
        return RetVal;
    }
    int UnshiftObj(const C Obj)
    {
        EnsureMinimumSize(_Count+1);
        for(int i=_Count;i>1;i--){Storage[i]=Storage[i-1];}
        Storage[0]=Obj;
        return _Count++;
    }
    C ShiftObj()
    {
        C RetVal=(_Count==0)?(C)NULL:Storage[0];
        for(int i=0;i<_Count-1;i++){Storage[i]=Storage[i+1];}
        Storage[--_Count]=(C)NULL;
        return RetVal;
    }
    void Clear()
    {
        if(Storage!=NULL)
        {
            delete[] Storage;
            Storage=NULL;
        }
        _Count=0;
        AllocatedSize=0;
    }
    C Remove(int RemoveIndex)
    {
        C RetVal=Storage[RemoveIndex];
        for(int i=RemoveIndex;i<_Count-1;i++){Storage[i]=Storage[i+1];}
        Storage[--_Count]=(C)NULL;
        return RetVal;
    }
    bool Remove(C RemoveObject)
    {
        bool Found=false;
        for(int i=0;i<_Count;i++)
        {
            if(Storage[i]==RemoveObject)
            {
                Remove(i--);
                Found=true;
            }
        }
        return Found;
    }
    int IndexOf(C FindObject)
    {
        for(int i=0;i<_Count;i++)
        {
            if(Storage[i]==FindObject){return i;}
        }
        return -1;
    }
    bool Contains(C FindObject){return IndexOf(FindObject)!=-1;}
    inline C& operator[](int Index){return Storage[Index];}
};