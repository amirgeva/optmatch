/***************************************************************************
Copyright (c) 2013-2015, Amir Geva
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "stdafx.h"

#ifdef __GNUG__

#include <opencv2/opencv.hpp>
#include <optmatch/generator.h>
#include <optmatch/xml.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace OpticMatch {

void crop(cv::Mat& image);

class FreeTypeWrapper
{
public:
  FreeTypeWrapper()
  {
  }
  
  ~FreeTypeWrapper()
  {
  }
  
  void render_char(cv::Mat& image, wchar_t c)
  {
  }
  
} g_FreeType;


class FreeTypeCharImageGenerator : public CharImageGenerator
{
  bool          m_Done;
  FontGenerator m_Generator;

  FT_Library m_FreeType;
  FT_Face    m_Face;
  std::string m_FaceName;

  void load_face(const std::wstring& wface)
  {
    std::string face(wface.length(),' ');
    std::copy(wface.begin(),wface.end(),face.begin());
    if (face==m_FaceName) return;
    m_FaceName=face;
    if (FT_New_Face( m_FreeType,face.c_str(),0,&m_Face)) throw general_message_exception("Failed to load font: "+face);
  }
  
  void set_size(int size)
  {
    if (FT_Set_Pixel_Sizes(m_Face,0,size)) throw general_message_exception("Failed to set font size");
  }
  

public:
  FreeTypeCharImageGenerator(const std::string& params)
  : m_Done(false)
  {
    if (FT_Init_FreeType(&m_FreeType)) throw general_message_exception("Failed to initialize FreeType2");
    if (!params.empty())
    {
      xml_ptr root=load_xml_from_text(params);
      m_Generator.load_from_xml(root);
    }
    else
    {
      m_Generator.add_face(L"/usr/share/fonts/truetype/freefont/FreeSans.ttf");
      m_Generator.add_height(24);
      m_Generator.add_weight(400);
    }
    m_Generator.reset();
  }
  
  virtual bool generate(cv::Mat& image, wchar_t& c) override
  {
    if (m_Done) return false;
    load_face(m_Generator.get_face());
    set_size(m_Generator.get_height());
    
    c=m_Generator.get_char();
    int glyph_index = FT_Get_Char_Index(m_Face, c);
    int flags=0;
    if (FT_Load_Glyph(m_Face,glyph_index,flags)) throw general_message_exception("Failed to glyph");
    auto render_mode=FT_RENDER_MODE_NORMAL;
    if (FT_Render_Glyph( m_Face->glyph,render_mode)) throw general_message_exception("Failed to render glyph");
    FT_Bitmap& bmp=m_Face->glyph->bitmap;
    cv::Mat tmp(cv::Size(bmp.width, bmp.rows), CV_8UC1, bmp.buffer, bmp.pitch);
    image=cv::Mat::ones(tmp.size(),tmp.type())*255;
    cv::subtract(image,tmp,image);
    crop(image);
    if (!m_Generator.next()) m_Done=true;
    return true;
  }
};

std::shared_ptr<CharImageGenerator> CharImageGenerator::create(const std::string& params)
{
  return std::shared_ptr<CharImageGenerator>(new FreeTypeCharImageGenerator(params));
}

} // namespace OpticMatch

#endif

