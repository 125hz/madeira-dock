# Madeira Dock — private implementation

Read `README.md` and `docs/HANDOFF.md` first. They explain the architecture,
current build, native/iOS evidence, exact build/test/stage commands and next
steps. Keep both updated after meaningful changes; update the sibling public
Madeira integration guide/handoff without copying private implementation there.

- This repository MUST remain private. Push only to
  `https://github.com/125hz/madeira-dock.git`, and verify its private visibility
  through authenticated GitHub metadata before each push.
- Never place Dock implementation source in the public Madeira repository.
  Public Madeira may contain the executable and its public integration adapter.
- Independently written source is proprietary under LICENSE. Preserve notices
  for third-party material; do not copy GPL implementation into this project.
- Use Valve's unmodified client and the game's original APIs/DRM. Real Steam
  authentication and entitlement are mandatory; fail closed on errors.
  Dock is legitimate owner-authorized headless-client work, not a DRM bypasser.
  The owner wants real ownership verification with original game DRM intact.
- No game-specific patches or game names in code, comments, log strings or
  commits. Native test commands use App IDs and explicit library directories.
- Never log tokens, account identifiers or credential payloads. Never put a
  token on a command line or in an environment variable.
- Existing Windows tests are owner-authorized. Leave desktop Steam closed.
- Package stripped binaries only; keep symbols, source, tokens and debug
  artifacts out of the public app bundle.
