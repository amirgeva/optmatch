#include "stdafx.h"

#ifdef WIN32

#include <windows.h>
#include <opencv2/opencv.hpp>
#include <optmatch/generator.h>
#include <optmatch/xml.h>

namespace OpticMatch {

  void crop(cv::Mat& image);

  class WindowsCharImageGenerator : public CharImageGenerator
  {
    HDC     m_DC;
    HBITMAP m_Bitmap;
    char*   m_Buffer;
    int     m_Pitch;
    RECT    m_Rect;
    int     m_Width;
    int     m_Height;
    FontGenerator m_Generator;
    bool    m_Done;
    
  public:
    WindowsCharImageGenerator(const std::string& params)
      : m_DC(0)
      , m_Bitmap(0)
      , m_Buffer(0)
      , m_Pitch(0)
      , m_Width(640)
      , m_Height(480)
      , m_Done(false)
    {
      if (!params.empty())
      {
        xml_ptr root=load_xml_from_text(params);
        m_Generator.load_from_xml(root);
      }
      else
      {
        m_Generator.add_face(L"Arial");
        m_Generator.add_height(24);
        m_Generator.add_weight(400);
      }
      m_Generator.reset();
      m_DC = CreateCompatibleDC(0);
      BITMAPINFO bi;
      ZeroMemory(&bi, sizeof(BITMAPINFO));
      bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bi.bmiHeader.biWidth = m_Width;
      bi.bmiHeader.biHeight = m_Height;
      bi.bmiHeader.biPlanes = 1;
      bi.bmiHeader.biBitCount = 32;
      m_Bitmap = CreateDIBSection(m_DC, &bi, DIB_RGB_COLORS, (LPVOID*)&m_Buffer, NULL, 0);
      if (m_Bitmap == NULL) throw general_message_exception("Failed to create windows bitmap");
      m_Rect.left = m_Rect.top = 0;
      m_Rect.right = m_Width;
      m_Rect.bottom = m_Height;
      SelectObject(m_DC, m_Bitmap);
    }

    ~WindowsCharImageGenerator()
    {
      if (m_Bitmap)
        DeleteObject(m_Bitmap);
      if (m_DC)
        DeleteDC(m_DC);
    }

    virtual bool generate(cv::Mat& image, wchar_t& c) override
    {
      if (m_Done) return false;
      HBRUSH br = CreateSolidBrush(RGB(255, 255, 255));
      FillRect(m_DC, &m_Rect, br);
      c = m_Generator.get_char();
      LOGFONT font_desc;
      ZeroMemory(&font_desc, sizeof(LOGFONT));
      font_desc.lfHeight = -MulDiv(m_Generator.get_height(), GetDeviceCaps(m_DC, LOGPIXELSY), 72);
      font_desc.lfWeight = m_Generator.get_weight();
      font_desc.lfItalic = m_Generator.get_italic();
      font_desc.lfCharSet = DEFAULT_CHARSET;
      font_desc.lfOutPrecision = OUT_DEFAULT_PRECIS;
      font_desc.lfQuality = ANTIALIASED_QUALITY;
      font_desc.lfPitchAndFamily = DEFAULT_PITCH;
      wcscpy(font_desc.lfFaceName, m_Generator.get_face().c_str());
      HFONT font = CreateFontIndirect(&font_desc);
      HFONT old_font = (HFONT)SelectObject(m_DC, font);
      TextOut(m_DC, 100, 200, &c, 1);
      cv::Mat tmp(cv::Size(m_Width, m_Height), CV_8UC4, m_Buffer, cv::Mat::AUTO_STEP);
      cv::flip(tmp, tmp, 0);
      cv::cvtColor(tmp, image, CV_RGBA2GRAY);
      crop(image);
      
      SelectObject(m_DC, old_font);
      DeleteObject(font);
      DeleteObject(br);
      if (!m_Generator.next()) m_Done = true;
      return true;
    }
  };

  std::shared_ptr<CharImageGenerator> CharImageGenerator::create(const std::string& params)
  {
    return std::shared_ptr<CharImageGenerator>(new WindowsCharImageGenerator(params));
  }


} // namespace OpticMatch

#endif // WIN32
