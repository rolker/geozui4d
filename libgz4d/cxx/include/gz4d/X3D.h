// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3D_H
#define GZ4D_X3D_H

//gz4d_cmake depends gz4d_x3d

#include<gz4d/gz4d.h>
#include <map>
#include <gz4d/x3d/X3DGroup.h>
#include <gz4d/x3d/X3DPrototypeRecord.h>
#include <functional>
#include <gz4d/glo/DisplayList.h>

#ifndef GZ4D_INTERNAL
    #define GZ4D_LIB_NAME gz4d_x3d
    #include <gz4d/config/auto_link.h>
#endif


namespace gz4d
{
    namespace glo
    {
        class Context;
    }

    /**
    \ingroup scene
    */
    class GZ4DEXPORT X3D
    {
    public:
        typedef std::shared_ptr<X3D> Ptr;

        X3D(std::shared_ptr<glo::Context> c);
        ~X3D();

        void Reset(bool resetTypesToo = true);

        void Draw();

        bool Load(std::string const &fn);
        bool LoadFragment(std::string const &fn);

        SFNode getNode(SFString nodeName);
        void registerNode(SFString nodeName, SFNode node);
        bool getShowNormals() {return showNormals;};
        std::string const &getFilename() const {return filename;};
        MFNode const &getRootNodes(){return root.getChildren();}

        void registerProto( SFString name, ProtoRecord record );
        void registerNodeType( SFString name, SFNode (*createMethod)(boost::property_tree::ptree x3d, X3D* root) );

        ProtoRecord getProtoRecord( SFString name ) { return prototypeList[name]; }
        SFNodeCreator getCreateMethod( SFString name ) { return nodeFactory[name]; }
        static std::shared_ptr<X3D> New(std::shared_ptr<glo::Context> c);

        SFNode GetGeoNode(){return geoNode;}
        void SetGeoNode(SFNode gn){geoNode = gn;}

        //void Handle(ui::Event &e){}
    private:
        friend class X3DAppearance;
        std::shared_ptr<glo::Context> context;
        bool showNormals;

        X3DGroup root; ///< Contains root level nodes.
        std::map<std::string, SFNode> nodeList;
        std::map<std::string, ProtoRecord> prototypeList; //proto names to prototype record names
        std::map<std::string, SFNodeCreator> nodeFactory; //type names to create(...) methods

        SFNode geoNode;

        glo::DisplayList displayList;
        std::string filename;
    };

};

#endif
