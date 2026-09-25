/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE */
#ifndef MADEIRA_STEAM_HOST_VALIDATION_H
#define MADEIRA_STEAM_HOST_VALIDATION_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
bool sh_subscription_list_contains(const uint32_t *apps, int32_t count,
                                   size_t capacity, uint32_t requested);
bool sh_decode_launch_result(const void *payload, size_t size, uint64_t expected_game,
                             int32_t *error);
#endif
