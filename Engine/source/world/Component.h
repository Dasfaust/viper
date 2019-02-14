#pragma once

typedef void* EntityHandle;
struct ECSBase
{
    EntityHandle entity;

    static long next();
};

template<typename T>
struct ECSComponentBase : public ECSBase
{
    static const long id;
};
template<typename T>
const long ECSComponentBase<T>::id = ECSBase::next();

struct Component : public ECSComponentBase<Component>
{

};