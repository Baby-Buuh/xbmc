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

#if defined(HAVE_XKBCOMMON)

#include <cstdint>
#include <string>

#include <xkbcommon/xkbcommon.h>

#include "input/XBMC_keysym.h"

namespace KODI
{
namespace KEYBOARD
{

class CXkbcommonKeymap
{
public:

  explicit CXkbcommonKeymap(xkb_keymap* keymap);
  ~CXkbcommonKeymap();
  
  std::uint32_t KeysymForKeycode(std::uint32_t code) const;
  void UpdateMask(std::uint32_t depressed,
                  std::uint32_t latched,
                  std::uint32_t locked,
                  std::uint32_t group);
  std::uint32_t CurrentModifiers() const;
  XBMCKey XBMCKeysymForKeycode(std::uint32_t code) const;
  XBMCMod ActiveXBMCModifiers() const;
  std::uint32_t UnicodeCodepointForKeycode(std::uint32_t code) const;

  static xkb_context * CreateXkbContext();
  static xkb_keymap * ReceiveXkbKeymapFromSharedMemory(xkb_context * context, int fd, std::size_t size);
  static xkb_state * CreateXkbStateFromKeymap(xkb_keymap *keymap);
  static xkb_keymap * CreateXkbKeymapFromNames(xkb_context * context, const std::string &rules, const std::string &model, const std::string &layout, const std::string &variant, const std::string &options);
private:
  
  xkb_keymap* m_keymap;
  xkb_state* m_state;

  xkb_mod_index_t m_internalLeftControlIndex;
  xkb_mod_index_t m_internalLeftShiftIndex;
  xkb_mod_index_t m_internalLeftSuperIndex;
  xkb_mod_index_t m_internalLeftAltIndex;
  xkb_mod_index_t m_internalLeftMetaIndex;

  xkb_mod_index_t m_internalRightControlIndex;
  xkb_mod_index_t m_internalRightShiftIndex;
  xkb_mod_index_t m_internalRightSuperIndex;
  xkb_mod_index_t m_internalRightAltIndex;
  xkb_mod_index_t m_internalRightMetaIndex;

  xkb_mod_index_t m_internalCapsLockIndex;
  xkb_mod_index_t m_internalNumLockIndex;
  xkb_mod_index_t m_internalModeIndex;
};

}
}

#endif