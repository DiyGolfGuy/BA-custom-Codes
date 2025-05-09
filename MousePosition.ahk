#Persistent
CoordMode, Mouse, Screen  ; Ensures absolute screen coordinates
SetTimer, ShowCoords, 100  ; Updates every 100ms

ShowCoords:
    MouseGetPos, X, Y
    ToolTip, Screen X: %X% | Screen Y: %Y%
    return

Esc::
    ToolTip  ; Clears the tooltip when Esc is pressed
    ExitApp  ; Exits the script