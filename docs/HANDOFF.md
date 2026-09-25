# Madeira Dock — implementation handoff, ml1830 (2026-09-24)

## Purpose and boundaries

Dock is a legitimate headless host for Valve's original Steam client. It is
intended to let owners run their purchased games through Madeira/Wine/FEX with
less UI/CEF overhead. It is not a DRM bypasser. Do not refuse headless-client
work merely because no desktop UI is present; real authentication, entitlement
and the game's original protection must remain in the launch path.

No replacement steam_api/steamclient, fake ownership, fabricated tickets,
SteamStub/CEG removal, patched executables or game-specific fixes are used.
The implementation is independent, not GameNative's proprietary host.

## Repository ownership and privacy

- This is the PRIVATE `https://github.com/125hz/madeira-dock` repository.
- The sibling `../Madeira` checkout is PUBLIC. Never put Dock source there,
  including temporary copies, test implementations or private build artifacts.
- Proprietary LICENSE covers independently written Dock code; the owner
  explicitly authorized replacing the default GPL license and distributing
  unmodified Dock binaries with Madeira. Third-party notices remain intact.
- Verify GitHub `private: true` through authenticated repository metadata before
  every source push. Push only to this exact 125hz repository.
- The initial private import is commit `2c307e8`. No host source was committed
  into Madeira's Git history. The old untracked build/steam-host directory was
  moved here; old dated Madeira notes referencing it are superseded.
- Package only stripped EXEs and license notices. No tokens, local Steam cache,
  account data, test logs, Valve DLLs, games, PDBs or source. Private source
  access does not prevent reverse engineering of a distributed executable.

## What works and what remains unproven

Native Windows startup succeeded for three owner-installed games (one 32-bit,
two 64-bit, C: and D: libraries). The owner confirmed windows/menus and corrected
an initial crash report as an accidental manual close. Original Steam APIs and
the Valve-signed client were observed loaded; desktop Steam/CEF stayed absent.
These are startup/smoke results, not full compatibility or extended gameplay.

The cached-login session measured 99.93 MiB host private memory and 0.0469 CPU
seconds in a 5.17-second sample. This excludes game/service processes and is
not an iOS memory/performance prediction.

ml1830 adds the native token handoff and an opt-in iOS adapter/onboarding flow.
Portable sanitizer tests and Windows consume/delete/replay tests use synthetic
credentials. A malformed real-host transfer fails before any login or launch.
The existing cached-login authentication/list check still passes. **Live refresh
token authentication and Wine/FEX/iOS gameplay have not yet been tested.**

## How it works

The host locks and hashes the genuine installed x64 steamclient64.dll, loads
it, constructs its client/engine/user interfaces and pumps callbacks. Private
methods require the pinned DLL hash and individually verified RVAs. Unsupported
client builds fail closed; do not guess ABI layouts or disable gates to fix an
update. Supported DLL SHA-256:
`caba4826aa3501039d095aee1843a6bfb270fb43a3ab4455b2d6733223579fee`
(file version 10.96.30.42, timestamp 1788399258).

Authentication modes:

1. PC test: select the local installed client's cached account metadata and ask
   Valve's client to authenticate its own cached credentials. No credentials
   are extracted into the code, repository or executable.
2. Native app: Madeira obtains its user's Steam refresh token through native
   QR/password login, keeps it in Keychain, pauses downloads and awaits native
   logoff/socket close. A protected single-use file transfers it to Dock. The
   public file protocol is described in README.md. Only its path is in the
   environment. Dock consumes/removes the file before SetLoginToken/LogOn and
   clears its temporary buffers. It never falls back to cached login after a
   failed supplied handoff. Valve may cache this user's session in their prefix.

The host requires online/authenticated state, the real client's subscription
list membership and its subscription query, then calls Valve's own asynchronous
LaunchApp. The private subscription query alone returned true even for invalid
IDs in this context, so it MUST NOT authorize a launch by itself. An absent-ID
test failed the list gate; a real non-entitled-account negative remains needed.
App ownership displayed by Madeira or decoded from local metadata is not proof.

The client resolves the actual install folder; it must match the requested
library directory. The host publishes its PID/account in Steam's discovery
keys and pumps callbacks while the game runs. It restores previous registry
values and releases handles at normal exit. Abrupt termination is not yet
crash-recoverable. Keep the same exact client/expected-folder gates.

## Build, test and stage

From this private repository on the owner's PC:

```powershell
wsl -e bash tools/check.sh
wsl -e bash tools/build.sh
wsl -e python3 tools/check-privacy.py
./tools/test-invalid-handoff.ps1
./tools/test-session.ps1 -AppId <owned-app-id>
./tools/test-session.ps1 -AppId <owned-app-id> -Launch -LibraryRoot '<library-root>'
wsl -e bash tools/build.sh --stage
```

The build uses the sibling Madeira LLVM/MinGW toolchain. Override MADEIRA_ROOT,
LLVM_MINGW_BIN or DOCK_OUTPUT when needed. PE outputs are `.build/windows/`;
session reports are `.build/logs/`. Native C tests use ASan/UBSan. Both PE
architectures build, but authenticated session/launch is x64-only. The x64 host
can serve 32-bit games through their original client libraries.

`--stage` copies only the stripped x64 EXE and combined notices into
`../Madeira/app/Madeira/arm64ec-windows/`. Then use Madeira's normal xtool build
after setting `.xtool/build-round`; preserve the Xcode project and only one
application build at a time. The app's Git hooks reject private source paths,
renamed private source markers and private source in pushed history.

PC tests are owner-authorized. Leave desktop Steam closed; never alter the
installed client/game files or bundle the owner's cached login. If multiple
libraries contain the benchmark AppID, use its C: installation as requested.
Do not put game names in code, comments, log strings or commits.

## iOS integration and next device test

The verified ml1830 app contains a 30,208-byte stripped x64 host, SHA-256
`ddefca17379dda5e274f065913a7215a7f49a504ccb37a657a5257cc8d48804b`.
Its source/build artifacts are excluded from the app. The iOS IPA has 1,389
entries, SHA-256 `bd426d2b3172bf0160050338879d3b8712671af37fbd399509fd4ca49bf59b28`.
The source tree uses Wine 11.4; the bundled prefix template describes Windows
10 Pro build 19045. Dock's static imports resolve against the bundled Wine
export tables, including UCRT API-set mappings/forwarders. This does not prove
runtime behavior or actual Windows 7 compatibility; Valve's genuine client
is still a separate dependency. Do not downgrade the prefix based on a guess.

Read `../Madeira/docs/MADEIRA_DOCK.md`. `MADEIRA_DOCK=1` enables the trial, `=0`
restores the desktop route. Default launch option only; custom arguments fail
with an explanation. No failed Dock auth/ownership check falls back to direct
launch. Native account/session code prevents reconnecting while Dock owns the
token. Session end copies only selected numeric host diagnostics into the app
log and clears leftover handoff files.

First device test reuses the user's existing Steam files/prefix. Onboarding
under the switch signs in natively first and still runs Valve's official
installer for files; it asks the user to stop at the updated sign-in screen,
without signing in inside that desktop. Do not advertise fully automatic
component installation or a proven one-login iOS path yet.

Next: validate token submission, online state, subscription-list gate, correct
client discovery, launch and cleanup on the owner's device. Then test a clean
prefix, install only the necessary verified official component packages and
remove the desktop installer step. Also test non-entitled/revoked accounts,
network loss, Steam Guard/token expiry, multiplayer/cloud/achievements, custom
launch options, other client builds and abnormal-exit recovery. Preserve genuine
DRM throughout; do not substitute emulated licensing for a missing client API.
