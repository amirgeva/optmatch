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
#include "matrix.h"
#include "utils.h"

namespace OpticMatch {

  typedef unsigned char byte;
  const int NSIZE = 24;

  class PerimeterPixel
  {
    byte   px,py;
    byte   gradients;
  public:
    PerimeterPixel(byte x = 0, byte y = 0)
      : px(x), py(y), gradients(0)
    {}

    byte x() const { return px; }
    byte y() const { return py; }
    byte g() const { return gradients; }

    void set_grad(byte g)    { gradients = g; }
    void add_grad(byte g)    { gradients |= g; }
    void set(byte x, byte y) { px = x; py = y; }
  };

  class Cell
  {
    byte           m_X,m_Y;
    unsigned short m_SqDist;
  public:
    Cell(unsigned x = 0, unsigned y = 0, unsigned short sqdist = 65535)
      : m_X(x)
      , m_Y(y)
      , m_SqDist(sqdist)
    {}

    byte x() const { return m_X; }
    byte y() const { return m_Y; }

    unsigned short sqr_dist() const { return m_SqDist; }
    void set(unsigned short sd) { m_SqDist = sd; }
    void set(byte x, byte y) { m_X = x; m_Y = y; }
    void set(byte x, byte y, unsigned short sd) { set(x, y); m_SqDist = sd; }
  };

  inline std::ostream& operator << (std::ostream& os, const Cell& c)
  {
    return os << c.sqr_dist();
  }

  typedef Matrix<Cell> cell_mat;
  typedef std::list<PerimeterPixel> pp_seq;
  typedef std::vector<cell_mat> cm_vec;
  typedef std::vector<byte> bvec;

  const int LEFT_IDX = 0;
  const int TOP_IDX = 1;
  const int RIGHT_IDX = 2;
  const int BOTTOM_IDX = 3;

  const unsigned LEFT = 1;
  const unsigned TOP = 2;
  const unsigned RIGHT = 4;
  const unsigned BOTTOM = 8;

  static const cell_mat s_EmptyMat(NSIZE, NSIZE, Cell());
  static const byte blank_row[] = { 255, 255, 255, 255, 255, 255, 255, 255,
                                    255, 255, 255, 255, 255, 255, 255, 255,
                                    255, 255, 255, 255, 255, 255, 255, 255,
                                    255, 255, 255, 255, 255, 255, 255, 255 };

  class Perimeter
  {
    typedef std::vector<PerimeterPixel> pp_vec;
    pp_vec  m_Points;
    cm_vec  m_Matrices;

    inline void check(cell_mat& m, const Cell& o, int x, int y, pp_seq& horizon)
    {
      unsigned short sd = sqr(x - int(o.x())) + sqr(y - int(o.y()));
      if (sd < m(x, y).sqr_dist())
      {
        m(x, y).set(o.x(), o.y(), sd);
        horizon.push_back(PerimeterPixel(x, y));
      }
    }

    void finalize_matrix(cell_mat& m)
    {
      static int cnt = 0; ++cnt;

      int x, y, w = m.get_width(), h = m.get_height();
      pp_seq horizon;
      for (y = 0; y < h; ++y)
      {
        Cell* row = m.get_row(y);
        for (x = 0; x < w; ++x)
        {
          if (row[x].sqr_dist() == 0)
          {
            row[x].set(x, y);
            horizon.push_back(PerimeterPixel(x, y));
          }
        }
      }

      while (!horizon.empty())
      {
        PerimeterPixel p = horizon.front();
        horizon.pop_front();
        x = p.x();
        y = p.y();
        const Cell& o = m(x, y);

        if (x > 0)       check(m, o, x - 1, y, horizon);
        if (x < (w - 1)) check(m, o, x + 1, y, horizon);
        if (y > 0)       check(m, o, x, y - 1, horizon);
        if (y < (h - 1)) check(m, o, x, y + 1, horizon);
      }
    }

    void minimize_matrix(cell_mat& dst, const cell_mat& src)
    {
      unsigned x, y, w = dst.get_width(), h = dst.get_height();
      for (y = 0; y < h; ++y)
      {
        Cell* drow = dst.get_row(y);
        const Cell* srow = src.get_row(y);
        for (x = 0; x < w; ++x)
        {
          if (srow[x].sqr_dist() < drow[x].sqr_dist())
            drow[x] = srow[x];
        }
      }
    }

    void finalize_matrices()
    {
      for (unsigned i = 0; i < 4; ++i)
        finalize_matrix(m_Matrices[i]);
      cm_vec mat16(16, s_EmptyMat);
      for (unsigned i = 0; i < 16; ++i)
      {
        if ((i & LEFT)   == LEFT)   minimize_matrix(mat16[i], m_Matrices[LEFT_IDX]);
        if ((i & TOP)    == TOP)    minimize_matrix(mat16[i], m_Matrices[TOP_IDX]);
        if ((i & RIGHT)  == RIGHT)  minimize_matrix(mat16[i], m_Matrices[RIGHT_IDX]);
        if ((i & BOTTOM) == BOTTOM) minimize_matrix(mat16[i], m_Matrices[BOTTOM_IDX]);
      }
      m_Matrices.swap(mat16);
    }
  public:
    Perimeter() {}

    Perimeter(const cv::Mat& image)
    {
      build_matrices(image);
    }

    void build_matrices(const cv::Mat& image)
    {
      if (m_Matrices.empty())
      {
        m_Matrices.resize(4, s_EmptyMat);
      }
      unsigned w = image.cols, h = image.rows;
      for (unsigned y = 0; y < h; ++y)
      {
        const byte* prow = (y == 0 ? &blank_row[0] : image.ptr(y - 1));
        const byte* crow = image.ptr(y);
        const byte* nrow = (y == (h - 1) ? &blank_row[0] : image.ptr(y + 1));
        for (unsigned x = 0; x < w; ++x)
        {
          if (crow[x] == 0)
          {
            if (x == 0 || x == (w - 1) || crow[x - 1] == 255 || crow[x + 1] == 255 || prow[x] == 255 || nrow[x] == 255)
            {
              m_Points.push_back(PerimeterPixel(x, y));
              if (x == 0 || crow[x - 1] == 255)
              {
                (m_Matrices[LEFT_IDX])  (x, y).set(0);
                m_Points.back().add_grad(LEFT);
              }
              if (x == (w - 1) || crow[x + 1] == 255)
              {
                (m_Matrices[RIGHT_IDX]) (x, y).set(0);
                m_Points.back().add_grad(RIGHT);
              }
              if (prow[x] == 255)
              {
                (m_Matrices[TOP_IDX])   (x, y).set(0);
                m_Points.back().add_grad(TOP);
              }
              if (nrow[x] == 255)
              {
                (m_Matrices[BOTTOM_IDX])(x, y).set(0);
                m_Points.back().add_grad(BOTTOM);
              }
            }
          }
        }
      }
      finalize_matrices();
    }

    typedef pp_vec::const_iterator const_iterator;
    const_iterator begin() const { return m_Points.begin(); }
    const_iterator end()   const { return m_Points.end(); }

    unsigned match(const Perimeter& p, unsigned thres) const
    {
      const int DEST_THRES = 4;
      unsigned sum = 0;
      Matrix<byte> count_mat(NSIZE, NSIZE, 0);
      for (const_iterator it = p.begin(); it != p.end(); ++it)
      {
        const PerimeterPixel& pp = *it;
        byte g = pp.g();
        const Cell& cell = (m_Matrices[g])(pp.x(), pp.y());
        unsigned d = cell.sqr_dist();
        if (d > thres)
        {
          sum += d;
          byte& count_cell = count_mat(cell.x(), cell.y());
          if (++count_cell > DEST_THRES)
          {
            if (count_cell > (DEST_THRES + 1))
              sum -= sqr(unsigned(count_cell) - 1U);
            sum += sqr(unsigned(count_cell));
          }
        }
      }
      return sum;
    }
  };

  double match(const Perimeter& a, const Perimeter& b, unsigned thres)
  {
    const double mult = sqr(1.0 / NSIZE);
    unsigned ma = a.match(b, thres);
    unsigned mb = b.match(a, thres);
    double m = 0.5*(ma + mb)*mult;
    return exp(-m);
  }

  void crop(cv::Mat& image);

  class OpticMatchCharClassifier : public CharClassifier
  {
    typedef std::vector<Perimeter>  pseq;
    typedef std::map<wchar_t, pseq> ts_map;

    ts_map m_TS;


    static cv::Mat normalize(const cv::Mat& src_image)
    {
      cv::Mat image;
      cv::resize(src_image, image, cv::Size(NSIZE, NSIZE));
      cv::threshold(image, image, 128, 255, cv::THRESH_BINARY);
      return image;
    }

  public:
    OpticMatchCharClassifier() {}

    virtual bool add_training_sample(const cv::Mat& image, wchar_t c) override
    {
      if (image.channels() != 1) throw invalid_parameters_exception("Only grayscale images are accepted.");
      cv::Mat img = normalize(image);
      auto it = m_TS.find(c);
      if (it==m_TS.end())
        it=m_TS.insert(std::make_pair(c,pseq())).first;
      pseq& s = it->second;
      s.push_back(Perimeter());
      s.back().build_matrices(img);
      return true;
    }

    virtual bool train(CharImageGenerator& cig) override
    {
      cv::Mat img;
      wchar_t c;
      int n = 0;
      while (cig.generate(img,c))
        add_training_sample(img, c);
      return n>0;
    }

    virtual wchar_t classify(const cv::Mat& image, double* conf) const override
    {
      if (m_TS.empty())
      {
        if (conf) *conf = 0;
        return wchar_t(0);
      }
      cv::Mat img = image;
      if (img.cols != NSIZE || img.rows != NSIZE)
      {
        crop(img);
        img = normalize(img);
      }
      Perimeter p(img);
      std::multimap<double, wchar_t> score_map;
      for (auto it = m_TS.begin(); it != m_TS.end();++it)
      {
        wchar_t c = it->first;
        const pseq& s = it->second;
        for(const auto& rp : s)
        {
          double score = OpticMatch::match(p, rp, 4);
          score_map.insert(std::make_pair(score, c));
        }
      }
      if (conf) *conf = score_map.rbegin()->first;
      return score_map.rbegin()->second;
    }
  };

  std::shared_ptr<CharClassifier> CharClassifier::create(const std::string& params)
  {
    CharClassifier* cls = new OpticMatchCharClassifier;
    return std::shared_ptr<CharClassifier>(cls);
  }

} // namespace OpticMatch
