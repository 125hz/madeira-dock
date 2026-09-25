/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE */
#ifndef MADEIRA_DOCK_AUTH_H
#define MADEIRA_DOCK_AUTH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define DOCK_AUTH_MAX (24 + 64 + 8192)
struct dock_auth {
    uint64_t steam_id;
    uint32_t app_id;
    char account[65];
    char token[8193];
};
/* This envelope transports a credential, never an entitlement assertion.
 * Authentication and ownership still require Valve's online response. */
bool dock_auth_parse(const unsigned char *bytes, size_t size, struct dock_auth *out);
void dock_auth_clear(void *bytes, size_t size);
#ifdef _WIN32
bool dock_auth_consume(const wchar_t *path, struct dock_auth *out);
#endif
#endif
