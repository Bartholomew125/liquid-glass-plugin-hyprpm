# Liquid Glass Plugin for Hyprland

A Hyprland plugin that adds a liquid glass refraction effect to windows, inspired by Apple's liquid glass design.

## Features

- Real-time liquid glass refraction effect on all windows
- Configurable refraction strength
- Smooth integration with Hyprland's rendering pipeline

## Installation

### Using hyprpm (Recommended)

```bash
hyprpm add https://github.com/xiaoxigua-1/liquid-glass-plugin-hyprpm
hyprpm enable liquid-glass
```

### Manual Build

Requirements:
- Hyprland with development headers
- pkg-config
- A C++23 compatible compiler (g++ or clang++)

```bash
git clone https://github.com/xiaoxigua-1/liquid-glass-plugin-hyprpm
cd liquid-glass-plugin-hyprpm
make all
```

Then load the plugin manually:
```bash
hyprctl plugin load /path/to/liquid-glass.so
```

## Configuration

Add the following to your `hyprland.conf`:

```conf
# Load the plugin (if not using hyprpm)
# exec-once = hyprctl plugin load /path/to/liquid-glass.so

# Plugin configuration
plugin {
    liquid-glass {
        enabled = true
        refraction_strength = 0.08
    }
}
```

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enabled` | int | 1 | Enable/disable the liquid glass effect (1 = enabled, 0 = disabled) |
| `refraction_strength` | float | 0.08 | Strength of the refraction effect (0.0 - 1.0) |

## Hyprland.conf Example

```conf
# =============================================
# Liquid Glass Plugin Configuration
# =============================================

# Enable the plugin
plugin {
    liquid-glass {
        enabled = true
        refraction_strength = 0.08
    }
}

# Recommended window rules for best effect
decoration {
    rounding = 10
    blur {
        enabled = false  # Disable default blur as plugin handles it
    }
}
```

## Troubleshooting

### Plugin fails to load
- Ensure your Hyprland version matches the plugin's compiled version
- Rebuild the plugin after updating Hyprland: `hyprpm update`

### Visual artifacts
- Try adjusting the `refraction_strength` value
- Ensure your GPU drivers are up to date

## License

MIT License - See [LICENSE](LICENSE) for details.

## Credits

- Original author: xiaoxigua-1
- Inspired by Apple's liquid glass design language
