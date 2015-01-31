#ifndef H_OPTIC_MATCH
#define H_OPTIC_MATCH

#include <optmatch/generator.h>

namespace OpticMatch {

class CharClassifier
{
public:
  virtual ~CharClassifier() {}
  
  virtual bool add_training_sample(const cv::Mat& image, wchar_t c) = 0;
  virtual bool train(CharImageGenerator& cig) = 0;
  virtual wchar_t classify(const cv::Mat& image, double* conf=nullptr) const = 0;
  
  static std::shared_ptr<CharClassifier> create(const std::string& params); 
};

} // namespace OpticMatch

#endif // H_OPTIC_MATCH
