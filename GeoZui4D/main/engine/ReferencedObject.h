// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef REFERENCED_OBJECT_H
#define REFERENCED_OBJECT_H

#include <memory>


class ReferencedObject
{
    protected:
        ReferencedObject(){}
        virtual ~ReferencedObject();

        void setSelf(std::shared_ptr<ReferencedObject> s);
        std::shared_ptr<ReferencedObject> getBaseSelf() const;
    private:
        std::weak_ptr<ReferencedObject> self;
};

template <typename T, typename BT> class RefD: virtual public BT
{
    public:
        typedef std::shared_ptr<T> Ptr;
        typedef std::weak_ptr<T> WeakPtr;

        inline static std::shared_ptr<T> instantiate(T * t)
        {
            std::shared_ptr<T> ret(t);
            ret->setSelf(ret);
            return ret;
        }
        inline static std::shared_ptr<T> instantiate() { std::shared_ptr<T> ret = instantiate(new T); return ret;}

        inline static std::shared_ptr<ReferencedObject> instantiateAsBase(){return T::instantiate();}
    protected:
        inline std::shared_ptr<T> getSelf() const
        {
            return std::dynamic_pointer_cast<T>(BT::getBaseSelf());
        }
};

#endif
