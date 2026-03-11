// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TEXTTOSPEECH_H
#define GZ4D_TEXTTOSPEECH_H

//gz4d_cmake depends festival

#include <gz4d/gz4d.h>
#include <string>
#include <festival.h>
#include <sstream>

#define IMPLEMENT_GZ4D_TEXTTOSPEECH bool gz4d::TextToSpeech::festival_initialized = false;

namespace gz4d
{
    class TextToSpeech
    {
        static bool festival_initialized;
        
        public:
            TextToSpeech()
            {
                if(!festival_initialized)
                {
                    festival_initialize(1,FESTIVAL_HEAP_SIZE);
                    festival_initialized = true;
                    EST_String cmd("(set! gz_base_f0_mean (cadr (assoc 'target_f0_mean int_lr_params)))");
                    festival_eval_command(cmd);
                    cmd = "(set! gz_base_f0_std (cadr (assoc 'target_f0_std int_lr_params)))";
                    festival_eval_command(cmd);
                }
            }
            
            bool say(std::string const &text) const
            {
                EST_String t(text.c_str());
                bool ret = festival_say_text(t);
                festival_wait_for_spooler();
                return ret;
            }
            
            bool setVoice(std::string const &voice) const
            {
                std::stringstream cmd;
                cmd << "(" << voice << ")";
                EST_String t(cmd.str().c_str());
                if(festival_eval_command(t))
                {
                    EST_String scmd("(set! gz_base_f0_mean (cadr (assoc 'target_f0_mean int_lr_params)))");
                    if(festival_eval_command(scmd))
                    {
                        scmd = "(set! gz_base_f0_std (cadr (assoc 'target_f0_std int_lr_params)))";
                        return festival_eval_command(scmd);
                    }
                }
                return false;
            }
            
            bool frequency(float mean, float std = 1.0) const
            {
                std::stringstream cmd;
                cmd << "(set! int_lr_params (list ( list 'target_f0_mean (* gz_base_f0_mean " << mean << ")) (list 'target_f0_std (* gz_base_f0_std " << std << ")) (list 'model_f0_mean (cadr (assoc 'model_f0_mean  int_lr_params))) (list 'model_f0_std (cadr (assoc 'model_f0_std int_lr_params)))))";
                EST_String t(cmd.str().c_str());
                return festival_eval_command(t);
            }
            
            bool rate(float r) const
            {
                std::stringstream cmd;
                cmd << "(Parameter.set 'Duration_Stretch " << r << ")";
                EST_String t(cmd.str().c_str());
                return festival_eval_command(t);
            }
            
    };
}

#endif
