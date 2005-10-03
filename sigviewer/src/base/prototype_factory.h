// prototpye_factory.h

#ifndef PROTOTYPE_FACTORY_H
#define PROTOTYPE_FACTORY_H

#include "user_types.h"

#include <QMap>
#include <QSet>
#include <QString>

namespace BioSig_
{

// generic prototype factory
template<typename ElementType>
class PrototypeFactory
{
public:
    typedef QSet<QString>::const_iterator StringIterator;

    enum Registration
    {
        PROTOTYPE_REGISTERED = 0,
        PROTOTYPE_NOTREGISTERED = 1
    };

    int8 addPrototype(const QString& name, ElementType* element);
    ElementType* getElement(const QString& name);

    StringIterator getElementNameBegin();
    StringIterator getElementNameEnd();

protected :
    PrototypeFactory();
    ~PrototypeFactory();

private:
    typedef QMap<QString, ElementType*> String2ElementPtrMap;
    typedef QSet<QString> StringSet;

    // not allowed
    PrototypeFactory(const PrototypeFactory& src);
    const PrototypeFactory& operator=(const PrototypeFactory& src);

    String2ElementPtrMap element_map_;
    StringSet name_set_;
};

// constructor
template<typename ElementType>
PrototypeFactory<ElementType>::PrototypeFactory()
{
    // nothing
}

// destructor
template<typename ElementType>
PrototypeFactory<ElementType>::~PrototypeFactory()
{
    typename String2ElementPtrMap::iterator iter;
    for (iter = element_map_.begin(); iter != element_map_.end(); iter++)
    {
        delete iter.value();
    }
}

// add prototype  
template<typename ElementType>
int8 PrototypeFactory<ElementType>::addPrototype(const QString& name,
                                                 ElementType* element)
{
    if (element && !name.isEmpty() &&
        !name_set_.contains(name))
    {
        name_set_.insert(name);
        element_map_[name] = element;
        return PROTOTYPE_REGISTERED;
    }
    else
    {
        delete element;
        return PROTOTYPE_NOTREGISTERED;
    }
}

// get element
template<typename ElementType>
ElementType* PrototypeFactory<ElementType>::getElement(const QString& name)
{
    typename String2ElementPtrMap::iterator iter = element_map_.find(name);
    return iter == element_map_.end() ? 0 : iter.value()->clone();
}

// get element name begin
template<typename ElementType>
typename PrototypeFactory<ElementType>::StringIterator
    PrototypeFactory<ElementType>::getElementNameBegin()
{
    return name_set_.begin();
}

// get element name end
template<typename ElementType>
typename PrototypeFactory<ElementType>::StringIterator
    PrototypeFactory<ElementType>::getElementNameEnd()
{
    return name_set_.end();
}

} // namespace BioSig_

#endif
