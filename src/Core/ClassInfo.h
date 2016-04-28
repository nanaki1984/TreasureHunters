#pragma once

#include <cstdio>
#include "Core/Debug.h"
#include "Core/StringHash.h"
#include "Core/Collections/List_type.h"
#include "Core/Collections/Hash_type.h"

template <typename T> class SmartPtr;

namespace Core {

class RefCounted;
class ClassInfoUtils;

    namespace Memory {

class Allocator;

    } // namespace Memory

class ClassInfo {
public:
    static const uint32_t kInvalidFCC = 0xffffffff;

    typedef void* (*Ctor)(void *pointer);
    typedef void (*StaticCtor)();
private:
    const char      *name;
    uint32_t        hash;
    uint32_t        fcc;
    const ClassInfo *parent;
    const ClassInfo *templ;
    size_t          size;
    size_t          align;
    Ctor            ctor;
    StaticCtor      staticCtor;

    Collections::ListNode<ClassInfo> node;
public:
    ClassInfo(const StringHash &_name, const ClassInfo *_parent, const ClassInfo *_templ, size_t _size, size_t _align, Ctor _ctor);
    ClassInfo(const StringHash &_name, uint32_t _fcc, const ClassInfo *_parent, const ClassInfo *_templ, size_t _size, size_t _align, Ctor _ctor);
    ~ClassInfo();

    int SetStaticCtor(StaticCtor _staticCtor);

    const char* GetName() const;
    uint32_t GetHashCode() const;
    bool HasFCC() const;
    uint32_t GetFCC() const;
    const ClassInfo* GetParent() const;
    const ClassInfo* GetTemplate() const;

    size_t GetSize() const;
    size_t GetAlign() const;

    bool IsDerivedFrom(const ClassInfo *classInfo) const;

    SmartPtr<RefCounted> Create(Memory::Allocator *allocator) const;

    friend class ClassInfoUtils;
};

class ClassInfoUtils
{
private:
    static ClassInfoUtils *instance;

    Collections::List<ClassInfo, &ClassInfo::node> classes;
    Collections::Hash<ClassInfo*> classesHash;
    Collections::Hash<ClassInfo*> fccHash;
public:
    ClassInfoUtils();
    ~ClassInfoUtils();

    void Initialize();

    const ClassInfo* FindClass(const StringHash &className) const;
    const ClassInfo* FindClassFCC(uint32_t fcc) const;

    static ClassInfoUtils* Instance();
    static void Destroy();

    friend class ClassInfo;
};

} // namespace Core

#define DeclareRootClassInfo \
public: \
    static Core::ClassInfo RTTI; \
 \
    virtual const Core::ClassInfo* GetRTTI() const; \
 \
    inline const char* GetTypeName() const \
    { \
        return this->GetRTTI()->GetName(); \
    } \
 \
    template <typename T> bool IsA() const \
    { \
        return this->GetRTTI()->IsDerivedFrom(&T::RTTI); \
    } \
\
    template <typename T> bool IsInstanceOf() const \
    { \
        return this->GetRTTI() == &T::RTTI; \
    }

#define DeclareClassInfo \
public: \
    static Core::ClassInfo RTTI; \
 \
    virtual const Core::ClassInfo* GetRTTI() const;

#define DefineRootAbstractClassInfo(type) \
Core::ClassInfo type::RTTI(#type, nullptr, nullptr, 0, 0, nullptr); \
 \
const Core::ClassInfo* type::GetRTTI() const \
{ \
	return &this->RTTI; \
}

#define DefineRootClassInfo(type) \
Core::ClassInfo type::RTTI(#type, nullptr, nullptr, sizeof(type), __alignof(type), nullptr); \
 \
const Core::ClassInfo* type::GetRTTI() const \
{ \
    return &this->RTTI; \
}

#define DefineAbstractClassInfo(type,ancestor) \
Core::ClassInfo type::RTTI(#type, &ancestor::RTTI, nullptr, 0, 0, nullptr); \
 \
const Core::ClassInfo* type::GetRTTI() const \
{ \
	return &this->RTTI; \
}

#define DefineClassInfo(type,ancestor) \
Core::ClassInfo type::RTTI(#type, &ancestor::RTTI, nullptr, sizeof(type), __alignof(type), nullptr); \
 \
const Core::ClassInfo* type::GetRTTI() const \
{ \
    return &this->RTTI; \
}

#define DefineClassInfoWithFactory(type,ancestor) \
Core::ClassInfo type::RTTI(#type, &ancestor::RTTI, nullptr, sizeof(type), __alignof(type), [](void *pointer){return static_cast<void*>(new(pointer) type());}); \
 \
const Core::ClassInfo* type::GetRTTI() const \
{ \
    return &this->RTTI; \
}

#define DefineClassInfoWithFactoryAndFCC(type,fcc,ancestor) \
Core::ClassInfo type::RTTI(#type, fcc, &ancestor::RTTI, nullptr, sizeof(type), __alignof(type), [](void *pointer){return static_cast<void*>(new(pointer)type()); }); \
\
const Core::ClassInfo* type::GetRTTI() const \
{ \
    return &this->RTTI; \
}

#define DefineTemplateClassInfo(type,ancestor) \
template <typename T> \
Core::ClassInfo type<T>::RTTI(#type, &ancestor::RTTI, &T::RTTI, sizeof(type<T>), __alignof(type<T>), nullptr); \
 \
template <typename T> \
const Core::ClassInfo* type<T>::GetRTTI() const \
{ \
    return &this->RTTI; \
}

#define DefineTemplateClassInfoWithFactory(type,ancestor) \
template <typename T> \
Core::ClassInfo type<T>::RTTI(#type, &ancestor::RTTI, &T::RTTI, sizeof(type<T>), __alignof(type<T>), [](void *pointer){return new(pointer) type<T>();}); \
 \
template <typename T> \
const Core::ClassInfo* type<T>::GetRTTI() const \
{ \
    return &this->RTTI; \
}
