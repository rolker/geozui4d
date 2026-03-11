// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CRS_AXIS_H
#define GZ4D_CRS_AXIS_H

#include <gz4d/Angle.h>

namespace gz4d
{
    namespace crs
    {
        namespace dir
        {
            struct Up 
            {
            };
            
            struct Down
            {
            };
            
            struct Left
            {
            };
            
            struct Right
            {
            };
            
            struct Near
            {
            };
            
            struct Far
            {
            };
            
            /// Axis positive direction is towards the future.
            struct Future
            {
            };
            
            /// Axis positive direction is towards the past.
            struct Past
            {
            };
            
            /// Axis positive direction is towards higher pixel column.
            struct ColumnPositive
            {
            };
            
            /// Axis positive direction is towards lower pixel column.
            struct ColumnNegative
            {
            };
            
            /// Axis positive direction is towards higher pixel row.
            struct RowPositive
            {
            };
            
            /// Axis positive direction is towards lower pixel row.
            struct RowNegative
            {
            };
        };
        
        namespace axis
        {
            enum Units {
                U_None,
                U_Meter,
                U_Unknown
            };
            
            template <int ID> struct Unit
            {
                enum {unit = ID};
            };
            
            typedef Unit<U_Meter> Meter;
            typedef Unit<U_None> Unitless;
            typedef Unit<U_Unknown> UnknownUnit;

            template <typename B, typename DIR, typename UNITS> struct Axis: B, DIR, UNITS
            {
                enum {index = B::index+1};
            };
            
            struct InitAxis
            {
                enum {index = -1};
            };
            
            template <typename B=InitAxis> struct NullAxis
            {
                enum {index = B::index};
            };
            
            
            
        }
    }
}

#endif
                