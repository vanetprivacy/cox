#pragma once
#include <iostream>   // for endl, fixed
#include "ArrayList.h"

template<class C> class Dictionary
{
    ArrayList<std::string> Keys;
    ArrayList<C> Values;
public:
    Dictionary(){}
    void Set(std::string Key,C Value)
    {
        int Index=Keys.IndexOf(Key);
        if(Index==-1)
        {
            Keys.PushObj(Key);
            Values.PushObj(Value);
        }
        else
        {
            Values[Index]=Value;
        }
    }
    C Get(std::string Key)
    {
        int Index=Keys.IndexOf(Key);
        if(Index==-1){return (C)NULL;}
        return Get(Index);
    }
    C Get(int i){return Values[i];}
    int Count(){return Keys.Count();}
    std::string GetKey(int i){return Keys[i];}
    void Clear()
    {
        Keys.Clear();
        Values.Clear();
    }
    bool HasKey(std::string Key){return Keys.IndexOf(Key)!=-1;}
};