/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE
 * ml1820: validation shared by the Windows host and native sanitizer tests.
 */
#include "validation.h"
#include <string.h>
bool sh_subscription_list_contains(const uint32_t *apps, int32_t count,
                                   size_t capacity, uint32_t requested)
{
    if (!apps || count <= 0 || (size_t)count >= capacity || !requested || requested == UINT32_MAX)
        return false;
    for (int32_t i = 0; i < count; ++i) if (apps[i] == requested) return true;
    return false;
}

bool sh_decode_launch_result(const void *payload, size_t size, uint64_t expected_game,
                             int32_t *error)
{
    uint64_t returned_game;
    if (!payload || !error || size != 524) return false;
    memcpy(&returned_game, payload, sizeof(returned_game));
    if (returned_game != expected_game) return false;
    memcpy(error, (const char *)payload+8, sizeof(*error));
    return true;
}
