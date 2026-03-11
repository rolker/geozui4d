// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

// #include "RasterStream.h"
// #include <gz4d/data/Image.h>
// 
// template <typename type>
// void RasterStream<type>::Adapter::Consume(std::shared_ptr<gz4d::Socket::Packet> &data)
// {
//     gz4d::Time t = ((double*)data->front())[0];
//     unsigned short r = *(unsigned short *)&((*data)[10]);
//     unsigned short rows = *(unsigned short *)&((*data)[12]);
//     unsigned short cols = *(unsigned short *)&((*data)[14]);
//     if (t != time)
//     {
//         if (raster)
//         {
//             gz4d::Producer<typename gz4d::TimeSequence<typename type::Ptr >::ValueType>::Send(typename gz4d::TimeSequence<typename type::Ptr>::ValueType(time,raster));
//         }
//         time = t;
//         raster = typename type::Ptr(new type);
//         raster->Resize(cols,rows);
//     }
//     memcpy(raster->GetRow(r),&(*data)[16],cols*raster->GetElementSize());
// 
// }
// 
// template <typename type>
// void RasterStream<type>::Start(gz4d::Socket const &s)
// {
//     //producer = gz4d::SocketProducer(s);
//     producer.SetBufferSize(2048);
//     //std::shared_ptr<gz4d::ThreadedQueue<std::shared_ptr<gz4d::Socket::Packet> > > packetQueue(new gz4d::ThreadedQueue<std::shared_ptr<gz4d::Socket::Packet> >);
//     //producer.SetOutQueue(packetQueue);
//     producer.AddOutQueue(adapter.GetInQueue());
//     //producerThread = gz4d::Thread(producer);
//     //std::shared_ptr<gz4d::ThreadedQueue<typename gz4d::TimeSequence<std::shared_ptr<type> >::value_type> > imageQueue(new gz4d::ThreadedQueue<typename gz4d::TimeSequence<std::shared_ptr<type> >::value_type>);
//     //adapter.SetOutQueue(imageQueue);
//     adapter.AddOutQueue(this->GetInQueue());
//     //adapterThread = gz4d::Thread(adapter);
//     //sequenceThread = gz4d::Thread(*this);
// }
// 
// template <typename Type>
// void RasterStream<Type>::Consume(ValueType &value)
// {
//     gz4d::TimeSequence<typename Type::Ptr >::insert(value);
// }
// 
// template class RasterStream<gz4d::Raster<gz4d::Pixel> >;
// template class RasterStream<gz4d::Raster<gz4d::Vector<float,2> > >;
