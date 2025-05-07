# Freedom series Analog Keypads

## Tech Stack

KFC (Kicad, Freecad, C)

## Firmware

<https://github.com/gamaPhy/qmk-keyboards/tree/main/firmware/freedom>

![freedom-3k](https://github.com/user-attachments/assets/1af95d7a-c4f3-4e92-a06f-d3448b5f194e)

## How To

### Create an PCB outline in FreeCAD and import to Kicad

1. Create sketch in FreeCAD
2. Mirror sketch
3. Merge original sketch and mirrored sketch together
4. Export the merged sketch as `filename.dxf` (An Autodesk DXF 2D file)
5. In Kicad, import graphics as an `Edge.Cuts` layer
