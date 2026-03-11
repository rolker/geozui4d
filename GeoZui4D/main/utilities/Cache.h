// Tell emacs -*- c++ -*-
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/

/// \file
/// \brief Base class for more specific caches

/*---------------------------------------------------------------------------*/
// Cache.h
/*---------------------------------------------------------------------------*/

class Cache;

#ifndef CACHE_H
#define CACHE_H

class Cacheable;
#include <map>
#include <set>
#include <memory>

/** 
 *  \brief Base Cache class.  Uses Cacheable.
 *
 *	This is the base class for caches that determine when and how
 *  Cacheables are loaded and/or unloaded.
 *
 *  \par This class is responsible for the following.
 *	\li &&&
 *	\li &&&
 *
 *	\author Matt Plumlee (mdp)
 *  \date 7/16/2009
 */
/*---------------------------------------------------------------------------*/
class Cache
{
	public:
		/*---Constructors/Destructors---*/
        Cache();
        virtual ~Cache();

		/// \name Cache actions and notifications
		/// @{
		/// Requests that the passed cacheable be loaded
        /// @param c Cacheable to be loaded
        /// @param min Minimum amount requested to load (0.0 to 1.0)
        /// @param max Maximum amount requested to load (0.0 to 1.0)
		virtual void requestLoad(Cacheable *c, float min = 1.0, float max = 1.0);
        ///
		/// Requests that the passed cacheable be unloaded
        /// @param c Cacheable to be unloaded
        /// @param min Minimum amount requested to unload (0.0 to 1.0)
        /// @param max Maximum amount requested to unload (0.0 to 1.0)
		virtual void requestUnload(Cacheable *c, float min = 1.0, float max = 1.0);
        ///
		/// Processes all unload requests and a load request (if present)
        /// \return true if a request was processed, false otherwise
        virtual bool processRequest();
		///
        /// Notify the Cache that the passed item has unloaded itself
        /// and should no longer be kept track of (at least for now).
        /// @param c Cacheable that should be "forgotten" about
        virtual void itemUnloaded(Cacheable *c);
		/// @}

		/// \name Cache settings
		/// @{
		/// Set the target amount of memory the cache should try to
        /// keep loaded.
		/// @param newSize new target amount of memory (in child-determined units--could be as a percentage of maxSize)
        virtual void setTargetSize(float newSize)
        {    targetSize = newSize; settingsChanged = true;}
        ///
		/// Set the maximum amount of memory the cache can cause to load
		/// @param newSize new maximum amount of memory (in child-determined units)
        virtual void setMaxSize(float newSize)
        {    maxSize = newSize; settingsChanged = true;}
        ///
		/// Set the current policy for deciding how to load and unload items
		/// @param newPolicy new policy to use (given meaning by the child class)
        virtual void setPolicy(int newPolicy)
        {    currentPolicy = newPolicy; settingsChanged = true;}
		/// @}
		
	protected:
        struct CacheRequest {
            Cacheable *c;
            float min;
            float max;
            CacheRequest(Cacheable *cl, float mn, float mx)
                : c(cl), min(mn), max(mx) {};
        };
        typedef std::set<Cacheable *> CacheableSet;
        typedef std::map<Cacheable *, std::shared_ptr<CacheRequest> > CacheRequestMap;

        /// \name Request-processing helper methods (to be defined by children)
		/// @{
        /// returns the next item that should be loaded
        virtual std::shared_ptr<CacheRequest> findNextLoadItem();
        ///
        /// returns the next item that should be (at least partially) unloaded
        /// @param amount The amount of memory sought to be unloaded
        /// @param min return value of "min" parameter to use in unloading
        /// \return pointer to the next Cacheable to unload (NULL if none)
        virtual Cacheable *findNextUnloadItem(float amount, float &min);
        ///
        /// unloads any memory necessary to make the passed amount fit when
        /// loaded, dipping into already-loaded items if need be.  It
        /// can be assumed that all unload requests have been processed
        /// and that loadedSize is up-to-date at the moment this method is
        /// called.
        /// @param amount minimum amount of memory to ensure available (in child-determined units)
        /// \return amount actually free after unloading
        virtual float unloadToAccommodate(float amount);
        ///
        /// removes load requests where the Cacheable item is at or above
        /// the passed distance.
        /// @param distance minimum distance at which to drop requests
        virtual void purgeLoadRequestsBeyond(double distance);
        ///
        /// Processes all unload requests and empties the unloadRequests list.
        virtual void processUnloadRequests();
        /// @}

        float maxSize, targetSize, loadedSize;
        int currentPolicy;
        bool settingsChanged, loadRequestsChanged, unloadRequestsChanged;
        bool loadedSizeChanged;

        CacheableSet loadedItems;
        CacheRequestMap loadRequests, unloadRequests;

    private:
        void updateLoadedSize();
        void removeLoadRequest(Cacheable *c);
        void removeUnloadRequest(Cacheable *c);
        void unloadItem(Cacheable *c, float min);
};

#endif
