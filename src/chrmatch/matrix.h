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
