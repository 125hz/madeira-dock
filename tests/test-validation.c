/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE */
#include "validation.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void)
{
    uint32_t apps[] = {17, 29, 44, 0};
    assert(sh_subscription_list_contains(apps, 3, 4, 29));
    assert(!sh_subscription_list_contains(apps, 3, 4, 30));
    assert(!sh_subscription_list_contains(NULL, 3, 4, 29));
    assert(!sh_subscription_list_contains(apps, -1, 4, 29));
    assert(!sh_subscription_list_contains(apps, 0, 4, 29));
    assert(!sh_subscription_list_contains(apps, 4, 4, 29));
    assert(!sh_subscription_list_contains(apps, INT32_MAX, 4, 29));
    assert(!sh_subscription_list_contains(apps, 3, 4, 0));
    assert(!sh_subscription_list_contains(apps, 3, 4, UINT32_MAX));
    unsigned char payload[524] = {0};
    uint64_t gameid = 29;
    int32_t error = -1, denied = 5;
    memcpy(payload, &gameid, 8);
    assert(sh_decode_launch_result(payload, sizeof(payload), gameid, &error) && error == 0);
    memcpy(payload+8, &denied, 4);
    assert(sh_decode_launch_result(payload, sizeof(payload), gameid, &error) && error == denied);
    assert(!sh_decode_launch_result(payload, sizeof(payload), gameid+1, &error));
    assert(!sh_decode_launch_result(payload, 12, gameid, &error));
    assert(!sh_decode_launch_result(payload, 528, gameid, &error));
    assert(!sh_decode_launch_result(NULL, 524, gameid, &error));
    assert(!sh_decode_launch_result(payload, 524, gameid, NULL));
    puts("steam-host: 16 entitlement-list and launch-result validation cases passed");
}
