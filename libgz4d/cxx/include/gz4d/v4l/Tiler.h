#ifndef GZ4D_V4L_TILER_H
#define GZ4D_V4L_TILER_H

#include <gz4d/gz4d.h>
#include <gz4d/Image.h>
#include "Stream.h"
#include <map>
#include <memory>
#include <boost/foreach.hpp>
#include <gz4d/glo/ScaleImage.h>
#include <gz4d/time.h>
#include <gz4d/time/ios.h>

namespace gz4d
{
    namespace v4l
    {

        /// Assembles multiple video streams into a single tiled stream.
        class Tiler
        {
            struct StreamData
            {
                std::shared_ptr<Stream> stream;
                Stream::output_queue_type queue;
                binary::Wrapper<binary::Image::Frame> next_frame;
                binary::Wrapper<binary::Image::Frame> last_frame;
				StreamData const &operator=(const StreamData &o){stream=o.stream; queue=o.queue; next_frame=o.next_frame; last_frame=o.last_frame; return *this;}
            };

            typedef std::map<std::size_t, StreamData> RowType;
            typedef std::map<std::size_t,RowType> GridType;
            GridType _streams;

            /// Total requested dimensions of the output stream.
            /// Set to zero to automatically calculate from input stream dimensions.
            std::size_t _fixedWidth,_fixedHeight;

            /// Actual output dimensions.
            std::size_t _width,_height;

            typedef std::vector<std::size_t> DimSizesType;
            DimSizesType _rowSizes,_colSizes;

            std::size_t _innerBorder;
            std::size_t _outerBorder;
            Color _backgroundColor;

            Time _lastFrameTime;
            time::Duration _framePeriod;

            inline void calculateWidths()
            {
                _colSizes.clear();
                for(GridType::const_iterator r = _streams.begin(); r != _streams.end(); ++r)
                {
                    if(!r->second.empty() && r->second.rbegin()->first >= _colSizes.size())
                        _colSizes.resize(r->second.rbegin()->first+1,0);
                    for(RowType::const_iterator c = r->second.begin(); c != r->second.end(); ++c)
                        if(c->second.stream)
                            _colSizes[c->first] = std::max(_colSizes[c->first],c->second.stream->width());
                }
                _width = 2*_outerBorder;
                if(!_colSizes.empty())
                    _width += (_colSizes.size()-1)*_innerBorder;
                BOOST_FOREACH(std::size_t w, _colSizes)
                {
                    _width += w;
                }
                if(_fixedWidth != 0)
                {
                    if(_width > 0)
                    {
                        double scale = _fixedWidth/double(_width);
                        for(DimSizesType::iterator s = _colSizes.begin(); s != _colSizes.end(); ++s)
                            *s*=scale;
                    }
                    _width = _fixedWidth;
                }
            }

            inline void calculateHeights()
            {
                _rowSizes.clear();
                for(GridType::const_iterator r = _streams.begin(); r != _streams.end(); ++r)
                {
                    _rowSizes.push_back(0);
                    for(RowType::const_iterator c = r->second.begin(); c != r->second.end(); ++c)
                        if(c->second.stream)
                            _rowSizes.back() = std::max(_rowSizes.back(),c->second.stream->height());
                }
                _height = 2*_outerBorder;
                if(!_rowSizes.empty())
                    _height += (_rowSizes.size()-1)*_innerBorder;
                BOOST_FOREACH(std::size_t h, _rowSizes)
                {
                    _height += h;
                }
                if(_fixedHeight != 0)
                {
                    if(_height > 0)
                    {
                        double scale = _fixedHeight/double(_height);
                        for(DimSizesType::iterator s = _rowSizes.begin(); s != _rowSizes.end(); ++s)
                            *s*=scale;
                    }
                    _height = _fixedHeight;
                }
            }

            inline void calculate(){ calculateWidths(); calculateHeights();}

            public:
                Tiler():_fixedWidth(0),_fixedHeight(0),_width(0),_height(0),_innerBorder(0),_outerBorder(0),_backgroundColor(Blue()),_framePeriod(1/30.0){}

                inline void add(std::shared_ptr<Stream> stream, std::size_t row, std::size_t col)
                {
                    StreamData sd;
                    sd.stream = stream;
                    sd.queue = stream->NewOutQueue(120);
                    sd.last_frame = sd.queue->pop();
                    sd.next_frame = sd.queue->pop();

                    _streams[row][col] = sd;
                    calculate();
                }

                inline std::shared_ptr<Stream> get(std::size_t row, std::size_t col) const
                {
                    GridType::const_iterator r = _streams.find(row);
                    if(r != _streams.end())
                    {
                        RowType::const_iterator c = r->second.find(col);
                        if(c != r->second.end())
                            return c->second.stream;
                    }
                    return std::shared_ptr<Stream>();
                }

                inline void remove(std::size_t row, std::size_t col)
                {
                    if(_streams.find(row) != _streams.end())
                    {
                        _streams[row].erase(col);
                        if(_streams[row].empty())
                            _streams.erase(row);
                        calculate();
                    }
                }

                std::size_t fixedWidth() const {return _fixedWidth;}
                std::size_t width(std::size_t w) {_fixedWidth = w; calculateWidths(); return _width;}
                std::size_t width() const {return _width;}

                std::size_t fixedHeight() const {return _fixedHeight;}
                std::size_t height(std::size_t h) {_fixedHeight = h; calculateHeights(); return _height;}
                std::size_t height() const {return _height;}

                std::size_t rows() const {return _rowSizes.size();}
                std::size_t columns() const {return _colSizes.size();}
                bool empty() const {return _streams.empty();}

                std::size_t innerBorder() const {return _innerBorder;}
                void innerBorder(std::size_t s) {_innerBorder = s; calculate();}

                std::size_t outerBorder() const {return _outerBorder;}
                void outerBorder(std::size_t s) {_outerBorder = s; calculate();}

                Color const &backgroundColor() const {return _backgroundColor;}
                Color &backgroundColor(){ return _backgroundColor;}
                Color const &backgroundColor(Color const &c) {_backgroundColor = c; return _backgroundColor;}

                void framePeriod(time::Duration fp) {_framePeriod = fp;}

                binary::Wrapper<binary::Image::Frame> Read()
                {
                    if(!_lastFrameTime.valid())
                    {
                        for(GridType::const_iterator r = _streams.begin(); r != _streams.end(); ++r)
                        {
                            for(RowType::const_iterator c = r->second.begin(); c != r->second.end(); ++c)
                            {
                                if(!_lastFrameTime.valid() || c->second.next_frame->timestamp > _lastFrameTime)
                                    _lastFrameTime = c->second.next_frame->timestamp;
                            }
                        }
                    }

                    binary::Wrapper<binary::Image::Frame> frame;
                    frame->dataSize = _width*_height*2;
                    frame->timestamp = _lastFrameTime;

                    std::size_t dr = _outerBorder;
                    std::size_t last_row = 0;
                    for(GridType::iterator r = _streams.begin(); r != _streams.end(); ++r)
                    {
                        for(; last_row < r->first; dr += _rowSizes[last_row]+_innerBorder, ++last_row){}
                        std::size_t dc = _outerBorder;
                        std::size_t last_col = 0;
                        for(RowType::iterator c = r->second.begin(); c != r->second.end(); ++c)
                        {
                            for(; last_col < c->first; dc += _colSizes[last_col]+_innerBorder, ++last_col){}
                            if(c->second.stream)
                            {
                                while(c->second.next_frame->timestamp < _lastFrameTime)
                                {
                                    c->second.last_frame = c->second.next_frame;
                                    c->second.next_frame = c->second.queue->pop();
                                }
                                binary::Wrapper<binary::Image::Frame> f = c->second.last_frame;

                                if(f->dataSize>0)
                                {
                                    if(f->width == _colSizes[c->first] && f->height == _rowSizes[r->first])
                                    {
                                        //ret->subRaster(dc,dr,f.image.GetRaster());
                                    }
                                    else
                                    {
//                                         ImageYUV8::raster_type::Ptr ir = f.image.GetRasterPtr();
//                                         double scale = std::min(_colSizes[c->first]/double(ir->GetColumns()),_rowSizes[r->first]/double(ir->GetRows()));
//                                         //ImageYUV8::raster_type::Ptr sir = glo::ScaleImage(ir,ir->GetColumns()*scale,ir->GetRows()*scale);
//                                         //ImageYUV8::raster_type::Ptr sir(new ImageYUV8::raster_type(ir->GetColumns()*scale,ir->GetRows()*scale,*ir));
//                                         ImageYUV8 si(ir->GetColumns()*scale,ir->GetRows()*scale,f.image);
//                                         ret->subRaster(dc+(_colSizes[c->first]-si.GetRaster().GetColumns())/2,dr+(_rowSizes[r->first]-si.GetRaster().GetRows())/2,si.GetRaster());
                                    }
                                }
                            }
                        }
                    }
                    _lastFrameTime += _framePeriod;
                    return frame;
                }
        };
    }
}

#endif
