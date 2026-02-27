#ifndef GZ4D_GLO_UNIFORM_H
#define GZ4D_GLO_UNIFORM_H

#include <gz4d/glo.h>
#include <vector>
#include <gz4d/Vector.h>

namespace gz4d
{
    namespace glo
    {
        class Program;
        
        class Uniform
        {
            friend class Program;
            
            int location;
            Uniform(int location):location(location){}
        public:
            Uniform():location(-1){}
            
            struct Info
            {
                int size;
                GLenum type;
                std::string name;
                Info():size(0),type(0){}
            };
            
            void operator()(float v) const
            {
                glUniform1f(location,v);
            }

            void operator()(double v) const
            {
                operator()(float(v));
            }

            void operator()(int v) const
            {
                glUniform1i(location,v);
            }

            void operator()(std::vector<float> const &v) const
            {
                glUniform1fv(location,v.size(),&v.front());
            }
            
            void operator()(std::vector<double> const &v) const
            {
                std::vector<float> fv(v.begin(),v.end());
                operator()(fv);
            }

            void operator()(Vector<float,2> const &v) const
            {
                glUniform2f(location,v[0],v[1]);
            }

            operator bool()
            {
                return location != -1;
            }
        };
    }
}


#endif
