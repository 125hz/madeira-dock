/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE */
#ifndef MADEIRA_STEAM_HOST_SESSION_H
#define MADEIRA_STEAM_HOST_SESSION_H
#include <windows.h>
#include "probe.h"
#include "client_layout.h"
int sh_session(HMODULE module, void *engine, const struct sh_api *api,
               const struct sh_observer *observer, const struct dock_client_layout *layout);
#endif
