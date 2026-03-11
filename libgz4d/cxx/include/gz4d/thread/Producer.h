// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_PRODUCER_H
#define GZ4D_PRODUCER_H

#include <gz4d/gz4d.h>
#include <boost/thread/thread.hpp>
#include <gz4d/thread/ThreadedQueue.h>
#include <list>

namespace gz4d
{
    template <typename T> class Producer
    {
        public:
            typedef T output_type;
            typedef typename ThreadedQueue<T>::Ptr output_queue_type;

            Producer();
            Producer(Producer const &o);
            
            void AddOutQueue(output_queue_type &q);
            output_queue_type NewOutQueue(std::size_t maxSize = 0);
            
        protected:
            void Send(T data);
            bool NoOutputQueue();

        private:
            mutable boost::mutex queuesMutex;
            std::list<typename ThreadedQueue<T>::WeakPtr> outQueues;
    };

    template <typename T> inline Producer<T>::Producer()
    {
    }
    
    template <typename T> inline Producer<T>::Producer(Producer const &o)
    {
        boost::mutex::scoped_lock lock(o.queuesMutex);
        outQueues = o.outQueues;
    }
    
    template <typename T> inline void Producer<T>::AddOutQueue(output_queue_type &q)
    {
        boost::mutex::scoped_lock lock(queuesMutex);
        outQueues.push_back(q);
    }

    template <typename T> inline typename Producer<T>::output_queue_type Producer<T>::NewOutQueue(size_t maxSize)
    {
        output_queue_type ret(new ThreadedQueue<T>(maxSize));
        AddOutQueue(ret);
        return ret;
    }

    template <typename T> inline void Producer<T>::Send(T data)
    {
        boost::mutex::scoped_lock lock(queuesMutex);
        for(typename std::list<typename ThreadedQueue<T>::WeakPtr>::iterator q = outQueues.begin(); q != outQueues.end();)
        {
            if(output_queue_type qptr = q->lock())
            {
                qptr->push(data);
                ++q;
            }
            else
            {
                //std::cerr << "gz4d::Producer::Send: Cleaning up expired queue." << std::endl;
                q = outQueues.erase(q);
            }
        }
        boost::thread::yield();
    }

    template <typename T> inline bool Producer<T>::NoOutputQueue()
    {
        boost::mutex::scoped_lock lock(queuesMutex);
        return outQueues.empty();
    }


#ifdef GZ4D_BOOST_PYTHON


    template<typename T> void WrapProducer(char const *name)
    {
        
        boost::python::class_<gz4d::Producer<T>, boost::python::bases<>, std::shared_ptr<gz4d::Producer<T> > >(name)
            .def("AddOutQueue", &gz4d::Producer<T>::AddOutQueue)
            .def("NewOutQueue", &gz4d::Producer<T>::NewOutQueue)
            ;
    }
    
#endif

}

#endif
