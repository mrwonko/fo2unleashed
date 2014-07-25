#pragma once

#include <cstdint>
#include <cstddef>

class Region
{
public:
  Region( unsigned char* begin, std::ptrdiff_t size, std::uint32_t protection );
  Region( const Region& ) = delete;
  ~Region();
  Region& operator=( const Region& ) = delete;
  Region( Region&& rhs );
  Region& operator=( Region&& );

  unsigned char* begin() const { return m_begin; }
  std::ptrdiff_t size() const { return m_size; }

private:
  void resetProtection();

private:
  unsigned char* m_begin;
  std::ptrdiff_t m_size;
  std::uint32_t m_prevProtection;
};
