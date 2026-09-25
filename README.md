# Madeira Dock (private)

Round ml1830, Windows proof of concept and initial iOS integration. This executable loads the user's
installed, unmodified Valve client. It does not replace Steam APIs or remove
game protection. No Valve binaries are included in this source directory.

## Current operation

The original five-second bootstrap remains available. The opt-in session mode
can now select the existing cached Steam account, ask Valve to authenticate it,
read subscriptions through the authenticated client and submit an installed
game to Valve's own `IClientAppManager::LaunchApp`. It pumps callbacks while
the game runs and restores the process-discovery registry values afterward.

The owner confirmed successful startup for three installed games: one 32-bit
game on C:, one 64-bit Steamworks game on C:, and another on D:. Desktop
`steam.exe` and `steamwebhelper.exe` remained absent. An initially reported
menu crash did not reproduce; the owner confirmed the retry worked and said
they likely closed the first attempt accidentally. This is startup/smoke
validation, not extended gameplay coverage. A host exit of zero records host
lifecycle success. Wine/iOS and live native-token authentication have not been
tested on a device. The handoff parser and Windows file consumption are tested
with synthetic credentials; those tests do not claim successful authentication.

## Private source and distribution

This repository must remain **private**. Its independently written source is
proprietary, and unmodified executable releases may accompany Madeira under
LICENSE. The public app contains the executable, license notices and a public
launch adapter only. No source, debug symbols, local Steam login, account data,
client configuration, Valve binaries, games or test logs may be uploaded.
Check repository visibility before every push. Executables remain inspectable;
private source access is not a guarantee against reverse engineering.

## Native sign-in handoff (ml1830)

The app's QR/password sign-in obtains its user's refresh token from Steam and
stores it in Keychain. The app pauses downloads, disables native reconnection,
awaits Steam logoff/socket close, then writes a one-use transfer in protected
Application Support storage. Only that path enters the guest environment.
The native app's JWT-subject parsing selects an account; it is not trusted as
proof of identity or ownership. The real client must accept the token online.

`MADEIRA_DOCK_AUTH_FILE` selects this route. Missing, locked, malformed or
mismatched handoffs fail; they never fall back to cached PC credentials.
The file is opened exclusively with delete-on-close before any login call.
The host clears its temporary buffers and submits the token to the exact
client's verified SetLoginToken method. Valve's LogOn, authenticated state,
subscription list and original game launch/DRM checks still gate execution.
Valve may maintain its own login cache inside the user's Wine prefix.

Wire version `MDOCK001`: 8 magic bytes; little-endian u64 SteamID, u32 AppID,
u16 account length, u16 token length; account bytes then token bytes, no NULs
or trailing data. Account length 1–64, token length 1–8192. No password or
ownership assertions are present. A transfer is scoped to one requested AppID.

The iOS trial is off until `MADEIRA_DOCK=1` is set in madeira-env.txt. It still
uses Valve's official installer to prepare client files, requires the pinned
client version, and supports the default launch option without custom args.
One native sign-in is intended to replace the desktop sign-in. Removing the
installer, clean-prefix support and authenticated device launches are pending
device evidence. `MADEIRA_DOCK=0` restores the existing desktop launch route.

**Private ABI support is limited to one independently inspected client build:**

- x64 SHA-256: `caba4826aa3501039d095aee1843a6bfb270fb43a3ab4455b2d6733223579fee`
- File version: 10.96.30.42; PE timestamp: 1788399258.
- The installed DLL's Authenticode signature was verified as Valve Corp.
- The host checks the SHA-256 and the selected method RVAs before private calls.
- Client updates require a new verified adapter. Unsupported files fail closed.

This is an internal experiment, not a supported Valve embedding SDK. Do not
ship the pinned Windows adapter as a universal solution to private ABI drift.

## Native Windows test

Exit desktop Steam normally first. From PowerShell in the repository root:

```powershell
# Sign in with the existing cached account and check an App ID, without launching.
./tools/test-session.ps1 -AppId <owned-app-id>

# Launch from an explicitly selected existing Steam library.
./tools/test-session.ps1 -AppId <owned-app-id> -Launch -LibraryRoot '<Steam-library-root>'
```

The root is the folder containing `steamapps`. The launcher reads that library's
manifest and requires the real client's resolved installation directory to
match it exactly. This avoids accidentally testing a second/old installation.
There are no game-specific paths or fixes in the host.

The script chooses the sole cached account, or the one explicitly marked most
recent when several exist. It fails on ambiguity. It does not read credential
tokens, print account identifiers, modify game executables or call APIs that
invalidate cached credentials. Account metadata is passed in the child
environment and the script restores its previous environment afterward.

The host uses the standard installed Steam service and supporting client files.
Its memory figure therefore does not include all system-wide Steam components.
During one authenticated game session, a 5.17-second sample measured 99.93 MiB
host private memory and 0.0469 seconds of host CPU time. These are native Windows
measurements, not predictions of Wine/FEX memory use or device frame rate.
Close the game normally to end the session. Desktop Steam remains closed.
The report is `.build/logs/session.txt`; copy it before the next test.

Current EXE: `.build/windows/dockhost-x86_64.exe`.
Double-clicking does nothing useful because the master opt-in defaults off.
The 32-bit EXE supports the bootstrap only; session/launch requires the x64 host.

## Gates and observations

- `MADEIRA_STEAM_HOST_PROBE=1`: master opt-in.
- `MADEIRA_STEAM_HOST_BOOTSTRAP=1`: original five-second export/bootstrap test.
- `MADEIRA_STEAM_HOST_SESSION=1`: version-gated private interface test.
- `MADEIRA_STEAM_HOST_LOGIN=1`: genuine cached authentication.
- `MADEIRA_STEAM_HOST_LAUNCH=1`: launch only after authentication and entitlement.
- Account, App ID, expected installation and report path are set by the script.
- No fallback launches a game after failed authentication or entitlement.

Authentication requires `Steam_BLoggedOn`, `IClientUser::BLoggedOn` and
`BConnected`. The requested App ID must also appear in the client-provided
subscription list and pass its subscription query. The standalone private
`BIsSubscribedApp` query returned true for invalid IDs in this host context;
its semantics remain unresolved and it is deliberately not sufficient to
authorize launch. An absent App ID was rejected by the list gate. That test
does not substitute for a future real-game test using a non-entitled account.
The game's original DRM and Valve's own launch checks remain in place.

`LaunchApp` returns an asynchronous result. This exact DLL emits callback
1270027 with **524** bytes, not the 528 bytes suggested by padded reference
structs. The first trial started a child but rejected the result length;
the corrected trial retained the host until game exit. Result validation now
has a regression test, and a rejected result no longer immediately abandons a
game that is already running.

The host temporarily publishes its real PID and authenticated account ID in
Steam's discovery keys. Normal exit restores the original values. Abrupt host
termination is not yet crash-recoverable. Do not run a second Steam session
concurrently. The native control handler allows cleanup on Ctrl+C/Ctrl+Break,
but terminating the host while playing necessarily removes its client service.

Known native diagnostics include failed overlay/injection-helper setup and
socket binding warnings. No patch disables those checks. The helper warning
did not prevent the first 32-bit game from reaching a visible window. Overlay,
cloud, achievements, multiplayer, CEG preparation, connection-loss recovery,
refunded/revoked licences and full gameplay remain separate validation work.

## Build and checks

```powershell
wsl -e bash tools/check.sh
wsl -e bash tools/build.sh
```

The native ASan/UBSan tests cover bootstrap cleanup, malformed callbacks,
callback fairness/time bounds, subscription-list bounds and exact launch-result
decoding. They do not emulate a successful Steam server response.

`tools/build.sh --stage` copies the stripped x64 EXE and required license
notices into Madeira's resource directory. Source remains in this private repo.
The public app routes games to Dock only with its explicit trial switch enabled.
`python3 tools/check-privacy.py` tests the public repository's source-leak hooks
using an isolated disposable Git repository.

## References and provenance

The C implementation is independent; no reference library was vendored.
Signatures and architecture were checked against
[OpenSteamworks](https://github.com/OpenSteamClient/OpenSteamworks/tree/d0abbe85f4b8314607836e1c588bb48813f642a4),
and selected methods were confirmed by RTTI/IPC strings and call signatures in
the exact installed DLL. The reference generated headers are not treated as
current binary layouts. Public shutdown slot 23 was checked against
[Valve's SteamClient021 header](https://github.com/ValveSoftware/Proton/blob/proton_10.0/lsteamclient/steamworks_sdk_162/isteamclient.h).

[Valve's API overview](https://partner.steamgames.com/doc/sdk/api) and
[DRM documentation](https://partner.steamgames.com/doc/features/drm) describe the
game-facing client requirements. GameNative's proprietary steamhost binaries
were not reused; see its
[third-party notices](https://github.com/utkarshdalal/GameNative/blob/master/THIRD_PARTY_NOTICES).
