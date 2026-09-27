/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 125hz
 * Madeira Converter Exception: see LICENSE-EXCEPTION.md */
#ifndef MADEIRA_DOCK_CLIENT_LAYOUT_H
#define MADEIRA_DOCK_CLIENT_LAYOUT_H
#include <stdint.h>
#include <stdbool.h>
struct dock_client_layout {
    const char *sha256;
    int revision;
    uintptr_t engine_user, engine_result, engine_manager;
    uintptr_t logon, logged_on, connected, cached, select_account, token;
    uintptr_t running, subscribed, subscriptions, launch, install_dir;
    /* ml1990: IClientUserMap slot 71, RequestCustomBinaries (see CLIENT_LAYOUTS.md). */
    uintptr_t ceg_request;
};
const struct dock_client_layout *dock_client_layout(const char *sha256);
bool dock_method_is(uintptr_t module, void *object, unsigned slot, uintptr_t rva);
#endif
