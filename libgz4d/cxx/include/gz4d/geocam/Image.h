#ifndef GZ4D_GEOCAM_IMAGE_H
#define GZ4D_GEOCAM_IMAGE_H

#include <gz4d/gz4d.h>
#include <gz4d/time.h>
#include <gz4d/TimeSequence.h>
#include <string>
#include <boost/filesystem.hpp>
#include <gz4d/time/ios.h>

namespace gz4d
{
    namespace geocam
    {
        class Image
        {
        protected:
            std::string _filename;
            Time _timestamp;
            std::string _camera_label;
        public:
            Image(){}
            
            Image(std::string const &fn):_filename(fn)
            {
                boost::filesystem::path ip(_filename);
                std::string fname = ip.filename().string();
                std::string tstamp = fname.substr(0,fname.find('_'));
                _timestamp = boost::lexical_cast<gz4d::Time>(tstamp);
                _camera_label = fname.substr(fname.find('_')+1);
                _camera_label = _camera_label.substr(0,_camera_label.find('.'));
            }
            
            Time getTimestamp() const
            {
                return _timestamp;
            }
            
            std::string const &getFilename() const
            {
                return _filename;
            }
            
            std::string const &getCameraLabel() const
            {
                return _camera_label;
            }
            
        };
        
        class ImageSet
        {
            TimeSequence<Image> _images;
        public:
            typedef std::shared_ptr<ImageSet> Ptr;
            
            void add(std::string const &fn)
            {
                Image i(fn);
                _images[i.getTimestamp()]=i;
            }
            
            void add(std::string const &dir, std::string const &pattern, std::string const &ext=std::string(".jpg"))
            {
                boost::filesystem::path dir_path(dir);
                if(exists(dir_path) && is_directory(dir_path))
                {
                    for(boost::filesystem::directory_iterator e(dir_path); e != boost::filesystem::directory_iterator(); ++e)
                    {
                        if(is_directory(*e))
                            add(boost::filesystem::path(*e).string(),pattern,ext);
                        if(is_regular_file(*e))
                        {
                            boost::filesystem::path f(*e);
                            if(pattern.empty() || f.filename().string().find(pattern) != std::string::npos)
                            {
                                if(ext.empty() || f.extension() == ext)
                                    add(f.string());
                            }
                        }
                    }
                }
                
            }
            
            std::size_t size() const
            {
                return _images.size();
            }
            
            TimeSequence<Image> &getImages()
            {
                return _images;
            }
            
        };
        
        typedef std::map<std::string,ImageSet::Ptr> ImageSets;
        
    }
}

#endif
