// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AVWRITER_H
#define GZ4D_AVWRITER_H

#include <gz4d/gz4d.h>
#include <gz4d/av/common.h>
#include <gz4d/Image.h>
#include <gz4d/thread/Consumer.h>
#include <boost/thread/thread.hpp>

#include <map>
#include <string>
#include <gz4d/data/binary/Packet.h>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
}


namespace gz4d
{
    /// \ingroup formats
    class AVWriter
    {
        private:

            struct Parameters
            {
                int width;
                int height;
                int framesPerSecond;
                unsigned int videoBitrate;
                unsigned int audioBitrate;
                unsigned int audioSampleRate;
                unsigned short audioChannels;
                int videoCodec;
                int audioCodec;
                Parameters()
                :width(720),height(480),framesPerSecond(30),videoBitrate(9800000),audioBitrate(192000),audioSampleRate(44100),videoCodec(AV_CODEC_ID_MSMPEG4V2),audioCodec(AV_CODEC_ID_MP2)
                {
                }

            };

            struct AudioFrame
            {
                std::vector<int16_t> data;
                AudioFrame(){}
                AudioFrame(std::vector<int16_t> const &data):data(data){}
            };

            struct Frame
            {
                ImageYUV8 video;
                std::shared_ptr<AudioFrame> audio;
            };

            class FormatContext
            {
                std::shared_ptr<AVFormatContext> context;
                AVStream *audioStream;
                AVStream *videoStream;

                std::shared_ptr<AVFrame> picture;
                std::shared_ptr<AVFrame> audio_frame;

                std::shared_ptr<AVCodecContext> videoCodecCtx;
                std::shared_ptr<AVCodecContext> audioCodecCtx;

                std::vector<uint8_t> pictureData;
                std::vector<int16_t> audioData;

                int64_t videoPts;

                public:
                    typedef std::shared_ptr<FormatContext> Ptr;

                    FormatContext(std::string const &fn, Parameters const &params)
                    :audioStream(nullptr),videoStream(nullptr),videoPts(0)
                    {
                        context = std::shared_ptr<AVFormatContext>(avformat_alloc_context(),avformat_free_context);
                        const AVOutputFormat *format = av_guess_format(NULL,fn.c_str(),NULL);
                        if (!format)
                            format = av_guess_format("mpeg", NULL, NULL);

                        context->oformat = format;

                        av_freep(&context->url);
                        context->url = av_strdup(fn.c_str());

                        if(params.videoCodec != AV_CODEC_ID_NONE)
                        {
                            const AVCodec *codec = avcodec_find_encoder(static_cast<AVCodecID>(params.videoCodec));
                            videoStream = avformat_new_stream(context.get(),codec);

                            videoCodecCtx = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec),[](AVCodecContext *c){ avcodec_free_context(&c); });
                            videoCodecCtx->codec_id = static_cast<AVCodecID>(params.videoCodec);
                            videoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
                            videoCodecCtx->bit_rate = params.videoBitrate;
                            videoCodecCtx->width = params.width;
                            videoCodecCtx->height = params.height;
                            videoCodecCtx->time_base.den = params.framesPerSecond;
                            videoCodecCtx->time_base.num = 1;
                            videoCodecCtx->gop_size = 10;
                            videoCodecCtx->max_b_frames=0;
                            videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
                            std::string formatName = format->name;
                            if(formatName == "mp4" || formatName == "mov" || formatName == "3gp")
                                videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                        }

                        if (params.audioCodec != AV_CODEC_ID_NONE)
                        {
                            const AVCodec *codec = avcodec_find_encoder(static_cast<AVCodecID>(params.audioCodec));
                            audioStream = avformat_new_stream(context.get(), codec);

                            audioCodecCtx = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec),[](AVCodecContext *c){ avcodec_free_context(&c); });
                            audioCodecCtx->codec_id = static_cast<AVCodecID>(params.audioCodec);
                            audioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
                            audioCodecCtx->bit_rate = params.audioBitrate;
                            audioCodecCtx->sample_rate = params.audioSampleRate;
                            AVChannelLayout layout = AV_CHANNEL_LAYOUT_STEREO;
                            if (params.audioChannels == 1)
                                layout = AV_CHANNEL_LAYOUT_MONO;
                            av_channel_layout_copy(&audioCodecCtx->ch_layout, &layout);
                            audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
                        }


                        if (videoStream)
                        {
                            avcodec_open2(videoCodecCtx.get(), videoCodecCtx->codec, nullptr);
                            avcodec_parameters_from_context(videoStream->codecpar, videoCodecCtx.get());
                            videoStream->time_base = videoCodecCtx->time_base;

                            picture = std::shared_ptr<AVFrame>(av_frame_alloc(),[](AVFrame *f){ av_frame_free(&f); });
                            picture->format = videoCodecCtx->pix_fmt;
                            picture->width = videoCodecCtx->width;
                            picture->height = videoCodecCtx->height;
                            av_frame_get_buffer(picture.get(), 0);
                        }

                        if (audioStream)
                        {
                            avcodec_open2(audioCodecCtx.get(), audioCodecCtx->codec, nullptr);
                            avcodec_parameters_from_context(audioStream->codecpar, audioCodecCtx.get());

                            audio_frame = std::shared_ptr<AVFrame>(av_frame_alloc(),[](AVFrame *f){ av_frame_free(&f); });
                            audio_frame->nb_samples = audioCodecCtx->frame_size;
                            audio_frame->format = audioCodecCtx->sample_fmt;
                            av_channel_layout_copy(&audio_frame->ch_layout, &audioCodecCtx->ch_layout);
                            av_frame_get_buffer(audio_frame.get(), 0);

                            int nch = audioCodecCtx->ch_layout.nb_channels;
                            audioData.resize(audioCodecCtx->frame_size * nch);
                        }

                        avio_open(&context->pb, fn.c_str(), AVIO_FLAG_WRITE);
                        avformat_write_header(context.get(),NULL);
                    }

                    ~FormatContext()
                    {
                        if(context)
                        {
                            // Flush video encoder
                            if (videoCodecCtx)
                            {
                                avcodec_send_frame(videoCodecCtx.get(), nullptr);
                                AVPacket *packet = av_packet_alloc();
                                while (avcodec_receive_packet(videoCodecCtx.get(), packet) == 0)
                                {
                                    av_packet_rescale_ts(packet, videoCodecCtx->time_base, videoStream->time_base);
                                    packet->stream_index = videoStream->index;
                                    av_interleaved_write_frame(context.get(), packet);
                                    av_packet_unref(packet);
                                }
                                av_packet_free(&packet);
                            }
                            // Flush audio encoder
                            if (audioCodecCtx)
                            {
                                avcodec_send_frame(audioCodecCtx.get(), nullptr);
                                AVPacket *packet = av_packet_alloc();
                                while (avcodec_receive_packet(audioCodecCtx.get(), packet) == 0)
                                {
                                    av_packet_rescale_ts(packet, audioCodecCtx->time_base, audioStream->time_base);
                                    packet->stream_index = audioStream->index;
                                    av_interleaved_write_frame(context.get(), packet);
                                    av_packet_unref(packet);
                                }
                                av_packet_free(&packet);
                            }
                            av_write_trailer(context.get());
                            avio_close(context->pb);
                            context->pb = NULL;
                        }

                    }


                    void AddFrame(ImageYUV8 const &video)
                    {
                        float wratio = video.width()/float(videoCodecCtx->width);
                        float hratio = video.height()/float(videoCodecCtx->height);

                        av_frame_make_writable(picture.get());

                        // convert colorspace
                        for(int i=0;i<videoCodecCtx->height;i++)
                            for(int j=0;j<videoCodecCtx->width;j++)
                            {
                                ImageYUV8::pixel_type p = video.GetRaster().GetRow(video.height()-1-(int(i*hratio)))[int(j*wratio)];
                                picture->data[0][i*picture->linesize[0]+j] = p[0];
                                if (!(i&0x01 || j&0x01))
                                {
                                    picture->data[1][i/2*picture->linesize[1]+j/2] = 0;
                                    picture->data[2][i/2*picture->linesize[2]+j/2] = 0;
                                }
                                picture->data[1][i/2*picture->linesize[1]+j/2] += p[1]/4;
                                picture->data[2][i/2*picture->linesize[2]+j/2] += p[2]/4;
                            }

                        picture->pts = videoPts++;

                        avcodec_send_frame(videoCodecCtx.get(), picture.get());
                        AVPacket *packet = av_packet_alloc();
                        while (avcodec_receive_packet(videoCodecCtx.get(), packet) == 0)
                        {
                            av_packet_rescale_ts(packet, videoCodecCtx->time_base, videoStream->time_base);
                            packet->stream_index = videoStream->index;
                            av_interleaved_write_frame(context.get(), packet);
                            av_packet_unref(packet);
                        }
                        av_packet_free(&packet);
                    }

                    void AddFrame(std::shared_ptr<AudioFrame> &audio)
                    {
                        av_frame_make_writable(audio_frame.get());

                        int nch = audioCodecCtx->ch_layout.nb_channels;
                        int samples = std::min((int)audio->data.size(), audioCodecCtx->frame_size * nch);
                        int16_t *dst = reinterpret_cast<int16_t*>(audio_frame->data[0]);
                        for(int i = 0; i < samples; ++i)
                            dst[i] = audio->data[i];

                        avcodec_send_frame(audioCodecCtx.get(), audio_frame.get());
                        AVPacket *packet = av_packet_alloc();
                        while (avcodec_receive_packet(audioCodecCtx.get(), packet) == 0)
                        {
                            av_packet_rescale_ts(packet, audioCodecCtx->time_base, audioStream->time_base);
                            packet->stream_index = audioStream->index;
                            av_interleaved_write_frame(context.get(), packet);
                            av_packet_unref(packet);
                        }
                        av_packet_free(&packet);
                    }

                    unsigned int GetAudioFrameSize() const
                    {
                        if(audioCodecCtx)
                            return audioCodecCtx->frame_size;
                        return 0;
                    }

            };

            class Encoder
            {
                FormatContext::Ptr context;
                public:
                    Encoder(FormatContext::Ptr &c)
                    :context(c)
                    {
                    }

                    void Consume(Frame frame)
                    {
                        if(frame.video)
                        {
                            context->AddFrame(frame.video);

                        }

                        if(frame.audio)
                        {
                            context->AddFrame(frame.audio);
                        }
                    }

            };

        public:
            AVWriter():audioInputFrameSize(0)
            {
            }

            ~AVWriter()
            {
                Close();
            }

            bool Open(std::string const &out, bool withAudio = false)
            {
                Parameters p = parameters;

                if (!withAudio)
                    p.audioCodec = AV_CODEC_ID_NONE;

                FormatContext::Ptr formatContext(new FormatContext(out,p));

                audioInputFrameSize =  formatContext->GetAudioFrameSize();

                std::shared_ptr<Encoder> ep(new Encoder(formatContext));
                encoder = std::shared_ptr<Consumer<Encoder,Frame> >(new Consumer<Encoder,Frame>(ep) );
                frameQueue = encoder->GetInQueue();

                return true;
            }

            bool Close()
            {
                if(encoder)
                {
                    while(!frameQueue->empty())
                        boost::thread::yield();
                    encoder.reset();
                }

                return true;
            }

            void SetSize(int w, int h){parameters.width = w; parameters.height = h;}
            void SetSize(Vector<std::size_t,2> const &s){SetSize(s[0],s[1]);}
            void SetFPS(int f){parameters.framesPerSecond = f;}
            void SetVideoBitrate(unsigned int vbr){parameters.videoBitrate = vbr;}
            void SetAudioBitrate(unsigned int abr){parameters.audioBitrate = abr;}
            void SetAudioSampleRate(unsigned int sr){parameters.audioSampleRate = sr;}
            void SetAudioChannels(unsigned short c){parameters.audioChannels = c;}

            bool AddFrame(ImageYUV8 const &i)
            {
                Frame f;
                f.video = i;
                frameQueue->push(f);
                return true;
            }

            bool AddFrame(binary::Image::Frame const &i)
            {
                Frame f;
                f.video = toImage(i);
                frameQueue->push(f);
                return true;
            }

            bool AddAudio(std::vector<int16_t> const &data)
            {
                Frame f;
                f.audio = std::shared_ptr<AudioFrame>(new AudioFrame(data));
                frameQueue->push(f);
                return true;
            }


            unsigned int GetAudioFrameSize(){return audioInputFrameSize;}

        private:
            typedef std::map<int,const AVCodec*> codecMap;
            typedef std::map<std::string,const AVOutputFormat*> formatMap;

            Parameters parameters;
            unsigned int audioInputFrameSize;

            std::shared_ptr<Consumer<Encoder, Frame> > encoder;
            ThreadedQueue<Frame>::Ptr frameQueue;
    };

}

#endif
