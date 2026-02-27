#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DPrototypeInstance.h>
#include <gz4d/X3D.h>
#include <iostream>

gz4d::X3DPrototypeInstance::X3DPrototypeInstance()
{
    typeName = "ProtoInstance";
}

void gz4d::X3DPrototypeInstance::Draw()
{
    if (preDraw())
        return;

    if (drawablePart)
    {
        glPushMatrix();
        drawablePart->Draw();
        glPopMatrix();
    }
}

void gz4d::X3DPrototypeInstance::Load(boost::property_tree::ptree x3d, X3D* root)
{
    LoadStarted();
    for(boost::property_tree::ptree::const_iterator n = x3d.begin(); n != x3d.end(); ++n)
    {
        //get fields.  Only fields can have children in this case (such as a field named "children")
        boost::optional<std::string> a = n->second.get_optional<std::string>("<xmlattr>.name");
        if(a)
        {
            LoadField( a.get(), n->second, root );
        }
    }
    LoadCompleted();
}

void gz4d::X3DPrototypeInstance::LoadField( SFString name,const boost::property_tree::ptree &x3d, X3D* root)
{
    /* General form for each field would be this:
    if (name == "myField")
    {
        LoadArray( x3d, "value", myFieldVar );
    }
    */
}
