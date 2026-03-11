// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "AttributeContainer.h"
#include <typeinfo>

std::map<std::string,AttributeContainer::DescriptionMap> AttributeContainer::attributeDescriptions;
std::map<std::string,std::vector<std::string> > AttributeContainer::attributeLists;

AttributeContainer::AttributeBase::~AttributeBase()
{

}

void AttributeContainer::DefineAttribute(const std::string& id, const AttributeDescription& d, AttributeContainer::AttributeBase::Ptr v)
{
    std::string cname = typeid(*this).name();
    if(attributeDescriptions[cname].find(id) == attributeDescriptions[cname].end())
    {
        attributeDescriptions[cname][id] = d;
        attributeLists[cname].push_back(id);
    }
    attributes[id] = v;
}

AttributeDescription const &AttributeContainer::GetAttributeDescription(const std::string& id) const
{
    return attributeDescriptions[typeid(*this).name()][id];
}

void AttributeContainer::SetAttributeValue(const std::string& id, const std::string& value)
{
    attributes[id]->Set(value);
}

std::string AttributeContainer::GetAttributeValue(const std::string& id) const
{
    return attributes.find(id)->second->Get();
}

std::vector< std::string> const & AttributeContainer::GetAttributeList() const
{
    return attributeLists[typeid(*this).name()];
}

AttributeDescription::AttributeDescription()
{
}

AttributeDescription::AttributeDescription(const std::string& l, const std::string& d)
:label(l),description(d)
{
}

bool AttributeContainer::HasAttribute(const std::string& id) const
{
    return attributes.find(id) != attributes.end();
}
