#ifndef GZ4D_X3DINDEXEDLINESET
#define GZ4D_X3DINDEXEDLINESET

#include "X3DGeometryNode.h"

namespace gz4d
{
    class X3DIndexedLineSet : public X3DGeometryNode
    {
    public:
        X3DIndexedLineSet();
        void Load(const boost::property_tree::ptree &x3d, X3D* root);
        void Draw();

    protected:
        void GenerateNormals(){}

        MFNode  attrib;
        SFNode  color;
        SFNode  coord;
        SFNode  fogCoord;
        MFInt32 colorIndex;
        SFBool  colorPerVertex;
        MFInt32 coordIndex;
    };
};

#endif
