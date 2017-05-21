/*
 *      Copyright (C) 2017 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include <cstdint>

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

/**
 * Convenience class for defining protocols
 * 
 * Do NOT use this for polymorphism
 */
template<typename WaylandT>
class CProtocolBase
{
public:
  typedef WaylandT NativeType;
  
  explicit CProtocolBase(NativeType* native)
  : m_native(native)
  {}
    
  CProtocolBase(const CProtocolBase &) = delete;
  CProtocolBase &operator=(const CProtocolBase &) = delete;
  
  NativeType* GetNative()
  {
    return m_native;
  }
  
  // Allow implicit conversion
  operator NativeType*()
  {
    return GetNative();
  }
  
protected:
  NativeType* m_native;
};


/**
 * Meta-information about global protocol wrapper
 * 
 * Each global protocol wrapper class should have this information as member "Meta"
 * 
 * \param protocol protocol name
 * \param version supported version (minimum)
 * \param interface wl_interface to use for binding
 */
struct ProtocolTag final
{
  constexpr ProtocolTag(const char* protocol, std::uint32_t version, const wl_interface * const interface)
  : protocol(protocol), version(version), interface(interface)
  {}
  const char * protocol;
  std::uint32_t version;
  const wl_interface * interface;
};

}
}
}
}