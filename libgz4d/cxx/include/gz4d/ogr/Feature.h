// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_OGR_FEATURE_H
#define GZ4D_OGR_FEATURE_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <vector>
#include <string>
#include<ogr_feature.h>
#include <gz4d/ogr/Geometry.h>

namespace gz4d
{

    namespace ogr
    {
        class Layer;

        class Feature
        {
            friend class Layer;
            std::shared_ptr<OGRFeature> feature;
            Feature(OGRFeature *f);


            public:
                class Definition;
                class Field
                {
                    friend class Feature;
                    std::shared_ptr<OGRFeature> feature;
                    int index;
                    Field(std::shared_ptr<OGRFeature> f, int i);
                    public:
                        class Definition
                        {
                            friend class Feature::Definition;
                            friend class Field;
                            std::string name;
                            std::string type;
                            Definition(OGRFieldDefn *d);
                            public:
                                std::string GetName();
                                std::string GetType();
                        };

                        Definition GetDefinition();

                        /// switch to same field from different feature
                        Field &operator()(Feature &f);


                        template<typename T> T GetValue();

                        operator bool();
                };


                class Definition
                {
                    friend class Layer;
                    friend class Feature;
                    std::shared_ptr<OGRFeatureDefn> definition;
                    Definition(OGRFeatureDefn *d);
                    public:
                        Definition(Definition const &other);
                        std::size_t size();
                        Field::Definition operator[](int i);
                        Field::Definition operator[](std::string const &name);
                        std::string GetName();
                };


                Feature();
                Feature(Feature const &other);

                bool operator==(Feature const &other);
                std::size_t size();
                Definition GetDefinition();
                Field operator[](int i) const;
                Field operator[](std::string const &name) const;
                Field operator[](char const *name) const {return operator[](std::string(name));}
                operator bool() const {return bool(feature);}
                Geometry GetGeometry();
        };

        template<> std::vector<int> Feature::Field::GetValue();
        template<> std::vector<double> Feature::Field::GetValue();
        template<> std::vector<std::string> Feature::Field::GetValue();

    }
}

inline gz4d::ogr::Feature::Feature(OGRFeature *f)
:feature(std::shared_ptr<OGRFeature>(f,OGRFeature::DestroyFeature))
{
}

inline gz4d::ogr::Feature::Feature()
{
}

inline gz4d::ogr::Feature::Feature(Feature const &other)
:feature(other.feature)
{
}

inline bool gz4d::ogr::Feature::operator==(Feature const &other)
{
    return feature->Equal(other.feature.get());
}

inline std::size_t gz4d::ogr::Feature::size()
{
    return feature->GetFieldCount();
}

inline gz4d::ogr::Feature::Definition gz4d::ogr::Feature::GetDefinition()
{
    return Definition(feature->GetDefnRef());
}

inline gz4d::ogr::Feature::Field gz4d::ogr::Feature::operator[](int i) const
{
    return Field(feature,i);
}

inline gz4d::ogr::Feature::Field gz4d::ogr::Feature::operator[](std::string const &name) const 
{
    return Field(feature,feature->GetFieldIndex(name.c_str()));
}

inline gz4d::ogr::Geometry gz4d::ogr::Feature::GetGeometry()
{
    return Geometry(feature->GetGeometryRef(),feature);
}

inline gz4d::ogr::Feature::Definition::Definition(OGRFeatureDefn *d)
:definition(make_shared_from_ogr(d))
{
}

inline gz4d::ogr::Feature::Definition::Definition(Definition const &other)
:definition(other.definition)
{
}

inline std::size_t gz4d::ogr::Feature::Definition::size()
{
    return definition->GetFieldCount();
}

inline gz4d::ogr::Feature::Field::Definition gz4d::ogr::Feature::Definition::operator[](int i)
{
    return Field::Definition(definition->GetFieldDefn(i));
}

inline gz4d::ogr::Feature::Field::Definition gz4d::ogr::Feature::Definition::operator[](std::string const &name)
{
    return operator[](definition->GetFieldIndex(name.c_str()));
}

inline std::string gz4d::ogr::Feature::Definition::GetName()
{
    return definition->GetName();
}

inline gz4d::ogr::Feature::Field::Field(std::shared_ptr<OGRFeature> f, int i)
:feature(f),index(i)
{
}

inline gz4d::ogr::Feature::Field::Definition gz4d::ogr::Feature::Field::GetDefinition()
{
    return Definition(feature->GetFieldDefnRef(index));
}


inline gz4d::ogr::Feature::Field &gz4d::ogr::Feature::Field::operator()(gz4d::ogr::Feature &f)
{
    feature = f.feature;
    return *this;
}

namespace gz4d
{
    namespace ogr
    {
        template<> inline int Feature::Field::GetValue()
        {
            assert(*this);
            return feature->GetFieldAsInteger(index);
        }
        
        template<> inline double Feature::Field::GetValue()
        {
            assert(*this);
            return feature->GetFieldAsDouble(index);
        }
        
        template<> inline std::string Feature::Field::GetValue()
        {
            assert(*this);
            if(feature->GetFieldDefnRef(index))
            {
                switch(feature->GetFieldDefnRef(index)->GetType())
                {
                    case OFTInteger:
                        return boost::lexical_cast<std::string>(feature->GetFieldAsInteger(index));
                    case OFTReal:
                        return boost::lexical_cast<std::string>(feature->GetFieldAsDouble(index));
                    case OFTString:
                        return feature->GetFieldAsString(index);
                    case OFTIntegerList:
                    {
                        std::vector<int> data = GetValue<std::vector<int> >();
                        std::stringstream ret;
                        for(std::vector<int>::iterator i = data.begin(); i != data.end(); ++i)
                        {
                            if(i != data.begin())
                                ret << ", ";
                            ret << *i;
                        }
                        return ret.str();
                    }
                    case OFTRealList:
                    {
                        std::vector<double> data = GetValue<std::vector<double> >();
                        std::stringstream ret;
                        for(std::vector<double>::iterator i = data.begin(); i != data.end(); ++i)
                        {
                            if(i != data.begin())
                                ret << ", ";
                            ret << *i;
                        }
                        return ret.str();
                    }
                    case OFTStringList:
                    {
                        std::vector<std::string> data = GetValue<std::vector<std::string> >();
                        std::stringstream ret;
                        for(std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
                        {
                            if(i != data.begin())
                                ret << ", ";
                            ret << *i;
                        }
                        return ret.str();
                    }
                    default:
                        return "Unknown type";
                        
                }
            }
            return "";
        }
        
        template<> inline std::vector<int> Feature::Field::GetValue()
        {
            assert(*this);
            int count = 0;
            int const *data = feature->GetFieldAsIntegerList(index,&count);
            return std::vector<int>(data,&data[count]);
        }
        
        template<> inline std::vector<double> Feature::Field::GetValue()
        {
            assert(*this);
            int count;
            double const *data = feature->GetFieldAsDoubleList(index,&count);
            return std::vector<double>(data,&data[count]);
        }
        
        template<> inline std::vector<std::string> Feature::Field::GetValue()
        {
            assert(*this);
            char **data = feature->GetFieldAsStringList(index);
            return std::vector<std::string>(data,&data[CSLCount(data)]);
        }
    }
}

inline gz4d::ogr::Feature::Field::operator bool()
{
    if(feature)
        return feature->IsFieldSet(index);
    return false;
}

inline gz4d::ogr::Feature::Field::Definition::Definition(OGRFieldDefn *d)
:name(d->GetNameRef())
{
    switch(d->GetType())
    {
        case OFTInteger:
            type = "Integer";
            break;
        case OFTIntegerList:
            type = "Integer List";
            break;
        case OFTReal:
            type = "Double Precision floating point";
            break;
        case OFTRealList:
            type = "List of doubles";
            break;
        case OFTString:
            type = "String";
            break;
        case OFTStringList:
            type = "String List";
            break;
        case OFTBinary:
            type = "Raw Binary";
            break;
        case OFTDate:
            type = "Date";
            break;
        case OFTTime:
            type = "Time";
            break;
        case OFTDateTime:
            type = "Date and Time";
            break;
        default:
            type = "Unknown";
    }
}

inline std::string gz4d::ogr::Feature::Field::Definition::GetName()
{
    return name;
}

inline std::string gz4d::ogr::Feature::Field::Definition::GetType()
{
    return type;
}



#endif
