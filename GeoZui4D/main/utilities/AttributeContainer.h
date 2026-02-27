/* Copyright (c) 2010 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef ATTRIBUTECONTAINER_H
#define ATTRIBUTECONTAINER_H

#include "TellableInterface.h"
#include <map>
#include <memory>
#include <boost/lexical_cast.hpp>

struct AttributeDescription
{
    std::string label;
    std::string description;
    AttributeDescription();
    AttributeDescription(std::string const &l, std::string const&d);
};

class AttributeContainer: public TellableInterface
{
    class AttributeBase
    {
        public:
            typedef std::shared_ptr<AttributeBase> Ptr;
            ~AttributeBase();
            virtual void Set(std::string const &sv)=0;
            virtual std::string Get() const=0;
    };

    template <typename T> class AttributeValue: public AttributeBase
    {
        T* value;
        public:
            AttributeValue(T &v):value(&v){}
            void Set(std::string const &v){*value = boost::lexical_cast<T>(v);}
            std::string Get() const{return boost::lexical_cast<std::string>(*value);}
    };

    typedef std::map<std::string,AttributeDescription> DescriptionMap;

    static std::map<std::string,DescriptionMap> attributeDescriptions;
    static std::map<std::string,std::vector<std::string> > attributeLists;

    void DefineAttribute(std::string const &id, AttributeDescription const &d, AttributeBase::Ptr v);
    std::map<std::string,AttributeBase::Ptr> attributes;
    protected:
        template <typename T> void DefineAttribute(std::string const &id, std::string const &label, std::string const &description, T &value)
        {
            std::shared_ptr<AttributeValue<T> > v(new AttributeValue<T>(value));
            DefineAttribute(id,AttributeDescription(label,description),v);
        }
        AttributeDescription const &GetAttributeDescription(std::string const &id) const;
        std::vector<std::string> const &GetAttributeList() const;
        void SetAttributeValue(std::string const &id, std::string const&value);
        std::string GetAttributeValue(std::string const &id) const;
        bool HasAttribute(std::string const &id) const;
};

#endif
