// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_XML_H
#define GZ4D_XML_H

#error Use boost property tree instead. (see following examples)

// vvv examples vvv

#include <boost/property_tree/ptree.hpp>

// Doc, Node, Attribute, etc are now boost::property_tree::ptree
// Attributes are under key named <xmlattr>

boost::property_tree::ptree my_xml_root;

boost::property_tree::ptree::const_iterator c = my_xml_root.begin();



// Loading from xml
#include <boost/property_tree/xml_parser.hpp>

read_xml("some_xml_file.xml",my_xml_root);




// ^^^ examples ^^^


#include <gz4d/gz4d.h>
#include <gz4d/Exception.h>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <gz4d/util/Utilities.h>
#include <memory>

namespace gz4d
{
    namespace xml
    {
        class GZ4DEXPORT Exception: public gz4d::Exception
        {
            public:
                Exception(std::string msg):gz4d::Exception(msg){}
        };

        class Doc;

        template<typename T>
                class Base
        {
            friend class Base<rapidxml::xml_node<char> >;
            friend class Base<rapidxml::xml_attribute<char> >;
            friend class Doc;

            T *node;
            Base(T *n):node(n){}
            T* GetNextPtr(std::string const &name) const;
            T* GetPreviousPtr(std::string const &name) const;
            public:
                Base():node(NULL){}
                operator bool() const {return node;}

                operator std::string() const;
                std::string PrettyPrint() const;

                std::string Name() const;
                void Name(std::string const &n);

                std::string Value() const;

                /// Returns value string.
                void Value(std::string const &v);

                /// Returns the value converted using lexical_cast.
                template<typename RetT>
                        RetT Value() const;

                /// Sets the value of argument using >> operators.
                /// This is useful for types that don't work with lexical_cast, such as gz4d::Vector
                template<typename RetT>
                        void GetValue(RetT &v) const;

                /// Sets the value of element of array argument using >> operators.
                template<typename CT> void GetValue(CT &v, size_t count) const;

                bool IsTrue() const;

                Base GetNext(std::string const &name = std::string()) const;
                Base GetPrevious(std::string const &name = std::string()) const;

                Base &Next(std::string const &name = std::string());
                Base &Previous(std::string const &name = std::string());

                /// prefix increment, goes to the next node.
                Base &operator++();
                /// postfix inc, returns this, then goes to the next node.
                Base operator++(int);

                /// prefix decrement, goes to the previous node.
                Base &operator--();
                /// postfix dec, returns this, then goes to the prev node.
                Base operator--(int);

                // children/parent navigation
                Base GetFirstNode(std::string const &name = std::string()) const;
                Base GetLastNode(std::string const &name = std::string()) const;

                /// Moves to first child node.
                Base &FirstNode(std::string const &name = std::string());
                /// Moves to last child node.
                Base &LastNode(std::string const &name = std::string());

                /// Moves to parent node.
                Base &ParentNode();
                Base<rapidxml::xml_node<char> > GetParentNode() const;

                Base<rapidxml::xml_attribute<char> > GetFirstAttribute(std::string const &name = std::string()) const;
                Base<rapidxml::xml_attribute<char> > GetLastAttribute(std::string const &name = std::string()) const;

                template<typename NodeT>
                        void Append(NodeT const &n);

                template<typename NodeT>
                        void Insert(NodeT const &w, NodeT const &n);

                template<typename NodeT>
                        void Remove(NodeT const &n);
        };

        template<typename T>
                inline Base<T>::operator std::string() const
        {
            std::stringstream ret;
            rapidxml::print(std::ostream_iterator<char>(ret),*node,rapidxml::print_no_indenting);
            return ret.str();
        }

        template<typename T>
                inline std::string Base<T>::PrettyPrint() const
        {
            return boost::lexical_cast<std::string>(*node);
        }

        template<typename T>
                inline std::string Base<T>::Name() const
        {
            return node->name();
        }

        template<typename T>
                inline void Base<T>::Name(std::string const &n)
        {
            rapidxml::xml_document<char> *doc = node->document();
            if(!doc)
                throw(Exception("No document associated with node."));
            node->name(doc->allocate_string(n.c_str()));
        }

        template<typename T>
                inline std::string Base<T>::Value() const
        {
            return node->value();
        }

        template<typename T> template<typename RetT>
                inline RetT Base<T>::Value() const
        {
            return boost::lexical_cast<RetT>(std::string(node->value()));
        }

        template<typename T>
                inline bool Base<T>::IsTrue() const
        {
            return gz4d::IsTrue(node->value());
        }

        template<typename T> template<typename RetT>
                inline void Base<T>::GetValue(RetT &v) const
        {
            std::stringstream ss(node->value());
            ss >> v;
        }

        template<typename T> template<typename CT> inline void Base<T>::GetValue(CT &v, size_t count) const
        {
            std::stringstream ss(node->value());
            for(size_t i = 0; i < count; ++i)
                ss >> v[i];
        }

        template<typename T>
                inline void Base<T>::Value(std::string const &v)
        {
            rapidxml::xml_document<char> *doc = node->document();
            if(!doc)
                throw(Exception("No document associated with node."));
            node->value(doc->allocate_string(v.c_str()));
        }

        template<>
                inline rapidxml::xml_node<char> *Base<rapidxml::xml_node<char> >::GetNextPtr(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return node->next_sibling(n);
        }

        template<>
                inline rapidxml::xml_node<char> *Base<rapidxml::xml_node<char> >::GetPreviousPtr(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return node->previous_sibling(n);
        }

        template<>
                inline rapidxml::xml_attribute<char> *Base<rapidxml::xml_attribute<char> >::GetNextPtr(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return node->next_attribute(n);
        }

        template<>
                inline rapidxml::xml_attribute<char> *Base<rapidxml::xml_attribute<char> >::GetPreviousPtr(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return node->previous_attribute(n);
        }

        template<typename T>
                inline Base<T> Base<T>::GetNext(std::string const &name) const
        {
            return Base<T>(GetNextPtr(name));
        }

        template<typename T>
                inline Base<T> Base<T>::GetPrevious(std::string const &name) const
        {
            return Base<T>(GetPreviousPtr(name));
        }

        template<typename T>
                inline Base<T> &Base<T>::Next(std::string const &name)
        {
            node = GetNextPtr(name);
            return *this;
        }

        template<typename T>
                inline Base<T> &Base<T>::Previous(std::string const &name)
        {
            node = GetPreviousPtr(name);
            return *this;
        }

        template<typename T>
                inline Base<T> &Base<T>::operator++()
        {
            return Next();
        }

        template<typename T>
                inline Base<T> Base<T>::operator++(int)
        {
            Base<T> ret = GetNext();
            Next();
            return ret;
        }

        template<typename T>
                inline Base<T> &Base<T>::operator--()
        {
            return Previous();
        }

        template<typename T>
                inline Base<T> Base<T>::operator--(int)
        {
            Base<T> ret = GetPrevious();
            Previous();
            return ret;
        }

        template<>
                inline Base<rapidxml::xml_node<char> > Base<rapidxml::xml_node<char> >::GetFirstNode(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return Base<rapidxml::xml_node<char> >(node->first_node(n));
        }

        template<>
                inline Base<rapidxml::xml_node<char> > Base<rapidxml::xml_node<char> >::GetLastNode(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return Base(node->last_node(n));
        }

        template<>
                inline Base<rapidxml::xml_node<char> > &Base<rapidxml::xml_node<char> >::FirstNode(std::string const &name)
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            node = node->first_node(n);
            return *this;
        }

        template<>
                inline Base<rapidxml::xml_node<char> > &Base<rapidxml::xml_node<char> >::LastNode(std::string const &name)
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            node = node->last_node(n);
            return *this;
        }

        template<>
                inline Base<rapidxml::xml_node<char> > &Base<rapidxml::xml_node<char> >::ParentNode()
        {
            node = node->parent();
            return *this;
        }

        template<typename T>
                inline Base<rapidxml::xml_node<char> > Base<T>::GetParentNode() const
        {
            return Base<rapidxml::xml_node<char> >(node->parent());
        }

        template<>
                inline Base<rapidxml::xml_attribute<char> > Base<rapidxml::xml_node<char> >::GetFirstAttribute(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return Base<rapidxml::xml_attribute<char> >(node->first_attribute(n));
        }

        template<>
                inline Base<rapidxml::xml_attribute<char> > Base<rapidxml::xml_node<char> >::GetLastAttribute(std::string const &name) const
        {
            char const *n;
            if(name.empty())
                n = NULL;
            else
                n = name.c_str();
            return Base<rapidxml::xml_attribute<char> >(node->last_attribute(n));
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Append(Base<rapidxml::xml_node<char> > const &n)
        {
            node->append_node(n.node);
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Append(Base<rapidxml::xml_attribute<char> > const &n)
        {
            node->append_attribute(n.node);
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Insert(Base<rapidxml::xml_node<char> > const &where, Base<rapidxml::xml_node<char> > const &n)
        {
            node->insert_node(where.node,n.node);
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Insert(Base<rapidxml::xml_attribute<char> > const &where, Base<rapidxml::xml_attribute<char> > const &n)
        {
            node->insert_attribute(where.node,n.node);
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Remove(Base<rapidxml::xml_node<char> > const &n)
        {
            node->remove_node(n.node);
        }

        template<> template<>
                inline void Base<rapidxml::xml_node<char> >::Remove(Base<rapidxml::xml_attribute<char> > const &n)
        {
            node->remove_attribute(n.node);
        }

        typedef Base<rapidxml::xml_node<char> > Node;
        typedef Base<rapidxml::xml_attribute<char> > Attribute;


        class Doc: public Node
        {
            std::shared_ptr<rapidxml::xml_document<> > doc;
            public:
                Doc():doc(new rapidxml::xml_document<char>){node = doc.get();}

                Doc(Doc const &d):doc(d.doc),Node(d){}

                void Parse(std::string const &doc_str)
                {
                    doc->parse<rapidxml::parse_validate_closing_tags>(doc->allocate_string(doc_str.c_str()));
                }
                
                void ParseFile(std::string const &fname)
                {
                    rapidxml::file<char> f(fname.c_str());
                    Parse(f.data());
                }

                Node AllocateNode(std::string const &name, std::string const &value = std::string())
                {
                    char *v;
                    if(value.empty())
                        v = NULL;
                    else
                        v = doc->allocate_string(value.c_str());
                    return Node(doc->allocate_node(rapidxml::node_element,doc->allocate_string(name.c_str()),v));
                }

                Attribute AllocateAttribute(std::string const &name, std::string const &value = std::string())
                {
                    char *v;
                    if(value.empty())
                        v = NULL;
                    else
                        v = doc->allocate_string(value.c_str());
                    return Attribute(doc->allocate_attribute(doc->allocate_string(name.c_str()),v));
                }

        };


    }
}

#endif
