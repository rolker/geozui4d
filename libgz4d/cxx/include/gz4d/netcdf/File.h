#ifndef GZ4D_NETCDF_FILE_H
#define GZ4D_NETCDF_FILE_H

#include <gz4d/gz4d.h>
#include <gz4d/netcdf/netcdf.h>
#include <gz4d/netcdf/Var.h>

namespace gz4d
{
    namespace netcdf
    {
        class File
        {
            public:
                File(){}
                File(File const &f):ncfile(f.ncfile){}
                File(std::string const &fn){OpenReadOnly(fn);}

                void OpenReadOnly(std::string const &fn)
                {
                    try { ncfile = std::make_shared<netCDF::NcFile>(fn, netCDF::NcFile::read); }
                    catch (netCDF::exceptions::NcException&) { ncfile.reset(); }
                }
                void OpenWrite(std::string const &fn)
                {
                    try { ncfile = std::make_shared<netCDF::NcFile>(fn, netCDF::NcFile::write); }
                    catch (netCDF::exceptions::NcException&) { ncfile.reset(); }
                }
                void OpenReplace(std::string const &fn)
                {
                    try { ncfile = std::make_shared<netCDF::NcFile>(fn, netCDF::NcFile::replace); }
                    catch (netCDF::exceptions::NcException&) { ncfile.reset(); }
                }
                void OpenNew(std::string const &fn)
                {
                    try { ncfile = std::make_shared<netCDF::NcFile>(fn, netCDF::NcFile::newFile); }
                    catch (netCDF::exceptions::NcException&) { ncfile.reset(); }
                }

                void Close(){ncfile.reset();}

                operator bool() {return ncfile && !ncfile->isNull();}

                int NumDims(){CheckValid(); return ncfile->getDimCount();}
                Dim GetDim(std::string const &name)
                {
                    CheckValid();
                    return Dim(ncfile->getDim(name));
                }
                Dim GetDim(int n)
                {
                    CheckValid();
                    std::multimap<std::string, netCDF::NcDim> dims = ncfile->getDims();
                    int idx = 0;
                    for (auto it = dims.begin(); it != dims.end(); ++it, ++idx)
                    {
                        if (idx == n) return Dim(it->second);
                    }
                    throw Exception("Dimension index out of range");
                }
                Dim GetUnlimitedDim()
                {
                    CheckValid();
                    std::multimap<std::string, netCDF::NcDim> dims = ncfile->getDims();
                    for (auto it = dims.begin(); it != dims.end(); ++it)
                    {
                        if (it->second.isUnlimited()) return Dim(it->second);
                    }
                    throw Exception("No unlimited dimension found");
                }
                Dim AddDim(std::string const &name)
                {
                    CheckValid();
                    return Dim(ncfile->addDim(name));
                }
                Dim AddDim(std::string const &name, long size)
                {
                    CheckValid();
                    return Dim(ncfile->addDim(name, size));
                }

                int NumVars(){CheckValid(); return ncfile->getVarCount();}
                Var GetVar(std::string const &name)
                {
                    CheckValid();
                    return Var(ncfile->getVar(name));
                }
                Var GetVar(int n)
                {
                    CheckValid();
                    std::multimap<std::string, netCDF::NcVar> vars = ncfile->getVars();
                    int idx = 0;
                    for (auto it = vars.begin(); it != vars.end(); ++it, ++idx)
                    {
                        if (idx == n) return Var(it->second);
                    }
                    throw Exception("Variable index out of range");
                }

                template <typename T> Var AddVar(std::string const &name, std::vector<Dim> const &dims, T dummy=0);

                int NumAtts(){CheckValid(); return ncfile->getAttCount();}
                Att GetAtt(std::string const &name)
                {
                    CheckValid();
                    return Att(ncfile->getAtt(name));
                }
                Att GetAtt(int n)
                {
                    CheckValid();
                    std::multimap<std::string, netCDF::NcGroupAtt> atts = ncfile->getAtts();
                    int idx = 0;
                    for (auto it = atts.begin(); it != atts.end(); ++it, ++idx)
                    {
                        if (idx == n) return Att(it->second);
                    }
                    throw Exception("Attribute index out of range");
                }

                template <typename T> void AddAtt(std::string const &name, T value)
                {
                    CheckValid();
                    ncfile->putAtt(name, getType<T>(), value);
                }

                void Sync() {CheckValid(); ncfile->sync();}
                void Abort() {ncfile.reset();}

            private:
                std::shared_ptr<netCDF::NcFile> ncfile;
                bool IsValid(){return ncfile && !ncfile->isNull();}
                void CheckValid(){if (!IsValid()) throw Exception("Invalid File");}

                template <typename T> static netCDF::NcType getType();
        };

        template<> inline netCDF::NcType File::getType<int8_t>() {return netCDF::ncByte;}
        template<> inline netCDF::NcType File::getType<char>() {return netCDF::ncChar;}
        template<> inline netCDF::NcType File::getType<int16_t>() {return netCDF::ncShort;}
        template<> inline netCDF::NcType File::getType<int32_t>() {return netCDF::ncInt;}
        template<> inline netCDF::NcType File::getType<float>() {return netCDF::ncFloat;}
        template<> inline netCDF::NcType File::getType<double>() {return netCDF::ncDouble;}
    }
}

#endif
