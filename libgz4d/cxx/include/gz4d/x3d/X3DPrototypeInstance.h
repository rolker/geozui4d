// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

/** Note: This class is used as a base class for all ProtoInstance uses.  It
 *  does not represent a generic ProtoInstance for X3D prototypes, at least
 *  not at this time.
 */
#ifndef GZ4D_X3DPROTOTYPEINSTANCE_H
#define GZ4D_X3DPROTOTYPEINSTANCE_H

#include <gz4d/x3d/X3DNode.h>

namespace gz4d
{
    class GZ4DEXPORT X3DPrototypeInstance: public X3DNode
    {
    public:
        X3DPrototypeInstance();
        virtual void LoadStarted() {}
        virtual void LoadField( SFString name, const boost::property_tree::ptree &x3d, X3D* root);
        virtual void LoadCompleted() {}
        virtual void Draw();

    protected:
        SFNode drawablePart;
        MFNode otherParts;

    private:
        virtual void Load( boost::property_tree::ptree x3d, X3D* root); //don't override this!
    };
};

#endif
