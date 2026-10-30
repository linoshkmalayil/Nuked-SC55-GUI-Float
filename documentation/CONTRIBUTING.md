# Contributing

Pull requests and issues are welcome. Before submitting a pull request, please
read the following sections entirely.

## Overview

One of the main features of this fork is to separate the upstream gui
application into two parts. The part that does the hardware emulation is the
backend, and the part that presents the emulator's output to the user is the
frontend.

### Backend

The backend is the emulator itself. These are generally the source files under
`src/backend`.

Currently the original [upstream](https://github.com/nukeykt/Nuked-SC55) is 
now read only. Enhancements to the backend should be made with careful 
consideration so as to keep the emulation faithful to original NukeYKT's 
vision which is to be feature parity with Roland-SC55 and related devices.

Optimizations to the backend are accepted as long as they can be shown to be
equivalent to the current behavior. The integration tests are designed to help
with this. See the [Development section of BUILDING](BUILDING.md#development)
for more information.

### Frontend

There are currently two frontends:

1. The standard frontend, a more feature-rich version of upstream's gui
   application
2. The renderer frontend, a command line tool for rendering MIDI files to WAVE
   files

It is permissible to add quality-of-life features to both of these.

## General advice

When submitting a PR please make the diff as clean as possible. Do not add
extra lines or whitespace. Small "fix minor thing in the previous commit" type
commits should be squashed. Follow the convention (naming, brace styles,
indentation) of surrounding code.

## LLM Contributions

LLM usage must be disclosed in pull requests. Because LLM generated
code currently has a dubious copyright status in much of the world,
LLM authored contributions will be rejected.

If you are determined to have used an LLM and have knowingly
misrepresented the authorship of a PR or commit, you will be banned
from the project.
