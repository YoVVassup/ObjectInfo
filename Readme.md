<p align="center">
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
- **Configurable Presets** — define which fields to display via INI config
- **Trigger Dump** — export all trigger data to `debug.log`

## Installation

**With Syringe (Ares/Phobos mods):**

Copy `ObjectInfo.dll` and `objectinfo.ini` into the game directory (next to `gamemd.exe`). Syringe automatically discovers and loads all DLLs in the root folder.

**Without Syringe:**

Use an external DLL injector to load `ObjectInfo.dll` into the `gamemd.exe` process.

## Configuration — objectinfo.ini

### `[ObjectInfoDisplayLists]`

Defines display presets. Each key is a separate preset, cycled with the "Next Info Preset" hotkey:

```ini
[ObjectInfoDisplayLists]
0=uiname,id,uid,hp,owner,location,target,destination,currentmission
1=id,hp,location,team,currentscript,passenger
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

## Hotkeys

Assign these in the game's keyboard options:

| Command | Description |
|---------|-------------|
| **Display Object Info** | Toggle the object info overlay |
| **Next Info Preset** | Cycle through display presets |
| **Dump Trigger Info** | Export all trigger data to `debug.log` |
| **Trigger Debug Mode** | Toggle the interactive trigger debugger |
| **Trigger Debug Page Up** | Scroll up in the trigger list |
| **Trigger Debug Page Down** | Scroll down in the trigger list |

## Displayed Fields

### FootClass (Infantry / Vehicles / Aircraft)

| Field | Output Format |
|-------|---------------|
| `uiname` | Localized UI name |
| `id` | `ID = <TypeID>` |
| `uid` | `UID = <UniqueID>` |
| `hp` | `HP = (current / max)` |
| `owner` | `Owner = <ID> (<PlainName>)` |
| `location` | `Location = (X, Y)` |
| `link` | `Link N: UID = .., ID = .., Location = (X, Y)` |
| `target` | `Target = <ID>, Distance = N, Location = (X, Y)` |
| `destination` | `Destination = <ID>, Distance = N, Location = (X, Y)` or `Destination = (X, Y)` |
| `focus` | `Focus = <ID>, Distance = N, Location = (X, Y)` |
| `ammo` | `Ammo = (current / max)` |
| `currentmission` | `Current Mission = N (name)` |
| `megamission` | `Mega Mission = N (name)` |
| `megatarget` | `Mega Target = <ID>, Distance = N, Location = (X, Y)` |
| `megadestination` | `Mega Destination = <ID>, Distance = N, Location = (X, Y)` |
| `group` | `Group = N` |
| `recruit` | `RecruitA = N, RecruitB = N` |
| `veterancy` | `Veterancy = Rookie/Veteran/Elite (N.xx)` |
| `tag` | `Tag = <ID>, InstanceCount = N` + nested triggers |
| `aitrigger` | `Trigger ID = .., weights [Current, Min, Max]: .., .., ..` |
| `team` | `Team ID = .., Script ID = .., Taskforce ID = ..` |
| `currentscript` | Current script line or missing units |
| `passenger` | `N Passengers: <ID>, <ID>, ...` |

### BuildingClass

| Field | Output Format |
|-------|---------------|
| `power` | `Power: N, Total Power Output: N, Total Power Drain: N` |
| `factory` | `Production: <ID> (N%)` |
| `money` | `Money: N` |
| `occupants` | `N Occupants: <ID>, <ID>, ...` |
| `enemy` | `Enemy = <ID> (<Name>), AngerLevel = N` |
| `upgrades` | `Upgrades (N / Max): Slot 1 = <ID>, Slot 2 = .., Slot 3 = ..` |

## Trigger Debug Mode

Interactive UI for inspecting and controlling triggers in real-time.

### Display

**Basic mode:**
- Trigger ID and name (green = enabled, black = disabled)

**Detailed mode:**
- `Frame Left(s): N(Ns)` — remaining time
- `(Modified)` — if timer was manually changed
- `Last Executed Frame(s): N(Ns)` — last execution time
- `Execute Count: N` — total executions
- `Conditions: N[@]/N[  ]` — condition states

**Destroyed triggers:**
- `<Expired> <ID> (<Name>)` with destruction timestamp

### UI Buttons

| Button | Action |
|--------|--------|
| Page Up/Down | Navigate pages |
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
msbuild ObjectInfo.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0
```

Output: `Release\ObjectInfo.dll` + `Release\objectinfo.ini`

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
