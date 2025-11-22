# Parkour Environment Blockout Generator

An Unreal Engine Editor plugin that radically simplifies level design for parkour gameplay.

## Overview

The designer only needs to define three things:
1. **Play Area Size and Shape** - Default cube that can be deformed (L-shape, U-shape, custom)
2. **Path** - A spline that defines the parkour route, adjustable in height and direction
3. **Jump Parameters** - Maximum jump height and maximum jump distance

With these minimal inputs, the plugin automatically generates a complete playable parkour blockout.

## Features

- **Automatic Platform Generation** - Platforms placed along the path
- **Valid Jump Distances** - Correct spacing and heights for achievable jumps
- **Intermediate Platforms** - Automatically added when jumps would be too far/high
- **Environment Fill** - Urban, Industrial, or Abstract blockout geometry around the path
- **Clean Blockout Geometry** - Simple, clear geometry ready for iteration

## Installation

1. Copy the `ParkourGenerator` folder to your project's `Plugins` directory
2. Regenerate project files (right-click .uproject → Generate Visual Studio project files)
3. Open project in Unreal Engine
4. Enable the plugin if not auto-enabled (Edit → Plugins → search "Parkour")

## Quick Start

### Using the Menu

1. Go to **Tools → Parkour Generator**
2. Click **Spawn Parkour Path** to create a new path
3. Click **Spawn Play Area** to create the bounds

### Manual Workflow

1. **Create a Parkour Path**
   - Place a `ParkourPath` actor in your level
   - Edit the spline points to define your route
   - Adjust jump parameters in the Details panel

2. **Create a Play Area** (optional)
   - Place a `ParkourPlayArea` actor
   - Set dimensions and shape type
   - Link to your ParkourPath in Details

3. **Generate**
   - Select the ParkourPath actor
   - Click "Generate Parkour" in Details panel
   - For environment: Select PlayArea → "Generate Environment"

## Actor Reference

### ParkourPath

The main path actor with spline-based route definition.

**Jump Parameters:**
- `MaxJumpHeight` - Maximum vertical jump (default: 200cm)
- `MaxJumpDistance` - Maximum horizontal jump (default: 400cm)
- `JumpSafetyMargin` - Safety multiplier (default: 0.85 = 85% of max)

**Platform Settings:**
- `PlatformWidth` - Platform width (default: 200cm)
- `PlatformDepth` - Platform depth (default: 200cm)
- `PlatformThickness` - Platform height (default: 50cm)

**Variation:**
- `bAllowPlatformVariation` - Enable random size variation
- `PlatformSizeVariation` - Variation amount (0.0-0.5)
- `RandomSeed` - Seed for reproducible results (0 = random)

**Functions:**
- `GenerateParkour()` - Generate platforms along path
- `ClearGeneratedParkour()` - Remove generated platforms

### ParkourPlayArea

Defines bounds and generates environment fill.

**Shape Settings:**
- `AreaShape` - Box, L-Shape, U-Shape, or Custom
- `AreaDimensions` - Size of the play area

**Environment Settings:**
- `EnvironmentStyle` - None, Urban, Industrial, or Abstract
- `ParkourPath` - Reference to path (for clearance)
- `PathClearance` - Distance to keep from path
- `BuildingDensity` - Fill density (0.0-1.0)

**Functions:**
- `GenerateEnvironment()` - Generate buildings/structures
- `GenerateAll()` - Generate path + environment
- `ClearAll()` - Clear everything

## Jump Presets

Access via `UParkourGeneratorSettings`:

| Preset | Max Height | Max Distance |
|--------|------------|--------------|
| Realistic | 150cm | 350cm |
| Athletic | 250cm | 500cm |
| Superhuman | 400cm | 800cm |

## Blueprint API

The `UParkourGeneratorSubsystem` provides Blueprint-callable functions:

```cpp
// Spawning
SpawnParkourPath(Location)
SpawnPlayArea(Location, Dimensions)
SpawnCompleteParkourSetup(Location, OutPath, OutPlayArea)

// Finding
GetAllParkourPaths()
GetAllPlayAreas()

// Generation
GenerateAllParkour()
GenerateAllEnvironments()
GenerateEverything()

// Cleanup
ClearAllParkour()
ClearAllEnvironments()
ClearEverything()

// Validation
ValidatePath(Path, OutInvalidJumpIndices)
```

## Core Principle

```
Path → Validate Jumps → Generate Platforms → Fill Environment
```

## Best Practices

1. **Start Simple** - Create a basic path first, then add complexity
2. **Use Safety Margin** - Keep at 0.8-0.9 for comfortable jumps
3. **Test Frequently** - Generate and playtest often
4. **Iterate on Path** - The spline is your main creative control
5. **Environment Last** - Generate environment after finalizing the path

## Suitable For

- Third-Person Action games
- FPS Traversal games
- Movement Shooters
- Parkour games
- Rapid prototyping

## Output

After generation, you get a complete blockout level that:
- Is immediately playable
- Has clear flow
- Guarantees achievable jumps
- Is visually structured for iteration

---

*"The designer controls only: Space, Route, Jump Ability. Everything else is automatic."*
