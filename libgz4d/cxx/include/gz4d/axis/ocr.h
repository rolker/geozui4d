// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AXIS_OCR_H
#define GZ4D_AXIS_OCR_H

#include <gz4d/gz4d.h>
#include <gz4d/Image.h>

namespace gz4d
{
    namespace axis
    {
        class OCR
        {
            class Character
            {
                gz4d::ImageL8 _image;
            public:
                Character(){}
                Character(const char* pattern[9])
                :_image(std::string(pattern[0]).size(),9,ImageL8::pixel_type(Black()))
                {
                    for(int r = 0; r < 9; ++r)
                    {
                        Pixel< L8 >* row = _image.GetRaster().GetRow(8-r);
                        for(int c = 0; c < _image.width(); ++c)
                            if (pattern[r][c] != ' ')
                                row[c][0] = 255;
                            else
                                row[c][0] = 128;
                    }
                }
                
                double score(ImageL8 const &img, int start_col, int start_row)
                {
                    double bg_total = 0.0;
                    int bg_count = 0;
                    for(int r = 0; r < _image.height() && r+start_row < img.height(); ++r)
                    {
                        const Pixel< L8 >* img_row = img.GetRaster().GetRow(r+start_row);
                        const Pixel< L8 >* char_row = _image.GetRaster().GetRow(r);
                        for(int c = 0; c < _image.width() && c+start_col < img.width(); ++c)
                        {
                            if(char_row[c][0] != 255)
                            {
                                bg_total += img_row[c+start_col][0]/255.0;
                                ++bg_count ;
                            }
                        }
                    }
                    double bg_avg = bg_total/double(bg_count);

                    double ret = 0.0;
                    for(int r = 0; r < _image.height() && r+start_row < img.height(); ++r)
                    {
                        const Pixel< L8 >* img_row = img.GetRaster().GetRow(r+start_row);
                        const Pixel< L8 >* char_row = _image.GetRaster().GetRow(r);
                        //std::cout << "\t";
                        for(int c = 0; c < _image.width() && c+start_col < img.width(); ++c)
                        {
                            //std::cout << int(char_row[c][0]) << "-" << int(img_row[c+start_col][0]) << "\t";
                            if(char_row[c][0] == 255)
                                ret += img_row[c+start_col][0]/255.0;
                            else
                                ret += 1.0-fabs(img_row[c+start_col][0]/255.0-bg_avg);
                        }
                        //std::cout << std::endl;
                    }
                    return ret/double(_image.width()*_image.height());
                }
                
                std::size_t width() const
                {
                    return _image.width();
                }
            };
            
            std::map<char, Character> _characters;
            
        public:
            OCR();
            
            std::string operator()(ImageL8 const &img, int start_col=4, int start_row=-15)
            {
                int row = start_row;
                if (row < 0)
                    row += img.height();
                int col = start_col;
                if (col < 0)
                    col += img.width();
                
                std::string ret;

                while(true)
                {
                    char best = '?';
                    double best_score = -1;
                    
                    for(std::map< char, Character >::iterator c = _characters.begin(); c != _characters.end(); ++c)
                    {
                        double score = c->second.score(img,col,row);
                        //std::cout << "\t" << "'" << c->first << "'\t" << score << std::endl;
                        if(score > best_score)
                        {
                            best = c->first;
                            best_score = score;
                        }
                    }
                    //std::cout << best << "\t" << best_score << std::endl;
                    if(best == ' ' && !ret.empty() && ret.back() == ' ')
                    {
                        ret = ret.substr(0,ret.size()-1); // strip last space
                        break; // consider two spaces the end of the string
                    }
                    ret += best;
                    col += _characters[best].width();
                    if(col >= img.width())
                        break;
                }
                
                return ret;
            }
        };
        
        inline std::string ocr(ImageL8 const &img)
        {
            std::string ret;
            std::size_t space = 0;

            std::vector<uint32_t> c;
            
            for (std::size_t col = 0; col < img.width(); ++col)
            {
                uint32_t val = 0;
                for(std::size_t r = img.height()-1; r > img.height()-25; --r)
                {
                    val *=2;
                    if (img.GetRaster().GetRow(r)[col][0] > 175)
                        val += 1;
                    //std::cerr << (int)(img.GetRaster().GetRow(r)[col][0]) << ",";
                }
//                 std::cerr << "\nval: " << val << std::endl;
                if(val > 0)
                {
                    if(c.empty())
                    {
                        if(space >= 5)
                            ret += " ";
                        space = 0;
                    }
                    c.push_back(val);
                }
                else
                {
                    if(!c.empty())
                    {
                        bool found = false;
                        if (!found && c.size() == 6 && c[0] == 19456 && c[1] == 37376 && c[2] == 37376 && c[3] == 37376 && c[4] == 31744 && c[5] == 512 ){ret += "a"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 31744 && c[1] == 33280 && c[2] == 33280 && c[3] == 33280 && c[4] == 17408 ){ret += "c"; found = true;}
                        if (!found && c.size() == 1 && c[0] == 196096 ){ret += "i"; found = true;}
                        if (!found && c.size() == 1 && c[0] == 261632 ){ret += "l"; found = true;}
                        if (!found && c.size() == 7 && c[0] == 65024 && c[1] == 16384 && c[2] == 32768 && c[3] == 32256 && c[4] == 16384 && c[5] == 32768 && c[6] == 32256 ){ret += "m"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 65024 && c[1] == 16384 && c[2] == 32768 && c[3] == 32768 && c[4] == 32256 ){ret += "n"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 31744 && c[1] == 33280 && c[2] == 33280 && c[3] == 33280 && c[4] == 31744 ){ret += "o"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 65472 && c[1] == 17408 && c[2] == 33280 && c[3] == 33280 && c[4] == 31744 ){ret += "p"; found = true;}
                        if (!found && c.size() == 4 && c[0] == 25600 && c[1] == 37376 && c[2] == 37376 && c[3] == 19456 ){ret += "s"; found = true;}
                        if (!found && c.size() == 3 && c[0] == 32768 && c[1] == 261632 && c[2] == 33280 ){ret += "t"; found = true;}
                        if (!found && c.size() == 4 && c[0] == 34304 && c[1] == 39424 && c[2] == 41472 && c[3] == 49664 ){ret += "z"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 130048 && c[1] == 131584 && c[2] == 131584 && c[3] == 131584 && c[4] == 130048 ){ret += "0"; found = true;}
                        if (!found && c.size() == 3 && c[0] == 65536 && c[1] == 65536 && c[2] == 261632 ){ret += "1"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 69120 && c[1] == 135680 && c[2] == 139776 && c[3] == 147968 && c[4] == 98816 ){ret += "2"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 68608 && c[1] == 131584 && c[2] == 147968 && c[3] == 147968 && c[4] == 113664 ){ret += "3"; found = true;}
                        if (!found && c.size() == 6 && c[0] == 6144 && c[1] == 10240 && c[2] == 51200 && c[3] == 67584 && c[4] == 261632 && c[5] == 2048 ){ret += "4"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 248832 && c[1] == 147968 && c[2] == 147968 && c[3] == 147968 && c[4] == 146432 ){ret += "5"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 130048 && c[1] == 139776 && c[2] == 147968 && c[3] == 147968 && c[4] == 80896 ){ret += "6"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 131072 && c[1] == 132608 && c[2] == 145408 && c[3] == 180224 && c[4] == 196608 ){ret += "7"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 113664 && c[1] == 147968 && c[2] == 147968 && c[3] == 147968 && c[4] == 113664 ){ret += "8"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 115712 && c[1] == 139776 && c[2] == 139776 && c[3] == 139776 && c[4] == 130048 ){ret += "9"; found = true;}
                        if (!found && c.size() == 1 && c[0] == 512 ){ret += "."; found = true;}
                        if (!found && c.size() == 1 && c[0] == 16896 ){ret += ":"; found = true;}
                        if (!found && c.size() == 5 && c[0] == 4096 && c[1] == 4096 && c[2] == 31744 && c[3] == 4096 && c[4] == 4096 ){ret += "+"; found = true;}
                        if (!found && c.size() == 4 && c[0] == 4096 && c[1] == 4096 && c[2] == 4096 && c[3] == 4096 ){ret += "-"; found = true;}
                        
                        if (!found)
                            return ret;
                        c.clear();
                    }
                    space += 1;
                }
            }
            return ret;
        };
    }
}


inline gz4d::axis::OCR::OCR()
{
    const char * c_spc[] = {    "    ",
                                "    ",
                                "    ",
                                "    ",
                                "    ",
                                "    ",
                                "    ",
                                "    ",
                                "    "};
    _characters[' '] = Character(c_spc);

    const char * c_col[] = {    "   ",
                                "   ",
                                "   ",
                                " * ",
                                "   ",
                                "   ",
                                "   ",
                                "   ",
                                " * "};
    _characters[':'] = Character(c_col);

    const char * c_per[] = {    "   ",
                                "   ",
                                "   ",
                                "   ",
                                "   ",
                                "   ",
                                "   ",
                                "   ",
                                " * "};
    _characters['.'] = Character(c_per);

    const char * c_hyph[] = {   "     ",
                                "     ",
                                "     ",
                                "     ",
                                "     ",
                                "**** ",
                                "     ",
                                "     ",
                                "     "};
    _characters['-'] = Character(c_hyph);
    
    const char * c_zero[] = {   "  ***  ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                "  ***  "};
    _characters['0'] = Character(c_zero);

    const char * c_one[] = {    "   *   ",
                                " ***   ",
                                "   *   ",
                                "   *   ",
                                "   *   ",
                                "   *   ",
                                "   *   ",
                                "   *   ",
                                "   *   "};
    _characters['1'] = Character(c_one);
    
    const char * c_two[] = {    "  ***  ",
                                " *   * ",
                                "     * ",
                                "    *  ",
                                "   *   ",
                                "  *    ",
                                " *     ",
                                " *     ",
                                " ***** "};
    _characters['2'] = Character(c_two);
    
    const char * c_three[] = {  "  ***  ",
                                " *   * ",
                                "     * ",
                                "   **  ",
                                "     * ",
                                "     * ",
                                " *   * ",
                                " *   * ",
                                "  ***  "};
    _characters['3'] = Character(c_three);
    
    const char * c_four[] = {   "    *  ",
                                "   **  ",
                                "  * *  ",
                                "  * *  ",
                                " *  *  ",
                                "*   *  ",
                                "****** ",
                                "    *  ",
                                "    *  "};
    _characters['4'] = Character(c_four);
    
    const char * c_five[] = {   " ***** ",
                                " *     ",
                                " *     ",
                                " ****  ",
                                "     * ",
                                "     * ",
                                " *   * ",
                                " *   * ",
                                "  ***  "};
    _characters['5'] = Character(c_five);

    const char * c_six[] = {    "  ***  ",
                                " *   * ",
                                " *     ",
                                " * **  ",
                                " **  * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                "  ***  "};
    _characters['6'] = Character(c_six);

    const char * c_seven[] = {  " ***** ",
                                "     * ",
                                "    *  ",
                                "    *  ",
                                "   *   ",
                                "   *   ",
                                "   *   ",
                                "  *    ",
                                "  *    "};
    _characters['7'] = Character(c_seven);
    
    const char * c_eight[] = {  "  ***  ",
                                " *   * ",
                                " *   * ",
                                "  ***  ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                "  ***  "};
    _characters['8'] = Character(c_eight);

    const char * c_nine[] = {   "  ***  ",
                                " *   * ",
                                " *   * ",
                                " *   * ",
                                "  **** ",
                                "     * ",
                                "     * ",
                                " *   * ",
                                "  ***  "};
    _characters['9'] = Character(c_nine);

}

#endif