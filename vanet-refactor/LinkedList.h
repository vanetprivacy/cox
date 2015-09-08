#pragma once

template<class C> class LinkedListComponent
{
public:
    C Obj;
    LinkedListComponent* Previous;
    LinkedListComponent* Next;
    LinkedListComponent(C Obj)
    {
        this->Obj=Obj;
        Previous=NULL;
        Next=NULL;
    }
};
template<class C> struct LinkedListIterator;
template<class C> class LinkedList
{
    friend struct LinkedListIterator<C>;
    LinkedListComponent<C>* FirstLink;
    LinkedListComponent<C>* LastLink;
    long int _Count;
public:
    LinkedList()
    {
        FirstLink=NULL;
        LastLink=NULL;
        _Count=0;
    }
    ~LinkedList(){Clear();}
    long int Count(){return _Count;}
    void PushObj(C Obj)
    {
        LinkedListComponent<C>* NewLink=new LinkedListComponent<C>(Obj);
        if(_Count==0)
        {
            FirstLink=NewLink;
            LastLink=NewLink;
        }
        LastLink->Next=NewLink;
        NewLink->Previous=LastLink;
        FirstLink->Previous=NewLink;
        NewLink->Next=FirstLink;
        LastLink=NewLink;
        _Count++;
    }
    C PopObj()
    {
        C RetVal=NULL;
        if(_Count!=0)
        {
            LinkedListComponent<C>* NextToLastLink=LastLink->Previous;
            NextToLastLink->Next=FirstLink;
            FirstLink->Previous=NextToLastLink;
            RetVal=LastLink->Obj;
            delete LastLink;
            LastLink=NextToLastLink;
            _Count--;
            if(_Count==0)
            {
                FirstLink=NULL;
                LastLink=NULL;
            }
        }
        return RetVal;
    }
    void UnshiftObj(const C Obj)
    {
        LinkedListComponent<C>* NewLink=new LinkedListComponent<C>(Obj);
        if(_Count==0)
        {
            FirstLink=NewLink;
            LastLink=NewLink;
        }
        LastLink->Next=NewLink;
        NewLink->Previous=LastLink;
        FirstLink->Previous=NewLink;
        NewLink->Next=FirstLink;
        FirstLink=NewLink;
        _Count++;
    }
    C ShiftObj()
    {
        C RetVal=NULL;
        if(_Count!=0)
        {
            LinkedListComponent<C>* SecondLink=FirstLink->Next;
            LastLink->Next=SecondLink;
            SecondLink->Previous=LastLink;
            RetVal=FirstLink->Obj;
            delete FirstLink;
            FirstLink=SecondLink;
            _Count--;
            if(_Count==0)
            {
                FirstLink=NULL;
                LastLink=NULL;
            }
        }
        return RetVal;
    }
    C GetFirstObj(){return (FirstLink==NULL)?NULL:FirstLink->Obj;}
    C GetLastObj(){return (FirstLink==NULL)?NULL:LastLink->Obj;}
    LinkedListIterator<C> GetIterator(){return LinkedListIterator<C>(this);}
    void Clear()
    {
        while(_Count>0)
        {
            LinkedListComponent<C>* Popped=LastLink;
            LastLink=Popped->Previous;
            delete Popped;
            _Count--;
        }
        FirstLink=NULL;
        LastLink=NULL;
    }
};
template<class C> struct LinkedListIterator
{
    //WARNING: WHILE THERE EXISTS ANY LINKEDLISTITERATOR, IT IS ILLEGAL TO MODIFY THE ASSCOCIATED LINKEDLIST EXCEPT THROUGH THE FUNCTIONS EXPOSED BY THE ITERATOR.
    // YOU MAY NOT MODIFY THE LINKEDLIST OR USE THE FUNCTIONS IN THE ITERATOR WHILE MORE THAN ONE ITERATOR EXISTS.
    LinkedList<C>* Host;
    LinkedListComponent<C>* Current;
public:
    LinkedListIterator(LinkedList<C>* Host)
    {
        this->Host=Host;
        Current=NULL;
    }
    bool hasnext(){return (Host==NULL)?false:Current!=Host->LastLink;}
    C next()
    {
        C RetVal=NULL;
        if(Host!=NULL&&hasnext())
        {
            if(Current==NULL){Current=Host->FirstLink;}
            else{Current=Current->Next;}
            RetVal=(C)(Current->Obj);
        }
        return RetVal;
    }
    C remove()
    {
        C RetVal=NULL;
        if(Host==NULL){return NULL;}
        if(Current!=NULL)
        {
            Current->Previous->Next=Current->Next;
            Current->Next->Previous=Current->Previous;
            LinkedListComponent<C>* Previous=NULL;
            if(Current!=Host->FirstLink){Previous=Current->Previous;}
            if(Host->FirstLink==Current){Host->FirstLink=Current->Next;}
            if(Host->LastLink==Current){Host->LastLink=Current->Previous;}
            RetVal=Current->Obj;
            delete Current;
            Current=Previous;
            Host->_Count--;
            if(Host->_Count==0)
            {
                Host->FirstLink=NULL;
                Host->LastLink=NULL;
            }
        }
        return RetVal;
    }
    void insert(C Obj)
    {
        if(Host==NULL){return;}
        if(Current==NULL){Host->UnshiftObj(Obj);}
        else
        {
            LinkedListComponent<C>* NewLink=new LinkedListComponent<C>(Obj);
            NewLink->Next=Current->Next;
            NewLink->Previous=Current;
            NewLink->Previous->Next=NewLink;
            NewLink->Next->Previous=NewLink;
            if(Host->LastLink==Current){Host->LastLink=NewLink;}
            Current=NewLink;
            Host->_Count++;
        }
    }
    void reset(){Current=NULL;}
};