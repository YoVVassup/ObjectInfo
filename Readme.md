<p align="center">
  <img src="https://img.shields.io/badge/Version-0.0.3.0-green" alt="Version"/>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus" alt="C++20"/>
  <img src="https://img.shields.io/badge/Platform-Win32-blue?logo=windows" alt="Win32"/>
  <img src="https://img.shields.io/badge/YRpp-Phobos-green" alt="YRpp"/>
  <img src="https://img.shields.io/badge/Sringe-0.7-orange" alt="Syringe"/>
  <img src="https://img.shields.io/badge/Visual%20Studio-2022-purple?logo=visualstudio" alt="Visual Studio"/>
  <img src="https://img.shields.io/badge/Game-RA2%20YR%201.001-red" alt="RA2 YR"/>
  <img src="https://img.shields.io/badge/License-GPL%20v3-blue.svg" alt="GPL v3"/>
</p>

<h1 align="center">ObjectInfo</h1>

<p align="center">
  Debug overlay plugin for <b>Red Alert 2: Yuri's Revenge</b><br/>
  Real-time object inspection and interactive trigger debugger
</p>

---

## Overview

ObjectInfo is a Syringe DLL plugin that adds an interactive debug overlay for inspecting game objects and triggers in real-time. Originally created by **Handama** for the [RA2DIY](https://bbs.ra2diy.com/forum.php?mod=viewthread&tid=24561) community.

### Features

- **Object Info Overlay** — hover over any unit/building to see detailed debug data
- **Trigger Debug Mode** — interactive UI to inspect, enable, disable, and force-execute triggers
- **AI Trigger Debug Mode** — inspect AI Trigger Types decision-making for each house
- **Configurable Presets** — define which fields to display via INI config
- **Trigger Dump** — export all trigger data to `debug.log`
- **Version Info** — DLL includes embedded Windows VERSIONINFO resource

## Installation

**With Syringe (Ares/Phobos mods):**

Copy `ObjectInfo.dll` and `objectinfo.ini` into the game directory (next to `gamemd.exe`). Syringe automatically discovers and loads all DLLs in the root folder.

**Without Syringe:**

Use an external DLL injector to load `ObjectInfo.dll` into the `gamemd.exe` process.

## Configuration — objectinfo.ini

The INI file contains full documentation of all available fields as comments.

### `[ObjectInfoDisplayLists]`

Defines display presets. Each key is a separate preset, cycled with the "Next Info Preset" hotkey:

```ini
; Preset 0: Full debug info
0=uiname,id,uid,hp,owner,location,power,factory,money,occupants,enemy,upgrades,passenger,aitrigger,team,currentscript,target,destination,focus,ammo,currentmission,group,recruit,veterancy,tag,megamission,megatarget,megadestination
; Preset 1: Minimal gameplay info
1=id,hp,location,target,destination,focus
; Preset 2: Name only
2=uiname
```

- Values are comma-separated field names
- Use `NONEALL` to display all fields
- Empty section = show all fields

### `[ObjectInfoDisplayOffset]`

Pixel offset for the overlay position:

```ini
[ObjectInfoDisplayOffset]
X=0
Y=15
```

### Panel Position

Configure panel positions for Trigger Debug and AI Trigger Debug:

```ini
[TriggerDebugPosition]
X=10
Y=180

[AITriggerDebugPosition]
X=-420
Y=180
```

Negative X values position from the right edge of the screen.

## Hotkeys

Assign these in the game's keyboard options:

| Command | Description | Availability |
|---------|-------------|--------------|
| **Display Object Info** | Toggle the object info overlay | Release + Debug |
| **Next Info Preset** | Cycle through display presets | Release + Debug |
| **Dump Trigger Info** | Export all trigger data to `debug.log` | Release + Debug |
| **Trigger Debug Mode** | Toggle the interactive trigger debugger | Release + Debug |
| **Trigger Debug Page Up** | Scroll up in the trigger list | Release + Debug |
| **Trigger Debug Page Down** | Scroll down in the trigger list | Release + Debug |
| **AI Trigger Debug Mode** | Toggle the AI Trigger Types decision-making panel | Release + Debug |
| **AI Trigger Debug Page Up** | Scroll up in the AI trigger list | Release + Debug |
| **AI Trigger Debug Page Down** | Scroll down in the AI trigger list | Release + Debug |

> **Note:** All commands are available in both Release and Debug builds.

## Displayed Fields

### Basic

| Field | Output Format |
|-------|---------------|
| `uiname` | Localized UI name (with MISSING fallback) |
| `id` | `ID = <TypeID>` |
| `uid` | `UID = <UniqueID>` |
| `hp` | `HP = (current / max)` |
| `owner` | `Owner = <ID> (<PlainName>)` |
| `location` | `Location = (X, Y)` |
| `group` | `Group = N` |
| `tag` | `Tag = <ID>, InstanceCount = N` + nested triggers |

### Combat

| Field | Output Format |
|-------|---------------|
| `ammo` | `Ammo = (current / max)` |
| `currentmission` | `Current Mission = N (name)` |
| `veterancy` | `Veterancy = Rookie/Veteran/Elite (N.xx)` |
| `recruit` | `RecruitA = N, RecruitB = N` |

### Movement / Targeting

| Field | Output Format |
|-------|---------------|
| `link` | `Link N: UID = .., ID = .., Location = (X, Y)` |
| `target` | `Target = <ID>, Distance = N, Location = (X, Y)` |
| `destination` | `Destination = <ID>, Distance = N, Location = (X, Y)` or `Destination = (X, Y)` |
| `focus` | `Focus = <ID>, Distance = N, Location = (X, Y)` |
| `megamission` | `Mega Mission = N (name)` |
| `megatarget` | `Mega Target = <ID>, Distance = N, Location = (X, Y)` |
| `megadestination` | `Mega Destination = <ID>, Distance = N, Location = (X, Y)` |

### Infantry / Vehicle

| Field | Output Format |
|-------|---------------|
| `passenger` | `N Passengers: <ID>, <ID>, ...` |

### Building

| Field | Output Format |
|-------|---------------|
| `power` | `Power: N, Total Power Output: N, Total Power Drain: N` |
| `factory` | `Production: <ID> (N%)` |
| `money` | `Money: N` |
| `occupants` | `N Occupants: <ID>, <ID>, ...` |
| `enemy` | `Enemy = <ID> (<Name>), AngerLevel = N` |
| `upgrades` | `Upgrades (N / Max): Slot 1 = <ID>, Slot 2 = .., Slot 3 = ..` |

### Team / AI

| Field | Output Format |
|-------|---------------|
| `aitrigger` | `Trigger ID = .., weights [Current, Min, Max]: .., .., ..` |
| `team` | `Team ID = .., Script ID = .., Taskforce ID = ..` |
| `currentscript` | Current script line or missing units |

## Trigger Debug Mode

Interactive UI for inspecting and controlling triggers in real-time.

### Display

**Basic mode:**
- Trigger ID and name

**Detailed mode:**
- `Frame Left(s): N(Ns)` — remaining time
- `(Modified)` — if timer was manually changed
- `Last Executed Frame(s): N(Ns)` — last execution time
- `Execute Count: N` — total executions
- `Conditions: N[@]/N[  ]` — condition states

**Destroyed triggers:**
- `<Expired> <ID> (<Name>)` with destruction timestamp

### Color Indication

| Color | Meaning |
|-------|---------|
| Green | Active trigger (enabled and executed at least once) |
| Gray | Not yet activated (enabled but never executed) |
| Red | Disabled trigger or expired trigger |

### UI Buttons

| Button | Action |
|--------|--------|
| Page Up/Down | Navigate pages (wraps around) |
| Details | Toggle detailed mode |
| Sort | Cycle sort mode |
| Search | Filter by ID/name (`!pattern!` to exclude) |
| Enable Timer-modified | Re-enable triggers with modified timers |

### Action Modes

| Mode | Action |
|------|--------|
| Run | Execute trigger actions immediately |
| Enable | Activate the trigger |
| Disable | Deactivate the trigger |
| Destroy | Destroy the trigger |
| Set Timer | Set the timer value |

### Sort Modes

| Mode | Description |
|------|-------------|
| Raw | Default order |
| ByID | Ascending by Type ID |
| ByName | Ascending alphabetical |
| ByTimeLeft | Ascending by remaining time |
| ByLastExecuted | Descending by last execution |
| ByDestroyed | Descending by destruction time |

## AI Trigger Debug Mode

Interactive panel for inspecting AI Trigger Types decision-making for each house.

### Display

Shows all `AITriggerTypeClass` entries with:
- **Status icon:** `+` (condition met), `o` (condition not met), `x` (disabled)
- **ID** — trigger identifier
- **Name** — trigger description (marquee scroll if too long)
- **Condition** — readable condition with comparator and object (marquee scroll)
- **Weight** — min/current/max weight (yellow if < 20% of max)
- **TL** — tech level
- **Team** — Team1/Team2

### Color Indication

| Color | Meaning |
|-------|---------|
| Green | Condition met for selected house |
| Gray | Condition not met |
| Red | Trigger disabled |
| Yellow | Weight < 20% of maximum |

### UI Controls

| Button | Action |
|--------|--------|
| Page Up/Down | Navigate trigger pages (wraps around) |
| House name | Cycle through AI houses or "All AI Houses" |
| Click on status | Enable/disable the trigger |
| Click on trigger | Show detailed info in message area |

### Detailed Info (on click)

Displays: condition type, all 4 comparators, owner filter, side, tech level, weight (min/current/max), teams, execution counts, difficulty flags, skirmish/global status.

### Weight Adjustment

| Key | Action |
|-----|--------|
| Numpad + | Increase weight by 1 |
| Numpad - | Decrease weight by 1 |
| Ctrl + Numpad +/- | Multiply step by 10 |
| Shift + Numpad +/- | Divide step by 10 |

### How it works

The game periodically evaluates all `AITriggerTypeClass` entries for each AI house. Each trigger checks:
1. Is it enabled?
2. Does the difficulty match?
3. Does the house/side filter match?
4. Is the tech level sufficient?
5. Is the weight > 0?
6. Is the condition met?

Triggers that pass are selected by weighted random. Weight = 5000 means "fire immediately".

## Hooks

| Address | Purpose |
|---------|---------|
| `0x533066` | Command registration |
| `0x4F4583` | Overlay rendering |
| `0x69300B` | Mouse hover tracking |
| `0x6931A5` | Mouse click handling |
| `0x693268` | Button state reset |
| `0x692F85` | Long-press prevention |
| `0x6851F0` | Logic initialization |
| `0x7265D1` | Trigger execution recording |
| `0x7264C0` | Event clearing |
| `0x726564` | Condition recording |
| `0x726720` | Trigger destruction |
| `0x5FACDF` | INI config loading |

## Known Conflicts

| Address | Frameworks | Severity |
|---------|------------|----------|
| `0x533066` | Phobos, AggressiveStance | High |
| `0x4F4583` | Ares, Kratos, Phobos | Critical |
| `0x6851F0` | Kratos | Medium |

> Loading ObjectInfo alongside the listed frameworks may cause hook conflicts at these addresses.

## Building

**Requirements:**
- Visual Studio 2022+ with MSVC v143 toolset
- Windows 10 SDK

```bash
# Release build
msbuild ObjectInfo.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0

# Debug build (includes Trigger Debug commands)
msbuild ObjectInfo.vcxproj /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0
```

Output: `Release\ObjectInfo.dll` or `Debug\ObjectInfo.dll` + `objectinfo.ini`

## Project Structure

```
ObjectInfo/
├── src/
│   ├── ObjectInfo.cpp      # Main overlay, commands, INI loading
│   ├── TriggerDebug.cpp    # Trigger debug UI, mouse hooks, event tracking
│   ├── AITriggerDebug.cpp  # AI Trigger Types decision-making panel
│   ├── Common.h            # Shared types, globals, utilities, templates
│   ├── Rules.h             # Display config, CanDisplay logic
│   ├── command.h           # MakeCommand template
│   ├── GeneralUtils.h/cpp  # String helpers (LoadStringUnlessMissing)
│   ├── CopyProtection.cpp  # Debug-only DRM bypass hooks
│   ├── version.h           # Version constants
│   └── ObjectInfo.rc       # Windows VERSIONINFO resource
├── YRpp/                   # Phobos-developers/YRpp submodule (phobos-dev)
├── objectinfo.ini          # Runtime configuration
├── ObjectInfo.vcxproj      # MSBuild project
├── LICENSE                  # GPL v3
└── Readme.md
```

## Related Projects

| Project | Description |
|---------|-------------|
| [Phobos-developers/Phobos](https://github.com/Phobos-developers/Phobos) | Modern engine extension framework for RA2:YR |
| [Phobos-developers/YRpp](https://github.com/Phobos-developers/YRpp) | C++ headers for the Yuri's Revenge engine |
| [Ares-Developers/Ares](https://github.com/Ares-Developers/Ares) | Original engine extension DLL (legacy) |
| [Ritanlisa/RA2YR_ReSource](https://github.com/Ritanlisa/RA2YR_ReSource) | Reverse-engineered class layouts and field names |
| [mirelle7/YRdecomp](https://github.com/mirelle7/YRdecomp) | Matching decompilation of gamemd.exe |
| [Everything-Compatible/YRDict](https://github.com/Everything-Compatible/YRDict) | Dictionary of YR engine reversing data |
| [SethGekco/YR-Hook-Encyclopedia](https://github.com/SethGekco/YR-Hook-Encyclopedia) | Framework-neutral hook address registry |
| [ModEnc](https://modenc.renegadeprojects.com) | Wiki encyclopedia of INI tags and flags |

## Credits

- **Handama** — original author ([RA2DIY](https://bbs.ra2diy.com/forum.php?mod=viewthread&tid=24561), [GitHub](https://github.com/handama/ObjectInfo))
- **Phobos-developers** — [YRpp](https://github.com/Phobos-developers/YRpp) headers
- **Ares / Phobos / Kratos** communities — reverse-engineered game knowledge

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

Originally created by **Handama** — see [LICENSE](https://github.com/handama/ObjectInfo/blob/main/LICENSE).
