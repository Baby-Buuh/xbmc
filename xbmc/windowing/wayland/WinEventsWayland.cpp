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

#include "WinEventsWayland.h"

#include "Application.h"
#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND;

namespace
{
wayland::display_t* g_WlDisplay = nullptr;
}

void CWinEventsWayland::SetDisplay(wayland::display_t* display)
{
  g_WlDisplay = display;
}

size_t CWinEventsWayland::GetQueueSize()
{
  // FIXME Where is this function used?
  throw std::runtime_error("Not implemented");
}

// FIXME Why is the return value of this ignored everywhere?
bool CWinEventsWayland::MessagePump()
{
  // TODO error handling; should be done in waylandpp
  if (!g_WlDisplay)
  {
    return false;
  }
  // TODO Maybe move processing to extra thread, dispatch only global events
  // with g_Application.OnEvent() here
  
  // Acquire global read lock
  while (g_WlDisplay->prepare_read() != 0)
  {
    g_WlDisplay->dispatch_pending();
  }
  g_WlDisplay->flush();

  // Read events and release lock; this does not block if no events are available
  g_WlDisplay->read_events();
  // Dispatch default event queue
  g_WlDisplay->dispatch_pending();
  
  // Forward any events that may have been pushed to our queue
  while (!m_queue.empty())
  {
    // First get event and remove it from the queue, then pass it on - be aware that this
    // function must be reentrant
    XBMC_Event event = m_queue.front();
    m_queue.pop();
    g_application.OnEvent(event);
  }
  
  return true;
}

void CWinEventsWayland::MessagePush(XBMC_Event* ev)
{
  // Currently, this is only called by other Wayland-related code which is in turn
  // called from g_WlDisplay->dispatch_pending() in MessagePump() on the same thread
  // so no locking is necessary
  m_queue.emplace(*ev);
}
