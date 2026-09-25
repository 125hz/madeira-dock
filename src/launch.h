/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE */
#ifndef MADEIRA_STEAM_HOST_LAUNCH_H
#define MADEIRA_STEAM_HOST_LAUNCH_H
#include "session.h"
int sh_launch(HMODULE module, void *engine, void *client_user,
              const struct sh_api *api, const struct sh_observer *o,
              int32_t pipe, int32_t user, uint64_t steamid, uint32_t appid);
#endif
