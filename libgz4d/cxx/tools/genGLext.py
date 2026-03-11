#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


current_function = None
functions = {}

for l in file('../specs/gl.spec'):
    line = l.strip()
    if len(line) and line[0] != '#' and not line.startswith('passthru:'):
        if line[-1] == ')':
            current_function = 'gl'+line.split('(')[0]
        else:
            if current_function is not None:
                parts = line.split()
                if parts[0] == 'version':
                    functions[current_function] = float(parts[1])

h = file('Extension.h','w')
h.write('''#ifndef GZ4D_GLO_EXTENSION_H
#define GZ4D_GLO_EXTENSION_H

#include <gz4d/glo.h>
#include <GL/glext.h>
#include <string>

namespace gz4d
{
    namespace glo
    {
''')

for k,v in functions.iteritems():
    if v > 1.1:
        t = 'PFN'+k.upper()+'PROC'
        h.write('        extern '+t+' '+k+';\n')

h.write('''
        class GZ4DEXPORT Extension
        {
            static bool initialized;
            static void init();
            public:
                Extension();
        };
    }
}

#endif

''')

cpp = file('Extension.cpp','w')
cpp.write('''#define GZ4D_INTERNAL

#include <gz4d/glo/Extension.h>
#include <gz4d/glo/Error.h>
#include <boost/algorithm/string.hpp>

bool gz4d::glo::Extension::initialized = false;
''')

for k,v in functions.iteritems():
    if v > 1.1:
        t = 'PFN'+k.upper()+'PROC'
        cpp.write (t+' '+'gz4d::glo::'+k+' = NULL;\n')

cpp.write('''
void gz4d::glo::Extension::init()
{
    if(!initialized)
    {
''')

for k,v in functions.iteritems():
    if v > 1.1:
        t = 'PFN'+k.upper()+'PROC'
        cpp.write('        '+k+' = ('+t+') wglGetProcAddress("'+k+'");\n')

cpp.write('''
    }
    initialized = true;
}

gz4d::glo::Extension::Extension()
{
    init();
}

''')
