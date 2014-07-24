#include "logger.hpp"

Logger& Logger::getSingleton()
{
  static Logger s_logger;
  return s_logger;
}

Logger::Logger()
  : m_stream( "fo2unleashed.log" )
  , m_logLevel( LogLevel::Info )
{
  info( "Log session starts." );
  setLogLevel( LogLevel::Default );
}

Logger::~Logger()
{
  setLogLevel( LogLevel::Info );
  info( "Log session ends." );
}

void Logger::write()
{
  m_stream << std::endl;
}

const char* Logger::levelName( LogLevel logLevel )
{
  switch( logLevel )
  {
  case LogLevel::Error:
    return "Error";
  case LogLevel::Warning:
    return "Warning";
  case LogLevel::Info:
    return "Info";
  default:
    return "";
  }
}
