# file-shelf

A lightweight Windows utility inspired by [Yoink](https://eternalstorms.at/yoink/mac/) on macOS. file-shlef gives you a floating shelf to temporarily stage files and folders while you navigate between windows — drag files onto it, then drag them out wherever you need them.

## Features

- Drag files and folders onto the shelf to stage them
- Drag staged items out to any destination
- Double-click an item to reveal it in Explorer
- Click the × button to remove an item from the shelf
- Acrylic/frosted glass appearance
- Always on top, out of your way
- Sits in the system tray

## Usage

Run `shelf.exe` — no installation or configuration needed.

| Action | How |
|---|---|
| Show / hide shelf | `Ctrl + Alt + Shift + S` |
| Stage a file | Drag and drop onto the shelf |
| Use a staged file | Drag it out to any window or folder |
| Reveal in Explorer | Double-click the item |
| Remove from shelf | Hover the item and click × |
| Hide shelf | Click the `^` button or the hotkey |

## Building

Requires [MSYS2](https://www.msys2.org/) with the UCRT64 toolchain and GDI+ (included with Windows SDK).

```bash
make
```

The binary is output to the `build/` directory.

