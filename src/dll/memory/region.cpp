#include "region.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdexcept>

Region::Region( unsigned char* begin, std::ptrdiff_t size, std::uint32_t protection )
  : m_begin( begin )
  , m_size( size )
{
  DWORD prev;
  if( !VirtualProtect( begin, size, protection, &prev ) )
  {
    throw std::runtime_error( "Failed to set page protection!" );
  }
  m_prevProtection = prev;
}

Region::Region( Region&& rhs )
  : m_begin( rhs.m_begin )
  , m_size( rhs.m_size )
  , m_prevProtection( rhs.m_prevProtection )
{
  rhs.m_size = 0;
}

Region::~Region()
{
  resetProtection();
}

Region& Region::operator=( Region&& rhs )
{
  resetProtection();

  m_begin = rhs.m_begin;
  m_size = rhs.m_size;
  m_prevProtection = rhs.m_prevProtection;

  rhs.m_size = 0;

  return *this;
}

void Region::resetProtection()
{
  if( m_size == 0 )
  {
    return;
  }

  DWORD prev;
  if( !VirtualProtect( m_begin, m_size, m_prevProtection, &prev ) )
  {
    throw std::runtime_error( "Failed to reset page protection!" );
  }
}
