#include "stdafx.h"

namespace OpticMatch {

  void crop(cv::Mat& image)
  {
    int w = image.cols, h = image.rows;
    int l = w, t = h, r = 0, b = 0;
    for (int y = 0; y < h;++y)
    {
      const unsigned char* row=image.ptr(y);
      for (int x = 0; x < w;++x)
      {
        if (row[x]!=255)
        {
          l = Min(l, x);
          r = Max(r, x + 1);
          t = Min(t, y);
          b = Max(b, y + 1);
        }
      }
    }
    cv::Rect rect(l, t, r - l, b - t);
    image = image(rect);
  }

} // namespace OpticMatch
