# 🎮 Switch App Store

A Nintendo Switch homebrew application with Apple App Store style UI, built for Atmosphere CFW.

<p align="center">
  <img src="docs/preview.png" alt="Preview" width="600">
</p>

## ✨ Features

- **Apple App Store UI** - Beautiful, familiar interface inspired by iOS App Store
- **Game Browser** - Browse games by category with smooth scrolling
- **Search** - Quick search with hot keywords
- **Today View** - Featured content with large cards
- **Download Manager** - Queue downloads with progress tracking
- **NSP/XCI Installer** - Direct installation to your Switch
- **Dark Mode** - Eye-friendly dark theme support
- **Touch & Controller** - Full support for both input methods

## 📱 Screenshots

| Today | Games | Detail |
|-------|-------|--------|
| Coming Soon | Coming Soon | Coming Soon |

## 🛠️ Building

### Prerequisites

1. Install [devkitPro](https://devkitpro.org/wiki/Getting_Started)
2. Install required packages via MSYS2 terminal:
   ```bash
   pacman -S switch-sdl2 switch-sdl2_image switch-sdl2_ttf switch-sdl2_gfx
   pacman -S switch-curl switch-mbedtls switch-freetype switch-libpng switch-libjpeg-turbo
   ```

### Compile

```bash
# Clone the repository
git clone https://github.com/yynickyeh/switch-appstore.git
cd switch-appstore

# Build
make

# Clean build
make clean && make
```

This will generate `switch-appstore.nro` in the project root.

## 📦 Installation

1. Copy `switch-appstore.nro` to `/switch/` on your SD card
2. Launch via Homebrew Menu (Album or title override)

## 🎮 Controls

| Button | Action |
|--------|--------|
| A | Select / Confirm |
| B | Back / Cancel |
| X | Quick Search |
| Y | Refresh / Options |
| L/R | Switch Tabs |
| ZL/ZR | Page Scroll |
| + | Settings |
| - | Downloads |

## 🔧 Configuration

Config file location: `/switch/switch-appstore/config.json`

```json
{
  "theme": "light",
  "language": "zh-CN",
  "download_path": "/switch/downloads/",
  "sources": []
}
```

## 📁 Project Structure

```
switch-appstore/
├── source/           # Source code
│   ├── core/         # Renderer, Input, Timer
│   ├── ui/           # UI components and screens
│   ├── network/      # HTTP, Download, ImageCache
│   ├── store/        # Store logic, Installer
│   └── utils/        # Config, Logger, Utilities
├── include/          # Header files
└── romfs/            # Bundled resources (fonts, icons)
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## ⚠️ Disclaimer

This software is provided for educational purposes only. The developers are not responsible for any misuse or damage caused by this software. Please respect intellectual property rights.

---

Made with ❤️ for the Nintendo Switch homebrew community
