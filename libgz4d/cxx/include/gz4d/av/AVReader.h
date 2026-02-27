#ifndef GZ4D_AVREADER_H
#define GZ4D_AVREADER_H

#include <boost/config.hpp>
#ifdef BOOST_MSVC
#include<stdint.h>
#endif


#include <gz4d/gz4d.h>
#include <gz4d/av/common.h>

#include <gz4d/thread/Producer.h>
#include <gz4d/sound/AudioPacket.h>
#include <gz4d/Image.h>
#include <gz4d/time/ios.h>
#include <boost/rational.hpp>

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace gz4d
{
    /// \ingroup formats
    class AVReader
    {
        public:
            typedef std::shared_ptr<AVReader> Ptr;
            
            struct Stream
            {
                std::shared_ptr<AVFormatContext> format;
                std::shared_ptr<AVCodecContext> codec;
                double time_base;
                int index;

                Stream(std::shared_ptr<AVFormatContext> &f, int index)
                :format(f),index(index)
                {
                    time_base = format->streams[index]->time_base.num/double(format->streams[index]->time_base.den);
                    AVCodecContext * cc = format->streams[index]->codec;
                    AVCodec * c = avcodec_find_decoder(format->streams[index]->codec->codec_id);
                    if(c)
                    {
                        if(avcodec_open2(cc,c,nullptr) >= 0)
                            codec = std::shared_ptr<AVCodecContext>(cc,avcodec_close);
                        else
                            std::cerr << "error opening codec" << std::endl;
                    }
                    else
                        std::cerr << "error finding codec" << std::endl;
                }
                
                virtual ~Stream(){}
                
                virtual void Decode(AVPacket &packet, bool discard=false)
                {
                    /*    std::cerr << "\t" << packet.stream_index << "\t" << packet.size;
                     *    if(packet.pts >= 0)
                     *        std::cerr << "\t" << packet.dts*time_base;
                     *    std::cerr << std::endl;*/
                }
                
                time::Duration GetDuration() const
                {
                    return seconds(format->streams[index]->duration*time_base);
                }

                int64_t GetFrameCount() const
                {
                    return format->streams[index]->nb_frames;
                }

                boost::rational<int> GetFrameRate() const
                {
                    return boost::rational<int>(format->streams[index]->r_frame_rate.num,format->streams[index]->r_frame_rate.den);
                }

                time::Duration GetStartTime() const
                {
                    return seconds(format->streams[index]->start_time*time_base);
                }
                
//                 virtual time::Duration Seek(time::Duration t)
//                 {
//                     if(format->streams[index]->cur_dts < 0)
//                         return seconds(0);
//                     
//                     std::cerr << "current dts: " << format->streams[index]->cur_dts << " " << format->streams[index]->cur_dts*time_base << std::endl;
//                     int64_t ts = t.asSeconds()/time_base;
//                     std::cerr << "seek to: " << ts << " " << t << std::endl;
//                     
//                     if(ts < 0)
//                         ts = 0;
//                     if(ts >= format->streams[index]->duration)
//                         ts = format->streams[index]->duration-1;
// 
//                     if(avformat_seek_file(format.get(),index,ts,ts,ts,0) < 0)
//                         std::cerr << "gz4d::AVReader::Stream::Seek: error seeking" << std::endl;
//                     
// //                     int flag = 0;//AVSEEK_FLAG_ANY;
// //                     //if(ts < format->streams[index]->cur_dts)
// //                     flag |= AVSEEK_FLAG_BACKWARD;
// // 
// //                     
// //                     if(av_seek_frame(format.get(),index,ts,flag) < 0)
// //                         std::cerr << "gz4d::AVReader::Stream::Seek: error seeking" << std::endl;
// //                    avcodec_flush_buffers(format->streams[index]->codec);
//                     return seconds(ts*time_base);
//                 }
                
            };

            struct AudioStream: public Stream, public Producer<AudioPacket::Ptr >
            {
                //std::vector<uint8_t> buffer;
                std::shared_ptr<int16_t> buffer;
                std::shared_ptr<AVFrame> frame;

                AudioStream(std::shared_ptr<AVFormatContext> &f, int index)
                :Stream(f,index),buffer(reinterpret_cast<int16_t*>(av_malloc((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2)),av_free),frame(avcodec_alloc_frame(),av_free)
                {
                }
                
                void Decode(AVPacket &packet, bool discard=false)
                {
                    Stream::Decode(packet, discard);
                    
                    if(!NoOutputQueue())
                    {
                        while(packet.size > 0)
                        {
                            int frame_size = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
                            
                            //int decoded_size = avcodec_decode_audio2(codec.get(),buffer.get(),&frame_size,packet.data,packet.size);
                            //int decoded_size = avcodec_decode_audio3(codec.get(),buffer.get(),&frame_size,&packet);
                            int decoded_size = avcodec_decode_audio4(codec.get(),frame.get(),&frame_size,&packet);
                            
                            if(decoded_size >= 0)
                            {
                                if(frame_size > 0 && !discard)
                                {
                                    AudioPacket::Ptr data(new AudioPacket);
                                    data->sample_rate = 0;
                                    data->sample_number = 0;
                                    float scale = std::numeric_limits<int16_t>::max();
                                    for(int i = 0; i < frame_size/2; ++i)
                                        data->samples.push_back(buffer.get()[i]/scale);
                                    Send(data);
                                }
                                
                                packet.size -= decoded_size;
                                packet.data += decoded_size;
                            }
                            else
                                break;
                        }
                    }
                }
                
            };

            struct VideoFrame
            {
                time::Duration presentationTime;
                time::Duration decompressionTime;
                time::Duration duration;
                time::Duration convergenceDuration;
                bool isKeyframe;
                ImageYUV8 frame;
            };

            struct VideoStream: public Stream, public Producer<VideoFrame>
            {
                typedef std::shared_ptr<VideoStream> Ptr;
                std::shared_ptr<AVFrame> frame;
                VideoStream(std::shared_ptr<AVFormatContext> &f, int index)
                :Stream(f,index),frame(avcodec_alloc_frame(),av_free)
                {
                }
                
                void Decode(AVPacket &packet, bool discard=false)
                {
                    Stream::Decode(packet, discard);
                    
                    if(!NoOutputQueue())
                    {
                        while(packet.size > 0)
                        {
                            int got_picture;
                            //int len = avcodec_decode_video(codec.get(),frame.get(),&got_picture,packet.data,packet.size);
                            int len = avcodec_decode_video2(codec.get(),frame.get(),&got_picture,&packet);
                            
                            if(len > 0)
                            {
                                if(got_picture && !discard)
                                {
                                    VideoFrame ret;
                                    ret.presentationTime = seconds(packet.pts*time_base);
                                    ret.decompressionTime = seconds(packet.dts*time_base);
                                    ret.duration = seconds(packet.duration*time_base);
                                    ret.convergenceDuration = seconds(packet.convergence_duration*time_base);
                                    ret.isKeyframe = packet.flags && AV_PKT_FLAG_KEY;
                                    ret.frame.Resize(codec->width,codec->height);
                                    //std::cerr << "flags: " << packet.flags << std::endl;
                                    for(int i=0;i<codec->height;i++)
                                    {
                                        //int outi = codec->height-i-1;
                                        Pixel<YUV8>* row = ret.frame.GetRaster().GetRow(i);
                                       
                                        for(int j=0;j<codec->width;j++)
                                        {
                                            row[j][0] = frame->data[0][i*frame->linesize[0]+j];
                                            row[j][1] = frame->data[1][i/2*frame->linesize[1]+j/2];
                                            row[j][2] = frame->data[2][i/2*frame->linesize[2]+j/2];
                                            //ret.frame.GetRaster().GetRow(outi)[j][0] = frame->data[0][i*frame->linesize[0]+j];
                                            //ret.frame.GetRaster().GetRow(outi)[j][1] = frame->data[1][i/2*frame->linesize[1]+j/2];
                                            //ret.frame.GetRaster().GetRow(outi)[j][2] = frame->data[2][i/2*frame->linesize[2]+j/2];
                                        }
                                    }
                                    Send(ret);
                                }
                                packet.size -= len;
                                packet.data += len;
                            }
                            else
                                break;
                        }
                    }
                }
                
                int GetWidth()
                {
                    if(codec)
                        return codec->width;
                    return 0;
                }
                
                int GetHeight()
                {
                    if(codec)
                        return codec->height;
                    return 0;
                }
                
//                 time::Duration Seek(time::Duration t)
//                 {
//                     return t = Stream::Seek(t);
//                 }
            };

            struct SubtitleFrame
            {
                time::Duration presentationTime;
                std::string data;
            };

            struct SubtitleStream: public Stream, public Producer<SubtitleFrame>
            {
                SubtitleStream(std::shared_ptr<AVFormatContext> &f, int index)
                :Stream(f,index)
                {
                }
                
                void Decode(AVPacket &packet)
                {
                    Stream::Decode(packet);
                    if(!NoOutputQueue() && packet.size > 0)
                    {
                        SubtitleFrame ret;
                        ret.presentationTime = seconds(packet.pts*time_base);
                        ret.data = std::string(reinterpret_cast<char *>(packet.data));
                        Send(ret);
                    }
                    
                }
                
            };

            AVReader()
            {
                av_register_all();
            }

            
            bool Open(std::string const &in)
            {
                return Open(in, NULL);
            }
            
            void DumpFormat()
            {
                av_dump_format(format.get(),0,fname.c_str(),false);
            }
            
            bool ReadFrame()
            {
                AVPacket packet;
                av_init_packet(&packet);
                int ret = av_read_frame(format.get(), &packet);
                if(ret == 0)
                {
                    //std::cerr << "ReadFrame: " << packet.dts << std::endl;
                    streams[packet.stream_index]->Decode(packet);
                }
                //av_free_packet(&packet);
                return ret==0;
            }
            
            bool Seek(time::Duration t, time::Duration timeout = seconds(.1), time::Duration lookBack = seconds(2.0))
            {
                Time start_time = time::Now();
                //std::cerr << "cur timestamp: " << format->timestamp << std::endl;
                int64_t ts = t.asSeconds()*AV_TIME_BASE;
                int64_t lb = (t-lookBack).asSeconds()*AV_TIME_BASE;
                //std::cerr << "seek to: " << ts << " " << t << std::endl;
                //std::cerr << "  lookback: " << lb << std::endl;
                
                if(ts < format->start_time)
                    ts = format->start_time;
                if(lb < format->start_time)
                    lb = format->start_time;
                
                if(ts >= format->start_time+format->duration)
                    ts = format->start_time+format->duration-1;
                if(lb >= format->start_time+format->duration)
                    lb = format->start_time+format->duration-1;
                
                //std::cerr << "seek to: (clipped) " << ts << " " << t << std::endl;
                //std::cerr << "  lookback: (clipped) " << lb << std::endl;
                
                if(avformat_seek_file(format.get(),-1,lb,ts,ts,0) < 0)
                    std::cerr << "gz4d::AVReader::Stream::Seek: error seeking" << std::endl;
                
                for(int i = 0; i < format->nb_streams; ++i)
                    avcodec_flush_buffers(format->streams[i]->codec);
                
                AVPacket packet;
                av_init_packet(&packet);
                while(time::Now()-start_time < timeout)
                {
                    int ret = av_read_frame(format.get(), &packet);
                    if(ret == 0)
                    {
                        time::Duration ct = seconds(packet.dts*streams[packet.stream_index]->time_base);
                        time::Duration pd = seconds(packet.duration*streams[packet.stream_index]->time_base);
                        //std::cerr << "packet: " << packet.dts << "\t" << ct << std::endl;
                        if (ct <= t && t <= ct+pd)
                        {
                            streams[packet.stream_index]->Decode(packet);
                            return true;
                        }
                        else
                            streams[packet.stream_index]->Decode(packet,true);
                    }
                }
                return false;
            }
            

            std::vector<std::shared_ptr<AudioStream> > const &GetAudioStreams() const
            {
                return audioStreams;
            }
            
            std::vector<std::shared_ptr<VideoStream> > const &GetVideoStreams() const
            {
                return videoStreams;
            }
            
            std::vector<std::shared_ptr<SubtitleStream> > const &GetSubtitleStreams() const
            {
                return subtitleStreams;
            }
            
        private:
            static void close_input(AVFormatContext * &f)
            {
                avformat_close_input(&f);
            }
            
            bool Open(std::string const &in, AVInputFormat *inf)
            {
                fname = in;
                AVFormatContext *f = NULL;
//                 if(av_open_input_file(&f, in.string().c_str(), inf, 0, fp))
//                     return false;
                if(avformat_open_input(&f,in.c_str(), inf, NULL))
                    return false;
                //format = std::shared_ptr<AVFormatContext>(f,av_close_input_file);
                format = std::shared_ptr<AVFormatContext>(f,close_input);
                if(avformat_find_stream_info(format.get(),nullptr)>=0)
                {
                    for(int i = 0; i < format->nb_streams; ++i)
                    {
                        std::shared_ptr<Stream> s;
                        switch (format->streams[i]->codec->codec_type)
                        {
                            case AVMEDIA_TYPE_AUDIO:
                            {
                                std::shared_ptr<AudioStream> as(new AudioStream(format,i));
                                audioStreams.push_back(as);
                                s = as;
                            }
                            break;
                            case AVMEDIA_TYPE_VIDEO:
                            {
                                std::shared_ptr<VideoStream> vs(new VideoStream(format,i));
                                videoStreams.push_back(vs);
                                s = vs;
                            }
                            break;
                            case AVMEDIA_TYPE_SUBTITLE:
                            {
                                std::shared_ptr<SubtitleStream> ss(new SubtitleStream(format,i));
                                subtitleStreams.push_back(ss);
                                s = ss;
                            }
                            break;
                            default:
                                s = std::shared_ptr<Stream>(new Stream(format,i));
                        }
                        streams.push_back(s);
                    }
                    return true;
                }
                return false;
            }
            
            std::shared_ptr<AVFormatContext> format;
            std::vector<std::shared_ptr<Stream> > streams;
            std::vector<std::shared_ptr<AudioStream> > audioStreams;
            std::vector<std::shared_ptr<VideoStream> > videoStreams;
            std::vector<std::shared_ptr<SubtitleStream> > subtitleStreams;
            std::string fname;

    };
}


#endif
