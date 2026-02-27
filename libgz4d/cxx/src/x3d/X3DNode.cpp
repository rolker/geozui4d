#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DNode.h>
#include <gz4d/glo/Error.h>

#include <sstream>

#include <gz4d/x3d/X3DTransform.h>
#include <gz4d/x3d/X3DShape.h>
#include <gz4d/x3d/X3DGeoLocation.h>
#include <gz4d/X3D.h>
#include <gz4d/x3d/X3DGroupingNode.h>
#include <gz4d/x3d/X3DTouchSensor.h>

gz4d::X3DNode::X3DNode()
{
}

gz4d::SFNode gz4d::X3DNode::newNode(std::string nodeType, boost::property_tree::ptree x3d, X3D* root)
{
    SFNode ret;

    if(nodeType == "Transform")
        ret = SFNode(new X3DTransform, x3d, root);

    else if(nodeType == "GeoLocation")
    {
        ret = SFNode(new X3DGeoLocation, x3d, root);
        if (!root->GetGeoNode())
            root->SetGeoNode(ret);
    }

    else if(nodeType == "Shape")
        ret = SFNode(new X3DShape, x3d, root);

    else if(nodeType == "Group")
        ret = SFNode(new X3DGroup, x3d, root);

    else if(nodeType == "TouchSensor")
        ret = SFNode(new X3DTouchSensor, x3d, root);

    else if(nodeType == "ProtoInstance")
    {

        SFString name;
        SFNodeCreator creator;
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>.name");
        if(a)
            name = a.get();
        creator = root->getCreateMethod( name );
        if (creator)
            ret = creator( x3d, root );
    }

    return ret;
}

void gz4d::X3DNode::Load(boost::property_tree::ptree const &x3d, X3D* root, SFNode safe_this)
{
    boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>.USE");
    if (a)
    {
        safe_this->USE = a.get();
        safe_this->useNode = root->getNode(safe_this->USE);
        return;
    }
    a = x3d.get_optional<std::string>("<xmlattr>.DEF");
    if(a)
    {
        safe_this->DEF = a.get();
        root->registerNode(safe_this->DEF,safe_this);
    }
    safe_this->Load(x3d, root);
}

void gz4d::X3DNode::Draw()
{
    if (preDraw())
        return;
}

/// Draw ref target if applicable.
/// @return true if we are a ref, and therefore drew the target, false if we need to render from our own data.
bool gz4d::X3DNode::preDraw()
{
    if (useNode)
    {
        useNode->Draw();
        glo::Error();
        return true;
    }
    return false;
}



gz4d::SFNode::SFNode(X3DNode *n, boost::property_tree::ptree x3d, X3D* root) : std::shared_ptr<X3DNode>(n)
{
    if(n)
        X3DNode::Load(x3d, root, *this);
}

gz4d::SFNode::SFNode(SFNode const &n, boost::property_tree::ptree x3d, X3D* root) : std::shared_ptr<X3DNode>(n)
{
    if(n)
        X3DNode::Load(x3d, root, *this);
}

void gz4d::X3DNode::LoadArray(const boost::property_tree::ptree &x3d, std::string name, MFString &array)
{
    boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>."+name);
    if(a)
    {
        std::string data = a.get();
        std::string::size_type start = data.find("\"");
        std::string::size_type end(0);

        if(start == std::string::npos)
        {
            array.push_back(SFString(data));
        }
        else
        {
            for(; start != std::string::npos; start = data.find("\"", end + 1))
            {
                start++; //get past the quote
                end = data.find("\"", start);
                if (end == std::string::npos)
                    break; //&&& should throw exception

                array.push_back(SFString(data.substr(start, end - start)));
            }
        }
    }
}

gz4d::X3DGroupingNode * gz4d::X3DNode::getGroupingNode()
{
    return dynamic_cast<X3DGroupingNode*>(this);
}
