/* Copyright (c) 2010 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
