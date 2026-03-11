// Tell emacs -*- c++ -*-
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
// Cache.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for caches that determine when and how
 *  Cacheables are loaded and/or unloaded.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 7/16/2009
 */
/*---------------------------------------------------------------------------*/

#include "utilities/Cache.h"
#include "utilities/Cacheable.h"
#include <limits>

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
Cache::Cache()
    : maxSize(-1.0), targetSize(-1.0), loadedSize(0.0),currentPolicy(0),
  settingsChanged(false), loadRequestsChanged(false),
  unloadRequestsChanged(false), loadedSizeChanged(false)
{}

/*---------------------------------------------------------------------------*/
Cache::~Cache()
{}

/*-------------------------Actions & Notifications---------------------------*/
/*---------------------------------------------------------------------------*/
// Requests that the passed cacheable be loaded
// @param c Cacheable to be loaded
// @param min Minimum amount requested to load (0.0 to 1.0)
// @param max Maximum amount requested to load (0.0 to 1.0)
void Cache::requestLoad(Cacheable *c, float min, float max)
{
    loadRequests[c] = std::shared_ptr<CacheRequest>(new CacheRequest(c, min, max));
    if (unloadRequests.find(c) != unloadRequests.end())
        removeUnloadRequest(c);
    loadRequestsChanged = true;
}

/*---------------------------------------------------------------------------*/
// Requests that the passed cacheable be unloaded
// @param c Cacheable to be unloaded
// @param min Minimum amount requested to unload (0.0 to 1.0)
// @param max Maximum amount requested to unload (0.0 to 1.0)
void Cache::requestUnload(Cacheable *c, float min, float max)
{
    unloadRequests[c] = std::shared_ptr<CacheRequest>(new CacheRequest(c, min, max));
    if (loadRequests.find(c) != loadRequests.end())
        removeLoadRequest(c);
    unloadRequestsChanged = true;
}

/*---------------------------------------------------------------------------*/
// Processes a load request and as many unload requests as needed
// \return true if a request was processed, false otherwise
bool Cache::processRequest()
{
    Cacheable *toLoad;
    std::shared_ptr<Cache::CacheRequest> request;
    float toLoadMinSize = 0.0;
    float startingSize = 0.0;
    float freeSpace, max, status;

    processUnloadRequests();

    //choose the next request to work on
    request = findNextLoadItem();
    if (!request)
    {
        settingsChanged = false;
        return false;
    }

    //work on the request
    toLoad = request->c;
    startingSize = toLoad->getCurrentSize(); //remember this for updating our size later
    toLoadMinSize = toLoad->getFullSize() * request->min;
    freeSpace = unloadToAccommodate(toLoadMinSize);
    max = freeSpace / toLoad->getFullSize();
    status = toLoad->load(request->min, max, currentPolicy);

    //update the books
    if (status < 0 || status >= 1.0)
        removeLoadRequest(toLoad);
    loadedSize += toLoad->getCurrentSize() - startingSize;
    if (status >= 0)
        loadedItems.insert(toLoad); //(doesn't insert if already there)

    settingsChanged = false;

    return true;
}

/*---------------------------------------------------------------------------*/
// returns the next item that should be loaded
std::shared_ptr<Cache::CacheRequest> Cache::findNextLoadItem()
{
    CacheRequestMap::iterator it;
    double minDistance = std::numeric_limits<double>::max(), min;
    std::shared_ptr<Cache::CacheRequest> retval;

    for (it = loadRequests.begin(); it != loadRequests.end(); it++)
    {
        min = (*it).first->getMeasures(currentPolicy)[0];
        if (min < minDistance)
        {
            retval = (*it).second;
            minDistance = min;
        }
    }
    loadRequestsChanged = false;
    return retval;
}

/*---------------------------------------------------------------------------*/
// returns the next item that should be (at least partially) unloaded
// @param amount The amount of memory sought to be unloaded
// @param min return value of "min" parameter to use in unloading
// \return pointer to the next Cacheable to unload (NULL if none)
Cacheable *Cache::findNextUnloadItem(float amount, float &min)
{
    CacheableSet::iterator it;
    Cacheable *c, *retval = NULL;
    double maxDist = -std::numeric_limits<double>::max(), *distances;
    double minDist = std::numeric_limits<double>::max();
    float releaseTarget;

    releaseTarget = loadedSize + amount - targetSize;
    for (it = loadedItems.begin(); it != loadedItems.end(); it++)
    {
        c = *it;
        distances = c->getMeasures(currentPolicy);
        if (!distances)
            continue;
        if (maxDist < *distances)
        {
            maxDist = *distances;
            retval = c;
            min = releaseTarget / c->getFullSize();
        }
        if (minDist > *distances)
            minDist = *distances;
    }
    if (minDist == maxDist) //can't really pick one if they're all the same... (causes cycles)
        return NULL;
    return retval;
}

/*---------------------------------------------------------------------------*/
// Notify the Cache that the passed item has unloaded itself
// and should no longer be kept track of (at least for now).
// @param c Cacheable that should be "forgotten" about
void Cache::itemUnloaded(Cacheable *c)
{
    if (loadedItems.erase(c))
        loadedSizeChanged = true;
    if (loadRequests.erase(c))
        loadRequestsChanged = true;
    if (unloadRequests.erase(c))
        unloadRequestsChanged = true;
}

/*---------------------------------------------------------------------------*/
// Processes all unload requests and empties the unloadRequests list.
void Cache::processUnloadRequests()
{
  CacheRequestMap::iterator it;

  updateLoadedSize();
  for (it = unloadRequests.begin(); it != unloadRequests.end(); it++)
      unloadItem((*it).second->c, (*it).second->max);

  unloadRequests.clear();
  unloadRequestsChanged = false;
}

/*---------------------------------------------------------------------------*/
// unloads any memory necessary to make the passed amount fit when
// loaded, dipping into already-loaded items if need be.  It
// can be assumed that all unload requests have been processed
// and that loadedSize is up-to-date at the moment this method is
// called.
float Cache::unloadToAccommodate(float amount)
{
    Cacheable *toUnload;
    float min;
    double distance, min_distance = std::numeric_limits<double>::max();
    bool wentInLoop = false;
    float trueTarget = targetSize;

    while (loadedSize + amount > trueTarget) //loadedSize decreases here
    {
        toUnload = findNextUnloadItem(amount, min);
        if (!toUnload)
            break;
        distance = toUnload->getMeasures(currentPolicy)[0];
        if (min_distance > distance)
            min_distance = distance;
        unloadItem(toUnload, min);
        wentInLoop = true;
        trueTarget = maxSize;
    }
    if (wentInLoop)
        purgeLoadRequestsBeyond(min_distance);
    return maxSize - loadedSize;
}

/*---------------------------------------------------------------------------*/
// removes load requests where the Cacheable item is at or above
// the passed distance.
// @param distance minimum distance at which to drop requests
void Cache::purgeLoadRequestsBeyond(double distance)
{
    CacheRequestMap::iterator it;

    for (it = loadRequests.begin(); it != loadRequests.end();)
    {
        if (distance <= (*it).first->getMeasures(currentPolicy)[0])
        {
            loadRequests.erase(it++);
            loadRequestsChanged = true;
        }
        else
            it++;
    }
}

/*------------------------Private utility methods----------------------------*/
/*---------------------------------------------------------------------------*/
void Cache::updateLoadedSize()
{
    CacheableSet::iterator it;

    if (!loadedSizeChanged)
        return;

    loadedSize = 0;
    for (it = loadedItems.begin(); it != loadedItems.end(); it++)
        loadedSize += (*it)->getCurrentSize();
    loadedSizeChanged = false;
}

/*---------------------------------------------------------------------------*/
void Cache::removeLoadRequest(Cacheable *c)
{
    if (loadRequests.erase(c))
        loadRequestsChanged = true;
}

/*---------------------------------------------------------------------------*/
void Cache::removeUnloadRequest(Cacheable *c)
{
    if (unloadRequests.erase(c))
        unloadRequestsChanged = true;
}

/*---------------------------------------------------------------------------*/
void Cache::unloadItem(Cacheable *c, float min)
{
    float startingSize = c->getCurrentSize();

    if (c->unload(min, currentPolicy) <= 0)
        loadedItems.erase(c);
    loadedSize -= startingSize - c->getCurrentSize();
}