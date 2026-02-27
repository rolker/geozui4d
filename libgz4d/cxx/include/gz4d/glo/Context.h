#ifndef GZ4D_GLO_CONTEXT_H
#define GZ4D_GLO_CONTEXT_H

#include <gz4d/glo.h>
#include <cassert>
#include "Error.h"
#include <vector>
#include "Extension.h"
#include <memory>

namespace gz4d
{
    namespace glo
    {

        class Object;
        
        class Context: protected Extension
        {
        public:
            typedef std::shared_ptr<Context> Ptr;

            class Active
            {
                friend class Context;
                typedef std::shared_ptr<Active> Ptr;
                Context::Ptr context;
                Active(Context::Ptr c):context(c){}

            public:

                class Token
                {
                    friend class Context;
                    std::shared_ptr<Active> active;
                    Token(std::shared_ptr<Active> a):active(a){}
                };

            };

            class ID
            {
                friend class Context;
                friend class Object;
                
                
                typedef std::shared_ptr<ID> Ptr;
                GLuint id;
                //std::weak_ptr<Token> token;
                GLuint (*creator)();
                void (*deleter)(GLuint);

                ID(GLuint (*c)(), void (*d)(GLuint)):id(0),creator(c),deleter(d){}

                
                class Token
                {
                    friend class Object;
                    friend class Texture;
                    friend class Context;
                    
                    std::weak_ptr<ID> id;
                    std::weak_ptr<Context> context;

                    Token(ID::Ptr i, Context::Ptr c):id(i),context(c){}


                    ~Token()
                    {
                        if(!context.expired())
                        {
                            Context::Ptr c(context);
                            if(!c)
                                throw Exception("Invalid context");
                            ID::Ptr i(id);
                            c->deleteID(i);
                        }
                    }

                public:
                    bool isAllocated() const
                    {
                        ID::Ptr i(id);
                        return i->id;
                    }

                    operator GLuint() const
                    {
                        if(context.expired())
                            throw Exception("Invalid context");
                        Context::Ptr c(context);
                        if(!c->isActive())
                            throw Exception("Context not active");
                        ID::Ptr i(id);
                        if(!i->id)
                            i->id = i->creator();
                        return i->id;
                    }

                };

            public:
                ~ID()
                {
                    if(id)
                        deleter(id);
                }
                
            };

            
        private:
            friend class Object;
            
            std::weak_ptr<Context> self;
            std::weak_ptr<Active> active;

            std::vector<ID::Ptr> ids;
            std::vector<ID::Ptr> ids_to_delete;

            bool initialized;
        protected:
            Context():initialized(false) {}
        private:
            void setSelf(Context::Ptr s)
            {
                assert(s.get() == this);
                self = s;
            }

            ID::Token createID(GLuint(*idCreator)(), void (*idDeleter)(GLuint))
            {
                ID::Ptr id(new ID(idCreator,idDeleter));
                ids.push_back(id);
                Context::Ptr c(self);
                return ID::Token(id,c);
            }

            void deleteID(ID::Ptr id)
            {
                if (isActive())
                {
                    for(std::vector< ID::Ptr >::iterator i = ids.begin(); i != ids.end(); ++i)
                        if(*i == id)
                        {
                            ids.erase(i);
                            break;
                        }
                }
                else
                    ids_to_delete.push_back(id);
            }
            
        public:
            Active::Token activate()
            {
                if(!initialized)
                {
                    Extension::Init();
                    initialized = true;
                }

                if(!active.expired())
                {
                    Active::Ptr a(active);
                    return Active::Token(a);
                }
                Ptr s(self);
                Active::Ptr a = Active::Ptr(new Active(s));
                active = a;

                for(std::vector< ID::Ptr >::iterator di = ids_to_delete.begin(); di != ids_to_delete.end(); ++di)
                    for(std::vector< ID::Ptr >::iterator i = ids.begin(); i != ids.end(); ++i)
                        if(*di == *i)
                        {
                            ids.erase(i);
                            break;
                        }
                ids_to_delete.clear();
                
                return Active::Token(a);
            }

            bool isActive()
            {
                return !active.expired();
            }


            static std::shared_ptr<Context> create()
            {
                return init_self(new Context);
            }
        protected:
            template <typename T> static std::shared_ptr<T> init_self(T* c)
            {
                std::shared_ptr<T> ret(c);
                ret->setSelf(ret);
                return ret;
            }

        };

        template <class T> class DerivedContext: public Context
        {
        protected:
            DerivedContext(){}
        public:
            static std::shared_ptr<T> create()
            {
                return init_self(new T);
            }
        };
    }
}

#endif
