# Dashboard Components

This firmware keeps component metadata, deterministic layout logic, rendering,
and runtime orchestration in separate layers. When adding, removing, or changing
a dashboard component, keep changes in the smallest matching layer.

## Layers

- `include/logic/LayoutTypes.h`: component identity enum and layout item data.
- `include/logic/components/*Component.h`: default layout, type key, perf label,
  and update flags for one component.
- `include/logic/ComponentRegistry.h`: ordered registry used to build default
  layouts and resolve metadata by component id.
- `include/logic/ComponentUpdateGroups.h`: derived update groups. Do not hard
  code component lists here; use `DashboardComponentDefinition.update_flags`.
- `src/render/*Renderer.*`: drawing code and render cache for one visual area.
- `src/ClockApp.Component*.inc`: adapter from app state to renderer input.
- `src/ClockApp.ComponentDispatch.inc`: runtime dispatch from component id to
  update adapter.
- `src/LayoutDocumentCodec.*`: config document schema and layout apply parsing.
- `src/LayoutPreviewDocument.*`: config UI preview data assembled from the same
  app settings and display formatting helpers.

## Add a Component

1. Add the enum value in `DashboardComponentId` before `Count`.
2. Add `include/logic/components/<Name>Component.h` with:
   - `k<Name>Component` default `DashboardLayoutItem`
   - `k<Name>ComponentDefinition`
   - correct update flags such as `kUpdateOnMinute`, `kUpdateOnDate`,
     `kUpdateOnSensor`, or `kUpdateOnMarket`
3. Include that component header in `ComponentRegistry.h` and add the definition
   to `kDashboardComponentDefinitions` in the same order as the enum.
4. Add renderer state/input/result and the renderer function under `src/render`
   when the component draws new UI.
5. Add a small `ClockApp.Component<Name>.inc` adapter that reads app state,
   builds renderer input, and pushes dirty rects.
6. Add the adapter declaration to `ClockApp.h` and dispatch entry to
   `ClockApp.ComponentDispatch.inc`.
7. Update `LayoutDocumentCodec.*` only when the component needs custom JSON
   properties beyond common bounds/visibility.
8. Update `LayoutPreviewDocument.*` only when the web preview needs new render
   data fields.
9. Add or adjust native tests in `test/test_logic/test_main.cpp` for registry,
   update grouping, layout parsing, and any pure helper behavior.
10. Verify with:
    - `git diff --check`
    - `platformio test -e native`
    - `platformio run -e m5stack-fire`

## Modify a Component

- Default placement or size: edit only its file in `include/logic/components/`.
- Refresh behavior: edit only `k<Name>ComponentDefinition.update_flags`; update
  group tests should catch mismatches.
- Drawing: edit only the matching `src/render/*Renderer.*` and the component
  adapter if renderer input changes.
- Config schema: edit `LayoutDocumentCodec.*`, then cover malformed and boundary
  inputs in native tests when the logic is platform-independent.
- Preview schema: edit `LayoutPreviewDocument.*` and reuse `DisplayFormatting`
  for display strings so preview and device rendering stay consistent.

## Remove a Component

1. Remove the enum value and component definition.
2. Remove it from `kDashboardComponentDefinitions`.
3. Remove the dispatch entry and adapter declaration/implementation.
4. Remove renderer files only if no other component uses them.
5. Remove layout document and preview special cases.
6. Run the same verification commands.

## Boundaries

- Keep deterministic business rules in `include/logic` so native tests can cover
  them without Arduino or M5EPD.
- Keep Arduino `String`, RTC structs, sensor readings, and market quote adapters
  in `src` modules such as `DisplayFormatting`, renderers, and services.
- Do not add component-specific conditionals to generic update groups. Add a flag
  to the component definition instead.
- Do not let renderers mutate app settings or call network/storage services.
  Renderers receive input and return dirty area/update mode decisions only.
