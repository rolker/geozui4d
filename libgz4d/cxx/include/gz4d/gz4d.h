// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_H
#define GZ4D_H


/** \mainpage libgz4d
Collection of modules for rapid and flexible development of interactive visualization systems. The goal is for the modules to be independent of any toolkit or scripting language. The only assumptions made are the use of C++ and OpenGL.

\section Modules

Here are the main modules.

\ref glo

\ref scene

\ref interfaces


\namespace gz4d
All libgz4d entities belong to the gz4d namespace to help avoid name collisions.


\defgroup base Base
Base gz4d types and classes.

\defgroup data Data
Contains modules for rendering, internal representation and external data access.


\defgroup container Containers
\ingroup data
Internal representation of various data objects.

\defgroup formats File formats
\ingroup data
Support for reading and/or writing various file formats.

\defgroup renderer OpenGL Renderers
\ingroup data
OpenGL views of data from containers. A container type may be representable by multiple renderers, and a renderer may be able to render from different container types. For example, raster data may be rendered as an image on a billboard, or as a terrain map.

\defgroup glo GL Objects
Classes that encapsulate OpenGL functionality.


\defgroup interfaces Interfaces
This is where device or framework specific classes can be found.

\defgroup devices Devices
Various input/output devices.
\ingroup interfaces

\defgroup frameworks Frameworks
\ingroup interfaces
Code to help interface with GUI frameworks and OpenGL context providers.

\defgroup scene Scene
Classes that help manage an OpenGL scene.

\defgroup utilities Utilities
Helpful, general pupose classes.
*/

#include <gz4d/config.h>

namespace gz4d{}

#endif
