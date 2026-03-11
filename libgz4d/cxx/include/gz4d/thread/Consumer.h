// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CONSUMER_H
#define GZ4D_CONSUMER_H

#include <gz4d/gz4d.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <boost/thread/thread.hpp>


namespace gz4d
{
    template <typename CT, typename VT> class Consumer : boost::noncopyable
    {
        public:
            typedef VT input_type;
            typedef typename ThreadedQueue<input_type>::Ptr input_queue_type;

            Consumer(std::shared_ptr<CT> &c, std::size_t maxQueueSize=0);
            ~Consumer();

            void Start(std::shared_ptr<CT> &c);
            input_queue_type &GetInQueue(){return inQueue;}

            void operator()(std::shared_ptr<CT> &c, input_queue_type &q);
            void operator()(CT &c, input_queue_type &q);
            void operator()();

            void WaitEmpty();
        protected:
            Consumer(std::size_t maxQueueSize=0);

            virtual void Consume(VT v){}
        private:
            void Start();
            boost::thread consumerThread;
            input_queue_type inQueue;
    };

    template <typename CT, typename VT> inline Consumer<CT,VT>::Consumer(std::shared_ptr<CT> &c, std::size_t maxQueueSize)
    :inQueue(new ThreadedQueue<VT>(maxQueueSize))
    {
        Start(c);
    }

    template <typename CT, typename VT> inline Consumer<CT,VT>::Consumer(std::size_t maxQueueSize)
    :inQueue(new ThreadedQueue<VT>(maxQueueSize))
    {
        Start();
    }
    
    template <typename CT, typename VT> inline void Consumer<CT,VT>::Start(std::shared_ptr<CT> &c)
    {
        consumerThread = std::move(boost::thread(boost::ref(*this),c,inQueue));
    }

    template <typename CT, typename VT> inline void Consumer<CT,VT>::Start()
    {
        consumerThread = std::move(boost::thread(boost::ref(*this)));
    }

    template <typename CT, typename VT> inline Consumer<CT,VT>::~Consumer()
    {
        if(consumerThread.joinable())
        {
            consumerThread.interrupt();
            consumerThread.join();
        }
    }

    template <typename CT, typename VT> inline void Consumer<CT,VT>::operator()(std::shared_ptr<CT> &c, input_queue_type &q)
    {
        if(c)
            operator()(*c,q);
    }

    template <typename CT, typename VT> inline void Consumer<CT,VT>::operator()(CT &c, input_queue_type &q)
    {
        while (!boost::this_thread::interruption_requested())
        {
            boost::thread::yield();
            try
            {
                if(q->wait_not_empty(1.0))
                {
                    boost::this_thread::disable_interruption di;
                    VT v = q->pop();
                    c.Consume(v);
                }
            }
            catch (boost::thread_interrupted)
            {
                break;
            }
        }
    }

    template <typename CT, typename VT> inline void Consumer<CT,VT>::operator()()
    {
        while (!boost::this_thread::interruption_requested())
        {
            boost::thread::yield();
            try
            {
                if(inQueue->wait_not_empty(1.0))
                {
                    boost::this_thread::disable_interruption di;
                    VT v = inQueue->pop();
                    this->Consume(v);
                }
            }
            catch (boost::thread_interrupted)
            {
                break;
            }
        }
    }

    template <typename CT, typename VT> inline void Consumer<CT,VT>::WaitEmpty()
    {
        while(!inQueue->empty())
            boost::this_thread::yield();
    }

#ifdef GZ4D_BOOST_PYTHON
    template<typename CT, typename VT> void WrapConsumer(char const *name)
    {
        boost::python::class_<gz4d::Consumer<CT,VT>, std::shared_ptr<Consumer<CT,VT> >, boost::noncopyable >(name, boost::python::init<std::shared_ptr<CT>& >())
                .def("GetInQueue", &gz4d::Consumer<CT,VT>::GetInQueue,boost::python::return_value_policy<boost::python::copy_non_const_reference>())
                .def("WaitEmpty", &gz4d::Consumer<CT,VT>::WaitEmpty)
                ;
    }

#endif

}

#endif
