#include <optmatch/optmatch.h>

cv::Mat create_image_from_string(const std::string& s, int w, int h)
{
  int idx = 0;
  cv::Mat image(w, h, CV_8UC1);
  if (s.length() < (w*h)) return image;
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
  auto cls = OpticMatch::CharClassifier::create("");
  std::string params = "<fonts> "
    "<font face=\"Arial\"/> "
    "<font face=\"Courier New\"/> "
    "<font face=\"Times New Roman\"/> "
    "<height value=\"24\"/>"
    "<weight value=\"400\"/>"
    "<weight value=\"700\"/>"
    "</fonts>";
  auto trainer = OpticMatch::CharImageGenerator::create(params);
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
  return 0;
}
