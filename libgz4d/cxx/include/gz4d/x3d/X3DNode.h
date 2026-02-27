#ifndef GZ4D_X3DNODE_H
#define GZ4D_X3DNODE_H

#include "X3DTypes.h"
#include "X3DAttributeContainer.h"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <gz4d/vector/ios.h>

namespace gz4d
{
    class ViewState;
    class X3DNode;

    class GZ4DEXPORT SFNode: public std::shared_ptr<X3DNode>
    {
        public:
            SFNode(){}
            SFNode(X3DNode *n, boost::property_tree::ptree x3d, X3D* root);
            SFNode(SFNode const &, boost::property_tree::ptree x3d, X3D* root);
            SFNode(X3DNode *n) : std::shared_ptr<X3DNode>(n){}
    };

    class X3DGroupingNode;

    /// Base class for all X3D nodes
    class GZ4DEXPORT X3DNode : public X3DAttributeContainer
    {
    public:

        X3DNode();
        virtual ~X3DNode(){};

        virtual SFNode newNode(std::string nodeType, boost::property_tree::ptree x3d, X3D* root);
        static void Load(const boost::property_tree::ptree& x3d, gz4d::X3D* root, gz4d::SFNode safe_this);
        virtual void Load(const boost::property_tree::ptree& x3d, X3D* root){}

        virtual void Draw();
        virtual void Draw(SFInt32 index){}
        bool preDraw();
        SFString getTypeName() {return typeName;}
        X3DGroupingNode * getGroupingNode();

        static void LoadArray(const boost::property_tree::ptree& x3d, std::string name, gz4d::MFString& array);
        template <typename baseType> static void LoadArrayItem(std::istringstream &datastrm, std::vector<baseType> &array);
        template <typename baseType> static void LoadArray(const boost::property_tree::ptree &x3d, std::string name, std::vector<baseType> &array);
        template <typename baseType> static bool LoadItem(const boost::property_tree::ptree &x3d, std::string name, baseType &item);
    protected:
        SFNode metadata;
        SFString DEF;
        SFString USE;
        SFNode useNode;

        SFString typeName;

    };

    typedef std::vector<SFNode> MFNode;
    typedef SFNode (*SFNodeCreator)(boost::property_tree::ptree x3d, X3D *root) ;

    template <class Type>
            SFNode createX3DNode( boost::property_tree::ptree x3d, X3D *root)
    {
        return SFNode( new Type, x3d, root );
    }

    template <typename T> inline void X3DNode::LoadArrayItem(std::istringstream &datastrm, std::vector<T> &array)
    {
        T item;
        try
        {
            if ((datastrm >> item)) //we need to check in case baseType is not another vector (like just plain int or float)
                array.push_back(item);
        }
        catch (gz4d::ExtractionException e)
        {
            //&&& If we want to report it and/or throw it, do it here!
            //if (e.itemsExtracted() > 0)
            //    throw e;
        }
    }
    
    
    template <typename T> inline void X3DNode::LoadArray(const boost::property_tree::ptree &x3d, std::string name, std::vector<T> &array)
    {
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>."+name);
        if(a)
        {
            std::vector<std::string> parts;
            std::string data = a.get();
            boost::split(parts,data,boost::algorithm::is_any_of(","));
    
            if (parts.size() >= 2)
            {
                for(std::vector<std::string>::iterator i = parts.begin(); i != parts.end(); ++i)
                {
                    std::istringstream datastrm(*i);
                    if(datastrm) //any shortage should be handled in the >> operator... should we do anything with leftovers?
                        LoadArrayItem(datastrm, array);
                }
            }
            else
            {
                std::istringstream datastrm(data);
                while(datastrm) //there were no commas, so assume the user put in the right amount in the right places!
                    LoadArrayItem(datastrm, array);
            }
        }
    }
    
    template <typename T> inline bool X3DNode::LoadItem(const boost::property_tree::ptree &x3d, std::string name, T &item)
    {
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>."+name);
        if(a)
        {
            std::stringstream v(a.get());
            v >> item;
            return true;
        }
        else
            return false;
    }
    
    template <> inline bool X3DNode::LoadItem(const boost::property_tree::ptree &x3d, std::string name, bool &item)
    {
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>."+name);
        if(a)
        {
            item = IsTrue(a.get());
            return true;
        }
        else
            return false;
    }

}

#endif

