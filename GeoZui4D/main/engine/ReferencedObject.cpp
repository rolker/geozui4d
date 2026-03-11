// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "ReferencedObject.h"

ReferencedObject::~ReferencedObject()
{
}

void ReferencedObject::setSelf(std::shared_ptr<ReferencedObject> s)
{
    assert(s.get() == this);
    self = s;
}

std::shared_ptr<ReferencedObject> ReferencedObject::getBaseSelf() const
{
    std::shared_ptr<ReferencedObject> ret(self);
    assert(ret);
    return ret;
}
