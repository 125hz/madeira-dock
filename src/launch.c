/* SPDX-License-Identifier: LicenseRef-Madeira-Dock-Proprietary
 * MADEIRA_DOCK_PRIVATE_SOURCE
 * ml1820: launch through Valve's real app manager after authenticated
 * entitlement checks. The installed game, API DLLs and DRM remain unchanged.
 */
#include "launch.h"
#include "validation.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN64
typedef void *(__thiscall *get_manager_fn)(void *, int32_t, int32_t);
typedef int32_t (__thiscall *install_dir_fn)(void *, uint32_t, char *, int32_t);
typedef uint64_t (__thiscall *launch_fn)(void *, const uint64_t *, uint32_t, int32_t, const char *);
typedef bool (__thiscall *running_fn)(void *, const uint64_t *);
typedef bool (__thiscall *call_result_fn)(void *, int32_t, uint64_t, void *, int32_t, int32_t, bool *);

static bool method_is(HMODULE module, void *object, unsigned slot, uintptr_t rva)
{
    return object && (*(void ***)object)[slot] == (void *)((uintptr_t)module + rva);
}

struct saved_value { HKEY key; const wchar_t *name; DWORD previous, written; bool existed, changed; };
static bool publish(struct saved_value *v, DWORD value)
{
    DWORD type = 0, size = sizeof(DWORD);
    LSTATUS status = RegQueryValueExW(v->key, v->name, NULL, &type, (BYTE *)&v->previous, &size);
    v->existed = status == ERROR_SUCCESS;
    if ((v->existed && (type != REG_DWORD || size != sizeof(DWORD))) ||
        (!v->existed && status != ERROR_FILE_NOT_FOUND)) return false;
    v->written = value;
    if (RegSetValueExW(v->key, v->name, 0, REG_DWORD, (BYTE *)&value, sizeof(value)) != ERROR_SUCCESS) return false;
    v->changed = true;
    return true;
}

static bool restore(struct saved_value *v)
{
    if (!v->changed) return true;
    DWORD current = 0, size = sizeof(current), type = 0;
    if (RegQueryValueExW(v->key, v->name, NULL, &type, (BYTE *)&current, &size) != ERROR_SUCCESS ||
        type != REG_DWORD || current != v->written) return false;
    LSTATUS status = v->existed ?
        RegSetValueExW(v->key, v->name, 0, REG_DWORD, (BYTE *)&v->previous, sizeof(DWORD)) :
        RegDeleteValueW(v->key, v->name);
    return status == ERROR_SUCCESS;
}

static volatile LONG interrupted;
static BOOL WINAPI on_control(DWORD control)
{
    if (control != CTRL_C_EVENT && control != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&interrupted, 1);
    return TRUE;
}

int sh_launch(HMODULE module, void *engine, void *client_user,
              const struct sh_api *api, const struct sh_observer *o,
              int32_t pipe, int32_t user, uint64_t steamid, uint32_t appid)
{
    if (!method_is(module, engine, 43, 0x970d30) ||
        !method_is(module, engine, 33, 0x970ba0) ||
        !method_is(module, client_user, 67, 0x7324e0)) return 40;
    void *manager = ((get_manager_fn)(*(void ***)engine)[43])(engine, user, pipe);
    if (!method_is(module, manager, 2, 0x83f0f0) ||
        !method_is(module, manager, 5, 0x758760)) return 40;
    static char actual_utf8[32768];
    static wchar_t actual[32768], expected[32768], normalized[32768];
    DWORD length = GetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_EXPECTED_INSTALL", expected, 32768);
    if (!length || length >= 32768) return 41;
    int32_t got = ((install_dir_fn)(*(void ***)manager)[5])(manager, appid, actual_utf8, sizeof(actual_utf8));
    if (got <= 0 || got >= (int32_t)sizeof(actual_utf8) ||
        !memchr(actual_utf8, 0, sizeof(actual_utf8)) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, actual_utf8, -1, actual, 32768)) return 41;
    length = GetFullPathNameW(actual, 32768, normalized, NULL);
    if (!length || length >= 32768 || _wcsicmp(normalized, expected)) {
        o->event("launch-install-directory-mismatch", 1);
        return 41;
    }
    o->event("launch-install-directory-verified", 1);
    HKEY active = NULL, machine = NULL;
    int result = 42;
    struct saved_value values[3] = {0};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0,
                     KEY_QUERY_VALUE | KEY_SET_VALUE, &active) != ERROR_SUCCESS ||
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Valve\\Steam", 0,
                     KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_WOW64_32KEY, &machine) != ERROR_SUCCESS) goto done;
    values[0] = (struct saved_value){.key=active, .name=L"pid"};
    values[1] = (struct saved_value){.key=active, .name=L"ActiveUser"};
    values[2] = (struct saved_value){.key=machine, .name=L"SteamPID"};
    if (!publish(&values[0], GetCurrentProcessId()) ||
        !publish(&values[1], (DWORD)steamid) || !publish(&values[2], GetCurrentProcessId())) goto done;
    o->event("launch-client-discovery-published", 1);
    SetConsoleCtrlHandler(on_control, TRUE);
    uint64_t gameid = appid;
    uint64_t call = ((launch_fn)(*(void ***)manager)[2])(manager, &gameid, 0, 0, "");
    o->event("launch-request-submitted", call != 0);
    if (!call) { result = 43; goto done; }
    uint64_t begin = o->now_ms(), stopped_at = 0;
    bool seen_running = false, result_received = false, result_rejected = false;
    result = 44;
    while (!InterlockedCompareExchange(&interrupted, 0, 0)) {
        for (unsigned batch = 0; batch < 64; ++batch) {
            struct sh_callback cb = {0};
            if (!api->get_callback(pipe, &cb)) break;
            bool valid = cb.id > 0 && cb.size >= 0 && (!cb.size || cb.data);
            if (valid && cb.id == 703 && cb.size >= 16) {
                uint64_t finished; int32_t kind; uint32_t size;
                memcpy(&finished, cb.data, 8);
                memcpy(&kind, (char *)cb.data+8, 4);
                memcpy(&size, (char *)cb.data+12, 4);
                if (finished == call) {
                    /* Do not print the error-detail string: it can contain
                     * private paths. Only decode the fixed numeric result.
                     */
                    /* This client reports 524 bytes: 8-byte GameID, 4-byte
                     * EAppError and 512 detail bytes, without tail padding.
                     */
                    unsigned char payload[524] = {0};
                    bool failed = false;
                    bool read = kind == 1270027 && size == sizeof(payload) &&
                        ((call_result_fn)(*(void ***)engine)[33])(
                            engine, pipe, call, payload, sizeof(payload), kind, &failed);
                    int32_t error = -1;
                    bool decoded = read && !failed && sh_decode_launch_result(payload, size, gameid, &error);
                    o->event("launch-result-kind", kind);
                    o->event("launch-result-size", (int32_t)size);
                    o->event("launch-client-error", error);
                    result_received = decoded && error == 0;
                    result_rejected = !result_received;
                    /* A game can start before this callback is decoded. Keep
                     * serving it until exit even if a result is rejected.
                     */
                }
            }
            api->free_callback(pipe);
            if (!valid) { result = SH_CALLBACK_INVALID; goto done; }
        }
        bool running = ((running_fn)(*(void ***)client_user)[67])(client_user, &gameid);
        if (running && !seen_running) {
            o->event("launch-game-running", 1);
            seen_running = true;
        }
        if (running) stopped_at = 0;
        else if (seen_running) {
            if (!stopped_at) stopped_at = o->now_ms();
            if (o->now_ms() - stopped_at >= 3000) {
                o->event("launch-game-ended", 1);
                result = result_received && !result_rejected ? 0 : 46;
                break;
            }
        }
        if (!seen_running && result_rejected && o->now_ms() - begin > 5000) { result = 45; break; }
        if (!seen_running && o->now_ms() - begin > 90000) break;
        o->sleep_ms(50);
    }
done:
    SetConsoleCtrlHandler(on_control, FALSE);
    bool restored = true;
    /* Another Steam process may have claimed registration while this host was
     * alive. Never restore account metadata over a different current PID.
     */
    if (values[0].changed) {
        DWORD current = 0, bytes = sizeof(current), type = 0;
        if (RegQueryValueExW(active, L"pid", NULL, &type, (BYTE *)&current, &bytes) != ERROR_SUCCESS ||
            type != REG_DWORD || current != GetCurrentProcessId()) restored = false;
    }
    if (restored)
        for (int i = 2; i >= 0; --i) if (!restore(&values[i])) restored = false;
    o->event("launch-discovery-restored", restored);
    if (machine) RegCloseKey(machine);
    if (active) RegCloseKey(active);
    if (!restored) result = 47;
    o->event("launch-host-result", result);
    return result;
}
#else
int sh_launch(HMODULE module, void *engine, void *client_user,
              const struct sh_api *api, const struct sh_observer *o,
              int32_t pipe, int32_t user, uint64_t steamid, uint32_t appid)
{
    (void)module; (void)engine; (void)client_user; (void)api; (void)o;
    (void)pipe; (void)user; (void)steamid; (void)appid;
    return 40;
}
#endif
