# Changelog

All notable changes to the ObjectInfo project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/).

## [0.0.4.0] - 2026-09-02

### Safety & Stability
- Replaced global static `char[]` buffers with local buffers in `Log()`, `Format()`, and `Message()` to prevent buffer overflows
- `Format()` now returns `std::string` instead of a raw pointer to a global buffer
- `SearchPattern` changed from fixed `wchar_t[0x200]` to `std::wstring`
- `displayBuffer` in the object info overlay changed from fixed `char[0x800]` to `std::string`
- Added null-pointer checks throughout `ObjectInfo.cpp` and `TriggerDebug.cpp`

### Bug Fixes
- Fixed crash on button click in Trigger Debug — `PressLeftMouseButton` hook now returns 0 when nothing is hovered instead of always intercepting all left clicks game-wide
- Fixed Trigger Debug header position — buttons now draw BEFORE entries, not after
- Fixed `GetSideName()` to read side names dynamically from `SideClass::Array` instead of hardcoded Allied/Soviet/Yuri values
- Fixed AI trigger side filtering for custom houses with `ParentCountry` — now uses `HouseTypeClass::SideIndex` (0-based) with correct offset conversion from `AITriggerTypeClass::SideIndex` (1-based, 0=All)
- House navigation now displays the side name for each house

### Performance
- AI Trigger Debug panel now caches filtered triggers (`sFilteredTriggers`), reducing per-click cost from O(N) to O(1)
- Cache is invalidated only on house selection change

### Code Quality
- Merged duplicate `match_wchar_patterns` / `match_wchar_patternsExt` into a single `match_trigger_patterns<Item>` template
- Extracted `DrawTextButton()` helper to eliminate repeated button-drawing boilerplate
- Replaced 14+ magic numbers with named constants (`TEXT_LINE_HEIGHT`, `COLOR_TRIGGER_*`, `FRAMES_TO_SECONDS`, `FACTORY_PROGRESS_MAX`, `AI_TRIGGER_FIRE_IMMEDIATELY_WEIGHT`, `AI_TRIGGER_LOW_WEIGHT_RATIO`)
- Refactored AI Trigger column header rendering to use a data-driven loop
- Added comprehensive English comments and documentation to all source files

## [0.0.3.0] - 2026-08-27

### Added
- AI Trigger Debug Mode panel with per-house filtering
- Weight adjustment via Numpad +/- (Ctrl x10, Shift /10)
- Marquee scrolling for long trigger names and conditions
- `DrawTextOutline()` for readable overlay text on any background

### Changed
- Migrated from vendored YRpp headers to Phobos-developers/YRpp git submodule
- Removed ~600K lines of bundled YRpp documentation

## [0.0.2.0] - 2026-04-18

### Added
- Radio link display for linked units

### Fixed
- Data accuracy issue regarding EXP output ([PR #1](../../pull/1) by 九千天华)

## [0.0.1.0] - 2025-05-14

### Added
- Trigger Debug Mode with interactive UI
- Trigger sorting (by ID, name, time left, last executed, destroyed)
- Detailed trigger view with condition states and execution counts
- Destroyed trigger tracking via `TriggerVariant`
- Trigger search with `!pattern!` exclude syntax
- Ability to run expired triggers
- Trigger dump to debug.log

## [0.0.0.1] - 2024-11-20

### Added
- Initial release
- Object info overlay with configurable field presets
- Basic display fields (ID, HP, owner, location, veterancy, etc.)
