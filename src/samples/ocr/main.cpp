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
#include <optmatch/optmatch.h>
#include <optmatch/cc.h>

cv::Mat create_image_from_string(const std::string& s, int w, int h)
{
  int idx = 0;
  cv::Mat image(w, h, CV_8UC1);
  if (s.length() < unsigned(w*h)) return image;
  for (int y = 0; y < h;++y)
  {
    unsigned char* row = image.ptr(y);
    for (int x = 0; x < w; ++x)
    {
      if (s[idx++] == '-') row[x] = 255;
      else row[x] = 0;
    }
  }
  return image;
}

int main(int argc, char* argv[])
{
  try
  {
    auto cls = OpticMatch::CharClassifier::create("");
    std::string params = "<fonts> "
      "<font face=\"Arial\"/> "
      "<font face=\"Courier New\"/> "
      "<font face=\"Times New Roman\"/> "
      "<height value=\"24\"/>"
      "<weight value=\"400\"/>"
      "<weight value=\"700\"/>"
      "</fonts>";
    auto trainer = OpticMatch::CharImageGenerator::create("");
    //auto trainer = OpticMatch::CharImageGenerator::create(params);
    cls->train(*trainer);
    std::string test_image =
      "------------"
      "------O-----"
      "-----OOO----"
      "----OO-OO---"
      "---OO--OO---"
      "--OO----OO--"
      "--OO----OO--"
      "--OO----OO--"
      "-OOOOOOOOOO-"
      "-OO------OO-"
      "-OO------OO-"
      "------------";
    cv::Mat img = create_image_from_string(test_image, 12, 12);
    double conf=0;
    wchar_t c=cls->classify(img, &conf);
    std::wcout << c << L"  " << conf << std::endl;
  } catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
