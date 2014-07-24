#pragma once

#include <fstream>

class Logger
{
public:
  enum class LogLevel
  {
    Error,
    Warning,
    Info,
    Verbose,
    None,
    Default = Verbose,
  };

public:
  ~Logger();
  Logger( const Logger& ) = delete;
  Logger& operator=( const Logger& ) = delete;
  Logger( Logger&& ) = delete;
  Logger& operator=( Logger&& ) = delete;

public:
  static Logger& getSingleton();

public:
  void setLogLevel( const LogLevel logLevel ) { m_logLevel = logLevel; }
  template< typename... Args >
  void verbose( Args... args ) { log( LogLevel::Verbose, args... ); }
  template< typename... Args >
  void info( Args... args ) { log( LogLevel::Info, args... ); }
  template< typename... Args >
  void warning( Args... args ) { log( LogLevel::Warning, args... ); }
  template< typename... Args >
  void error( Args... args ) { log( LogLevel::Error, args... ); }

private:
  Logger();

private:
  template< typename... Args >
  void log( LogLevel logLevel, Args... args );
  void write();
  template< typename Arg, typename... Args >
  void write( Arg arg, Args... args );

private:
  static const char* levelName( LogLevel logLevel );

private:
  std::ofstream m_stream;
  LogLevel m_logLevel;
};

template< typename... Args >
void Logger::log( LogLevel logLevel, Args... args )
{
  if( m_logLevel >= logLevel )
  {
    m_stream << levelName( logLevel ) << ": ";
    write( args... );
  }
}

template< typename Arg, typename... Args >
void Logger::write( Arg arg, Args... args )
{
  m_stream << arg;
  write( args... );
}
