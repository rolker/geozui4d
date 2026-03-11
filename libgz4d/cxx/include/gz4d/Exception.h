// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_EXCEPTION_H
#define GZ4D_EXCEPTION_H

#include <gz4d/gz4d.h>
#include <exception>
#include <string>
#include <typeinfo>

namespace gz4d
{
    /// Base for exception originating from libgz4d.
    /// \ingroup base
    class Exception: public std::exception
    {
        public:
            Exception(std::string const &message = "Unknown exception."):message(message){}
            ~Exception() throw(){};
            char const *what() const throw(){return message.c_str();}
        protected:
            std::string message;
    };

    /// Base for extraction-operator exceptions originating from libgz4d.
    /// \ingroup base
    class ExtractionException: public Exception
    {
        public:
            ExtractionException(int numItemsExtracted = 0, std::string message = "Extraction exception."):Exception(message),items(numItemsExtracted){}
            ~ExtractionException() throw() {};
            int itemsExtracted() const throw() {return items;}
        protected:
            int items;
    };

    template <typename T> class TypedException: public Exception
    {
        public:
            TypedException():Exception(std::string(typeid(T).name())+": unspecified exception."){};
            TypedException(std::string const &message):Exception(message){};
    };
}

#endif
