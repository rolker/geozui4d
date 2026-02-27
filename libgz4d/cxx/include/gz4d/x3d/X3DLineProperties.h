#ifndef GZ4D_X3DLINEPROPERTIES_H
#define GZ4D_X3DLINEPROPERTIES_H

#include "X3DAppearanceChildNode.h"

namespace gz4d
{
    class X3DLineProperties : public X3DAppearanceChildNode
    {
        public:
            X3DLineProperties();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();

            enum Linetype {Solid = 1, Dashed, Dotted, Dashed_dotted,
                           Dash_dot_dot, Single_arrow, Single_dot,
                           Double_arrow, Stitch_line, Chain_line,
                           Center_line, Hidden_line, Phantom_line,
                           Break_line_1, Break_line_2, User_specified_dash};

        protected:
            SFBool applied;
            SFInt32 linetype;
            SFFloat linewidthScaleFactor;

            bool stippling;
            static unsigned short stipplings[User_specified_dash + 1]; //all from Single_arrow on are not implemented, but appear as Solid
    };
};

#endif
