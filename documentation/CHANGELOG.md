# IMPORTANT NOTE

This fork is dependent on the changes on the fork [jcmoyer/Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55)
For **FULL CHANGELOG** from please check [jcmoyer/Nuked-SC55/CHANGELOG.md](https://github.com/jcmoyer/Nuked-SC55/blob/master/CHANGELOG.md)

# Version 0.5.1 (2025-04-01)

- Synced to changes from [jcmoyer/Nuked-SC55#9842310](https://github.com/jcmoyer/Nuked-SC55/commit/984231094fcb977af9f22a7c30e129ee8afc7914)

Jist of changes are follows:
- Added ASIO Supported builds (Currently on SDL builds for Windows are released)
- Rework to remove SDL dependency for backend and renderer.
- Update to handle Non-English paths in Windows
- Some code cleanup

# Version 0.4.1 (2025-02-17)

- Synced to changes from [jcmoyer/Nuked-SC55#9f43062](https://github.com/jcmoyer/Nuked-SC55/commit/9f4306240f8edab806ceaced3917cd18e9412642)

Jist of changes are follows:
- Made frontend run independently of emulator, therefore the frontend will be responsive even if emulator falls behind.
- Fixed bug with f32 wav rendering. (Previously wav files had long periods of silence in some players)
- renderer ignores extra data at the end if MIDI files have them after end-of-track event.
- The renderer will no longer ask the user to submit a bug report when the instance count is set to a number that results in only some instances having a midi track to render.

# Version 0.3.2 (2024-11-09)

- Improved Windows MIDI Handling code
- Updated JV880 LCD contrast code
- Added midi out callback to handle MIDI/SysEx out.
- Updated renderer code

# Version 0.3.2 (2024-11-09)

- Minor rework of Code
- Volume adjustment works properly
- Updated to add and fix renderer

# Version 0.3.1 (2024-11-05)

Initial release