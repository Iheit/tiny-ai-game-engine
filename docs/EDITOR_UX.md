# Tiny AI Editor UX Direction

The editor should feel like a conventional professional game editor rather than a demo application. The goal is familiarity, hierarchy, and low visual noise.

## Layout

- Top: compact menu and toolbar.
- Left: Scene/Hierarchy dock.
- Center: 3D viewport with unobtrusive grid and transform gizmos.
- Right: Inspector dock with collapsible component sections.
- Bottom: Output/Console and asset/script panels.
- Startup: project Hub with New, Open, Recent, and Settings.

## Visual language

- Dark neutral editor chrome.
- One restrained accent color for selection and active controls.
- Consistent 4-8 px spacing rhythm.
- Clear typography hierarchy.
- No oversized demo buttons inside the viewport.
- Tooltips for unfamiliar controls.
- Keyboard shortcuts for common operations.
- Selection state must be obvious without obscuring the object.

## Core interaction expectations

- Ctrl+S saves immediately.
- Ctrl+Z / Ctrl+Y provide undo/redo.
- Delete removes the selected entity after a safe edit-state check.
- F focuses the selected entity.
- W/E/R switch move/rotate/scale tools.
- F6 starts Play mode and F8 stops it.
- Double-clicking an asset opens the appropriate editor/importer.
- The Inspector always reflects the current selection.
- Long operations report progress and errors in the Output panel.

## Viewport

The viewport is the primary workspace. It should provide:

- Perspective and orthographic views.
- Grid with axis labels.
- Camera orbit, pan and dolly.
- Transform gizmos.
- Visible light/camera icons.
- Selection outlines.
- Material preview and lit preview modes.
- Play-in-editor state clearly separated from edit state.

## Lighting UX

Lights are first-class scene entities. The Inspector exposes:

- Type
- Color
- Intensity
- Range for local lights
- Direction/rotation for directional and spot lights
- Cone angle and softness for spot lights
- Shadows when supported by the active renderer

A new 3D scene should contain sensible neutral lighting, but users can remove or replace it.

## Quality bar

A control is not considered complete because it exists visually. It must have a working action, a useful status/error response, persistence where appropriate, and a corresponding runtime behavior when applicable.
