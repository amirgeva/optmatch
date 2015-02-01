#ifndef H_EXCEPTIONS_OPT_MATCH
#define H_EXCEPTIONS_OPT_MATCH

namespace OpticMatch {

  class general_message_exception : public std::exception
  {
    std::string m_Message;
  public:
    general_message_exception(const std::string& msg)
      : m_Message(msg) {}
    virtual const char* what() const override
    {
      return m_Message.c_str();
    }
  };

#define OPTMATCH_EXCEPTION(x)\
  class x : public general_message_exception { public:\
  x(const std::string& msg) : general_message_exception(msg) {} }

  OPTMATCH_EXCEPTION(invalid_parameters_exception);

#undef OPTMATCH_EXCEPTION

} // namespace OpticMatch

#endif // H_EXCEPTIONS_OPT_MATCH
