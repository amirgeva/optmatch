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
#ifndef H_XML_OPT_MATCH
#define H_XML_OPT_MATCH

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <optmatch/exceptions.h>

using std::string;

namespace OpticMatch {

  typedef std::shared_ptr<class xml_element> xml_ptr;

  class xml_element
  {
    typedef xml_element& reference;
    typedef xml_ptr pointer;
    typedef std::vector<pointer> child_vec;
    typedef std::map<string, string> attr_map;

    string    m_Type;
    child_vec m_Children;
    attr_map  m_Attributes;
    string    m_Content;

    xml_element(const xml_element&) {}
    xml_element& operator= (const xml_element&) { return *this; }
  public:
    xml_element(const std::string& type = "") : m_Type(type) {}

    void set_type(const string& type) { m_Type = type; }
    const string& get_type() const { return m_Type; }

    int  get_child_count() const { return int(m_Children.size()); }
    void add_child(pointer p) { m_Children.push_back(p); }

    xml_ptr add_child(const string& type)
    {
      xml_ptr child(new xml_element(type));
      add_child(child);
      return child;
    }

    void remove(pointer child)
    {
      for (iterator it = begin(); it != end(); ++it)
      {
        xml_ptr c = *it;
        if (c == child) { m_Children.erase(it); break; }
      }
    }

    xml_ptr find_child(const string& type, const string& attr_name, const string& attr_val)
    {
      for (iterator it = begin(); it != end(); ++it)
      {
        xml_ptr c = *it;
        if (c->get_type() == type && c->get_attribute(attr_name) == attr_val) return c;
      }
      return xml_ptr();
    }

    xml_ptr find_child(const string& type)
    {
      for (iterator it = begin(); it != end(); ++it)
      {
        xml_ptr c = *it;
        if (c->get_type() == type) return c;
      }
      return xml_ptr();
    }

    bool has_attribute(const string& name) const { return m_Attributes.count(name) > 0; }
    void set_attribute(const string& name, const string& value) { m_Attributes[name] = value; }
    string get_attribute(const string& name) const
    {
      attr_map::const_iterator it = m_Attributes.find(name);
      if (it == m_Attributes.end()) return "";
      return it->second;
    }
    typedef child_vec::iterator iterator;
    typedef child_vec::const_iterator const_iterator;
    iterator begin() { return m_Children.begin(); }
    iterator end()   { return m_Children.end(); }
    const_iterator begin() const { return m_Children.begin(); }
    const_iterator end()   const { return m_Children.end(); }

    typedef attr_map::const_iterator attr_iterator;
    attr_iterator attr_begin() const { return m_Attributes.begin(); }
    attr_iterator attr_end()   const { return m_Attributes.end(); }

    void print(std::ostream& os = std::cout, int indent = 0, bool packed = false) const
    {
      string spaces = packed ? string("") : string(indent, ' ');
      string eol = packed ? string("") : string("\n");
      os << spaces << "<" << get_type();
      for (attr_iterator it = attr_begin(); it != attr_end(); ++it)
        os << " " << it->first << "=\"" << it->second << "\"";
      if (get_child_count() == 0 && m_Content.empty()) { os << "/>" << eol; return; }
      os << ">" << eol;
      if (!m_Content.empty()) os << spaces << m_Content << eol;
      for (const_iterator ci = begin(); ci != end(); ++ci)
        (*ci)->print(os, indent + 2, packed);
      os << spaces << "</" << get_type() << ">" << eol;
    }

    string print(bool packed)
    {
      std::ostringstream os;
      print(os, 0, packed);
      return os.str();
    }
  };

  class xml_parser
  {
  public:
    xml_parser() : m_InQuotes(false), m_LineNumber(1) {}
  private:
    enum Token { LTAG, RTAG, EQ, QUOTES, SLASH, IDENT, TEXT, XEOF };

    bool m_InQuotes;
    int  m_LineNumber;

    static bool is_white_space(char c)
    {
      return (c <= 32);
    }

    static bool quotes_pred(char c)
    {
      return c == '"';
    }

    static bool not_alnum(char c)
    {
      return ((c<'A' || c>'Z') && (c<'a' || c>'z') && (c<'0' || c>'9') && c != '_');
    }

    Token analyze(std::istream& is, string& token_text)
    {
      char ch = ' ';
      while (!is.eof() && is_white_space(ch))
      {
        ch = is.get();
        if (ch == '\n') ++m_LineNumber;
      }
      token_text = "";
      if (is.eof()) return XEOF;
      token_text = string(1, ch);
      if (ch == '"')
      {
        m_InQuotes = !m_InQuotes;
        return QUOTES;
      }
      if (m_InQuotes)
      {
        token_text += read_until(is, quotes_pred);
        return TEXT;
      }
      if (ch == '<') { return LTAG; }
      if (ch == '>') { return RTAG; }
      if (ch == '=') { return EQ; }
      if (ch == '/') { return SLASH; }
      token_text += read_until(is, not_alnum);
      return IDENT;
    }

    template<class PRED>
    string read_until(std::istream& is, PRED p)
    {
      string res;
      while (!is.eof())
      {
        char ch = is.peek();
        if (p(ch)) return res;
        ch = is.get();
        res += string(1, ch);
      }
      return res;
    }

#define SYNTAX_ERROR throw general_message_exception("XML Syntax Error")
#define EXPECT(t) { token=analyze(is,last); if (token!=t) SYNTAX_ERROR; }

    void parse_element(std::istream& is, xml_ptr parent)
    {
      string last;
      Token  token;
      while (true)
      {
        token = analyze(is, last);
        if (token == XEOF) return;
        if (token == LTAG)
        {
          token = analyze(is, last);
          if (token == SLASH)
          {
            if (!parent) SYNTAX_ERROR;
            EXPECT(IDENT);
            if (last != parent->get_type()) SYNTAX_ERROR;
            EXPECT(RTAG);
            return;
          }
          else
          if (token == IDENT)
          {
            xml_ptr child(new xml_element);
            child->set_type(last);
            parent->add_child(child);
            while (true)
            {
              token = analyze(is, last);
              if (token == XEOF) return;
              if (token == IDENT)  // Attribute
              {
                string attr_name = last;
                EXPECT(EQ);
                EXPECT(QUOTES);
                token = analyze(is, last);
                string attr_value;
                if (token == TEXT)
                {
                  attr_value = last;
                  EXPECT(QUOTES);
                }
                else
                if (token != QUOTES) SYNTAX_ERROR;
                child->set_attribute(attr_name, attr_value);
              }
              else
              if (token == SLASH)
              {
                EXPECT(RTAG);
                break;
              }
              else
              if (token == RTAG)
              {
                parse_element(is, child);
                break;
              }
            }
          }
        }
      }
    }

  public:

    xml_ptr parse(std::istream& is)
    {
      if (is.fail()) return 0;
      xml_ptr root(new xml_element);
      try
      {
        parse_element(is, root);
        int n = root->get_child_count();
        if (n > 1) throw "Error: Multiple root nodes";
        if (n == 0) throw "Error: No root node";
        xml_ptr new_root = *(root->begin());
        root->remove(new_root);
        root = new_root;
      }
      catch (const char* msg)
      {
        std::cerr << "Line " << m_LineNumber << " - " << msg << std::endl;
        root.reset();
      }
      return root;
    }
  };

  inline xml_ptr load_xml_from_file(const char* filename)
  {
    std::ifstream fin(filename);
    if (fin.fail()) return 0;
    return xml_parser().parse(fin);
  }

  inline xml_ptr load_xml_from_file(const string& filename)
  {
    return load_xml_from_file(filename.c_str());
  }

  inline xml_ptr load_xml_from_text(const string& text)
  {
    std::istringstream is(text);
    return xml_parser().parse(is);
  }

  inline string get_xml_text(xml_ptr root)
  {
    std::ostringstream os;
    root->print(os, 0, false);
    return os.str();
  }

} // namespace OpticMatch

#endif // H_XML_OPT_MATCH

