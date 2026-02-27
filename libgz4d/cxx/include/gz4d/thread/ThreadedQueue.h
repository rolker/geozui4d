#ifndef GZ4D_THREADEDQUEUE_H
#define GZ4D_THREADEDQUEUE_H

#include <gz4d/gz4d.h>
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition.hpp>
//#include <boost/thread/xtime.hpp>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

namespace gz4d
{
    /// Queue to send data between threads.
    /// \ingroup threads
    template <typename T> class ThreadedQueue :public boost::noncopyable
    {
        std::queue<T> q;
        //boost::mutex mutex;
        std::mutex mutex;
        //boost::condition not_empty;
        //boost::condition not_full;
        std::condition_variable not_empty;
        std::condition_variable not_full;
        std::size_t _maxSize;
        public:
            typedef std::shared_ptr<ThreadedQueue<T> > Ptr;
            typedef std::weak_ptr<ThreadedQueue<T> > WeakPtr;


            ThreadedQueue(std::size_t maxSize=0):_maxSize(maxSize){}

            T pop()
            {
            	std::unique_lock<std::mutex> lock(mutex);
                //boost::mutex::scoped_lock lock(mutex);
                while(q.empty())
                    not_empty.wait(lock);
                T ret = q.front();
                q.pop();
                not_full.notify_all();
                return ret;
            }

            /// Waits a specified of time for the queue to become not empty.
            /// Returns true when data is present before the timeout elapses
            /// or false if queue is still empty after timeout seconds have elapses.
            bool wait_not_empty(double timeout)
            {
            	if(mutex.try_lock()){
            		mutex.unlock();
            		return true;
            	}
            	std::this_thread::sleep_for(std::chrono::milliseconds(int(timeout*1000)));
            	if(mutex.try_lock()){
            		mutex.unlock();
            		return true;
            	}
            	return false;
            }

            void push_no_wait(T value)
            {
            	std::unique_lock<std::timed_mutex> lock(mutex);
                q.push(value);
                if(_maxSize)
                    while(q.size() > _maxSize)
                        q.pop();
                not_empty.notify_all();
            }


            void push(T value)
            {
            	std::unique_lock<std::mutex> lock(mutex);
                while(_maxSize > 0 &&  q.size() >= _maxSize)
                    not_full.wait(lock);
                q.push(value);
                not_empty.notify_all();
            }

            bool empty()
            {
            	std::unique_lock<std::mutex> lock(mutex);
                return q.empty();
            }

            bool full()
            {
                if(_maxSize==0)
                    return false;
                std::unique_lock<std::mutex> lock(mutex);
                return q.size() >= _maxSize;
            }

            std::size_t size()
            {
            	std::unique_lock<std::mutex> lock(mutex);
                return q.size();
            }

            std::size_t maxSize()
            {
            	std::unique_lock<std::mutex> lock(mutex);
                return _maxSize;
            }

            void maxSize( std::size_t ms)
            {
            	std::unique_lock<std::mutex> lock(mutex);
                _maxSize = ms;
            }
    };

#ifdef GZ4D_BOOST_PYTHON
    template<typename T> void WrapThreadedQueue(const char *name)
    {
        boost::python::class_<ThreadedQueue<T>, std::shared_ptr<ThreadedQueue<T> >, boost::noncopyable >(name)
            .def("empty", &ThreadedQueue<T>::empty)
            .def("pop", &ThreadedQueue<T>::pop)
            .def("wait_not_empty", &ThreadedQueue<T>::wait_not_empty)
            ;
    }
#endif

}

#endif
