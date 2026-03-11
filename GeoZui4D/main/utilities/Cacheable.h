// Tell emacs -*- c++ -*-
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/

/// \file
/// \brief Lightweight interface for items with un/loadable components: class Cacheable

/*---------------------------------------------------------------------------*/
// Cacheable.h
/*---------------------------------------------------------------------------*/

class Cacheable;

#ifndef CACHEABLE_H
#define CACHEABLE_H

/** 
 *  \brief Cacheable class.
 *
 *	This is the base class for entities that have components that
 *  can be dynamically loaded and unloaded.  This interface facilitates
 *  communication between the cache and the cacheable entity.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 7/15/2009
 */
/*---------------------------------------------------------------------------*/
class Cacheable
{
	public:
		/// \name Cache actions
		/// @{
		/// Causes the entity to load its components
        /// @param min Minimum amount it should try to load (0.0 to 1.0)
        /// @param max Maximum amount it should try to load (0.0 to 1.0)
        /// @param policy An indication of what min and max actually mean (mutually decided upon by derived classes)
        /// \return amount loaded (0.0 to 1.0, -1.0 indicates an error)
		virtual float load(float min = 1.0, float max = 1.0, int policy = 0) = 0;
        float load(int policy, float min = 1.0, float max = 1.0) {return load(min, max, policy);}
		///
		/// Causes the entity to unload its components
        /// @param min Minimum amount it should try to unload (0.0 to 1.0)
        /// @param policy An indication of what min actually means (mutually decided upon by derived classes)
        /// \return amount still loaded (0.0 to 1.0, -1.0 indicates an error)
		virtual float unload(float min = 1.0, int policy = 0) = 0;
        float unload(int policy, float min = 1.0) {return unload(min, policy);}
        ///
        /// Causes the entity to update its sense of when it was last
        /// needed/accessed/required/used.  The default behavior is to
        /// call load() with a request for 100% of the entity to be loaded.
        /// @param policy An indication of what the refresh pertains to (mutually decided upon by derived classes)
        virtual void refresh(int policy = 0) {load(1.0, 1.0, policy);}
		/// @}

		/// \name Cache queries
		/// @{
		/// \return the approximate current size of load/unloadable components
		virtual float getCurrentSize() = 0;
        ///
		/// \return the approximate total size of all loadable components
		virtual float getFullSize() = 0;
        ///
        /// @param policy An indication of what quantum is meant (mutually decided upon by derived classes)
		/// \return minimum amount the entity can load (0.0 to 1.0)
        virtual float getSmallestQuantum(int policy = 0) {return 1.0 - (getCurrentSize() / getFullSize());}
		///
        /// Returns a list of measures of the "distance" of this Cacheable
        /// item from being needed again.  Larger numbers indicate greater
        /// distance, and therefore higher likelihood of getting unloaded.
        /// A NULL return value indicates that this item can't be unloaded
        /// right now.
        /// @param policy An indication of what measures are of interest (mutually decided upon by derived classes)
        /// \return array of measures the cache will use to make decisions (mutually decided upon by derived classes)
		virtual double *getMeasures(int policy = 0) = 0;
		/// @}

};

#endif
