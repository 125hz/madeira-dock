# Third-party references and components

Dock's implementation is independently written. No Steamworks emulator,
GameNative host binary, Valve client binary, game binary or reference-project
implementation is included in this repository.

API declarations and architecture were researched using OpenSteamworks
(commit d0abbe85f4b8314607836e1c588bb48813f642a4), Valve's public Steamworks
documentation and independently inspected, unmodified installed client files.
Reference interface layouts were checked against the exact supported client;
no copied implementation was relicensed.

Builds use LLVM/MinGW. Their licenses and runtime notices continue to apply;
they are separate from Dock's proprietary source license. Windows system DLLs
are provided by Windows or the Wine environment. Valve's client components
are obtained separately under Valve's terms and are not covered by Dock's
license. GameNative's proprietary host was not used.

Runtime license texts accompanying executable releases are retained in
`notices/MinGW-w64-runtime.txt` and `notices/LLVM.txt`. The stage command
includes these with the proprietary binary permission in dock-notices.txt.
