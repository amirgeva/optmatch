#ifndef H_GENERATOR_OPTMATCH
#define H_GENERATOR_OPTMATCH

#include <memory>
#include <opencv2/opencv.hpp>
#include <optmatch/exceptions.h>

namespace OpticMatch {

class CharImageGenerator
{
public:
  virtual ~CharImageGenerator() {}
  virtual bool generate(cv::Mat& image, wchar_t& c) = 0;

  static std::shared_ptr<CharImageGenerator> create(const std::string& params);
};

class FontGenerator
{
  typedef std::wstring str;
  typedef std::vector<str> str_vec;
  typedef std::vector<int> int_vec;
  typedef str_vec::const_iterator str_it;
  typedef int_vec::const_iterator int_it;
  typedef str::const_iterator chr_it;
  str_vec m_Faces;
  int_vec m_Heights, m_Weights, m_Italics;
  str     m_Alphabet;
  str_it  m_FaceIt;
  int_it  m_HeightIt, m_WeightIt, m_ItalicIt;
  chr_it  m_CharIt;
public:
  FontGenerator() : m_Italics(1, 0), m_Alphabet(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ") {}
  void add_face(const str& face) { m_Faces.push_back(face); }
  void add_height(int h) { m_Heights.push_back(h); }
  void add_weight(int w) { m_Weights.push_back(w); }
  void add_italics(int i) { m_Italics.push_back(i); }
  void set_alphabet(const str& abc) { m_Alphabet = abc; }

  void reset()
  {
    m_FaceIt = m_Faces.begin();
    m_HeightIt = m_Heights.begin();
    m_WeightIt = m_Weights.begin();
    m_ItalicIt = m_Italics.begin();
    m_CharIt = m_Alphabet.begin();
  }

  const str& get_face() const { return *m_FaceIt; }
  int get_height() const { return *m_HeightIt; }
  int get_weight() const { return *m_WeightIt; }
  int get_italic() const { return *m_ItalicIt; }
  wchar_t get_char() const { return *m_CharIt; }

  bool next()
  {
    if (++m_CharIt == m_Alphabet.end())
    {
      m_CharIt = m_Alphabet.begin();
      if (++m_ItalicIt == m_Italics.end())
      {
        m_ItalicIt = m_Italics.begin();
        if (++m_WeightIt == m_Weights.end())
        {
          m_WeightIt = m_Weights.begin();
          if (++m_HeightIt == m_Heights.end())
          {
            m_HeightIt = m_Heights.begin();
            if (++m_FaceIt == m_Faces.end())
            {
              return false;
            }
          }
        }
      }
    }
    return true;
  }
};

} // namespace OpticMatch 

#endif // H_GENERATOR_OPTMATCH
