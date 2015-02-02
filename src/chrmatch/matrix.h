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
#ifndef H_MATRIX
#define H_MATRIX

#include <vector>

template<class T>
class Matrix
{
	typedef std::vector<T> base_vec;
  base_vec               m_Elements;
  unsigned               m_Width,m_Height;

	void init(unsigned w, unsigned h, const T& init_value)
	{
		unsigned buffer_size=w*h;
		if (buffer_size>0U)
			m_Elements.resize(buffer_size,init_value);
	}
public:
	Matrix(unsigned width=0, unsigned height=0, const T& init_value=T())
		: m_Width(width)
    , m_Height(height)
	{
		init(width,height,init_value);
	}

	const T& operator() (int x, int y) const
	{
    assert(x >= 0 && y >= 0 && x < int(m_Width) && y < int(m_Height));
		return m_Elements[y*m_Width+x];
	}

	T& operator() (int x, int y)
	{
    assert(x >= 0 && y >= 0 && x < int(m_Width) && y < int(m_Height));
    return m_Elements[y*m_Width + x];
	}

	const T* get_row(int y) const
	{
    assert(y >= 0 && y < int(m_Height));
		return &(m_Elements[y*m_Width]);
	}

	T* get_row(int y)
	{
    assert(y >= 0 && y < int(m_Height));
    return &(m_Elements[y*m_Width]);
  }

	unsigned get_width()  const { return m_Width; }
	unsigned get_height() const { return m_Height; }

  void print(std::ostream& os) const
  {
    for(unsigned y=0;y<get_height();++y)
    {
      const T* row=get_row(y);
      for(unsigned x=0;x<get_width();++x)
      {
        if (x>0) os << "\t";
        os << row[x];
      }
      os << std::endl;
    }
  }
};


#endif // H_MATRIX
