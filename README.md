# Freedom series Analog Keypads

## Tech Stack

KFC (Kicad, Freecad, C/C++)

## Firmware

<https://github.com/gamaPhy/qmk-keyboards/tree/main/firmware/freedom>

## How To

### Create an PCB outline in FreeCAD and import to Kicad

1. Create sketch in FreeCAD
2. Mirror sketch
3. Merge original sketch and mirrored sketch together
4. Export the merged sketch as `filename.dxf` (An Autodesk DXF 2D file)
5. In Kicad, import graphics as an `Edge.Cuts` layer
