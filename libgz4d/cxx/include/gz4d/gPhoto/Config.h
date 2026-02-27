#ifndef GZ4D_GPHOTO_CONFIG
#define GZ4D_GPHOTO_CONFIG

#include <gz4d/gz4d.h>
#include <memory>
#include <map>
#include <gphoto2.h>
#include <gz4d/util/Utilities.h>

//gz4d_cmake depends gPhoto2

struct _CameraWidget;

namespace gz4d
{
    namespace gphoto
    {
        class Camera;

        class Config
        {
            friend class Camera;

            std::shared_ptr<_CameraWidget> cameraWidget;

            std::string name;
            std::string label;
            std::string info;

            std::string type;
            std::string dataType;

            float min,max,increment;
            std::vector<std::string> choices;

            bool readOnly;

            std::map<std::string, Config> children;

            Config(std::shared_ptr<_CameraWidget> cw)
            :cameraWidget(cw),min(gz4d::Nan<float>()),max(gz4d::Nan<float>()),increment(gz4d::Nan<float>()),readOnly(true)
            {
                char const * str;
                if(gp_widget_get_name(cameraWidget.get(), &str) >= 0)
                    name = str;
                if(gp_widget_get_label(cameraWidget.get(), &str) >= 0)
                    label = str;
                if(gp_widget_get_info(cameraWidget.get(), &str) >= 0)
                    info = str;
                
                int ro;
                if(gp_widget_get_readonly(cameraWidget.get(), &ro) >= 0)
                    readOnly = ro;
                
                CameraWidgetType t;
                if(gp_widget_get_type(cameraWidget.get(), &t) >= 0)
                {
                    switch(t)
                    {
                        case GP_WIDGET_WINDOW:
                        case GP_WIDGET_SECTION:
                            type = "container";
                            break;
                        case GP_WIDGET_TEXT:
                            type = "text";
                            dataType = "string";
                            break;
                        case GP_WIDGET_RANGE:
                            type = "range";
                            dataType = "float";
                            gp_widget_get_range(cameraWidget.get(),&min,&max,&increment);
                            break;
                        case GP_WIDGET_TOGGLE:
                            type = "bool";
                            dataType = "bool";
                            break;
                        case GP_WIDGET_RADIO:
                        case GP_WIDGET_MENU:
                            type = "choice";
                            dataType = "string";
                            for(int i = 0; i < gp_widget_count_choices(cameraWidget.get()); ++i)
                            {
                                char const *choice;
                                if(gp_widget_get_choice(cameraWidget.get(), i, &choice) >= 0)
                                    choices.push_back(choice);
                            }
                            break;
                        case GP_WIDGET_BUTTON:
                            type = "button";
                            break;
                        case GP_WIDGET_DATE:
                            type = "date";
                            break;
                        default:
                            type = "unknown";
                    }
                }
                
                int cc = gp_widget_count_children(cameraWidget.get());
                for(int i = 0; i < cc; ++i)
                {
                    CameraWidget *cw;
                    if(gp_widget_get_child(cameraWidget.get(), i, &cw) >= 0)
                    {
                        //std::shared_ptr<CameraWidget> cwsp(cw,gp_widget_unref);
                        std::shared_ptr<CameraWidget> cwsp(cw,NullDeleter());
                        Config cfg(cwsp);
                        children[cfg.GetName()] = cfg;
                    }
                }
            }
            
            public:
                typedef std::map<std::string, Config>::iterator iterator;

                Config()
                :min(gz4d::Nan<float>()),max(gz4d::Nan<float>()),increment(gz4d::Nan<float>()),readOnly(true)
                {
                }
                
                Config(Config const & config)
                :cameraWidget(config.cameraWidget),name(config.name),label(config.label),info(config.info),type(config.type),dataType(config.dataType),min(config.min),max(config.max),increment(config.increment),choices(config.choices),readOnly(config.readOnly),children(config.children)
                {
                }
                
                operator bool() const
                {
                    return bool(cameraWidget);
                }
                

                std::string GetName() const
                {
                    return name;
                }
                
                std::string GetLabel() const
                {
                    return label;
                }
                
                std::string GetInfo() const
                {
                    return info;
                }
                

                std::string GetType() const
                {
                    return type;
                }
                
                std::string GetDataType() const
                {
                    return dataType;
                }
                

                float GetMin() const
                {
                    return min;
                }
                
                float GetMax() const
                {
                    return max;
                }
                
                float GetIncrement() const
                {
                    return increment;
                }
                

                std::vector<std::string> const &GetChoices() const
                {
                    return choices;
                }
                

                bool IsReadOnly() const
                {
                    return readOnly;
                }
                

                Config &operator[](std::string const& n)
                {
                    return children[n];
                }
                

                iterator begin()
                {
                    return children.begin();
                }
                
                iterator end()
                {
                    return children.end();
                }
                

                bool Set(std::string const& value)
                {
                    if(cameraWidget && dataType == "string" && gp_widget_set_value(cameraWidget.get(), value.c_str()) >= 0)
                    {
                        int c = 1;
                        gp_widget_set_changed(cameraWidget.get(),c);
                        return true;
                    }
                    return false;
                }
                
                std::string Get()
                {
                    char const * str;
                    if(cameraWidget && dataType == "string" && gp_widget_get_value(cameraWidget.get(), &str) >= 0 && str)
                        return str;
                    return std::string();
                }
                

                bool SetFloat(float value)
                {
                    if(cameraWidget && dataType == "float" && gp_widget_set_value(cameraWidget.get(), &value) >= 0)
                    {
                        int c = 1;
                        gp_widget_set_changed(cameraWidget.get(),c);
                        return true;
                    }
                    return false;
                }
                
                float GetFloat()
                {
                    float val;
                    if(cameraWidget && dataType == "float" && gp_widget_get_value(cameraWidget.get(), &val) >= 0)
                        return val;
                    return Nan<float>();
                }
                

                bool SetBool(bool b)
                {
                    if(cameraWidget && dataType == "bool" && gp_widget_set_value(cameraWidget.get(), &b) >= 0)
                    {
                        int c = 1;
                        gp_widget_set_changed(cameraWidget.get(),c);
                        return true;
                    }
                    return false;
                }
                
                bool GetBool()
                {
                    int val;
                    if(cameraWidget && dataType == "bool" && gp_widget_get_value(cameraWidget.get(), &val) >= 0)
                        return val;
                    return false;
                }
        };
    }
}

#endif
