// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DISPLAYLIST_H
#define GZ4D_DISPLAYLIST_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>

namespace gz4d
{
    namespace glo
    {
        /// OpenGL display list and staleness flag.
        /// \ingroup glo
        class DisplayList
        {
            public:
                DisplayList():list(0),needsBuild(true){}
                ~DisplayList()
                {
                    if (list)
                        glDeleteLists(list,1);
                }

                /// Called when the list needs to be rebuilt.
                void Rebuild(){needsBuild = true;}

                /// Execute the list, if possible.
                /// Returns true on success and false if the list needs to 
                /// be built. If autoNew is true and the list needs to be 
                /// rebuilt, it returns having already started a new list ready to accept new commands.
                bool Call(bool autoNew=true)
                {
                    if(!list)
                    {
                        list = glGenLists(1);
                        if(autoNew)
                            glNewList(list,GL_COMPILE);
                        return false;
                    }
                    if(needsBuild)
                    {
                        if(autoNew)
                            glNewList(list,GL_COMPILE);
                        return false;
                    }
                    glCallList(list);
                    return true;
                }


                /// Start specifying a new list of commands for the list.
                void New()
                {
                    if(!list)
                        list = glGenLists(1);
                    glNewList(list,GL_COMPILE);
                }


                /// Finish specifing commands.
                /// If autoCall is true, the list is also executed.
                void End(bool autoCall=true)
                {
                    glEndList();
                    if(autoCall)
                        glCallList(list);
                    needsBuild = false;
                }

            private:
                GLuint list;
                bool needsBuild; ///< Flag indicating the list needs to be rebuilt.
        };

    }
}

#endif
