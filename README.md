# FlyingGuys
External cheat for Fall Guys
- Modify Max Speed/Speed Hack
- Modify Gravity
- No Collision (EXPERIMENTAL)

## Build Instructions
```
1. Open FlyingGuys.sln (VS2019)
2. Build Package (x64, Release) (Requires Windows SDK and Windows WDK) OR Use driver provided in releases
3. Build FlyingGuys 
```

## Run Instructions
```
1. Open Fall Guys with Epic Games Launcher (Steam version does not work with this cheat)
2. Run FlyingGuys.exe as Administrator
3. Press "Insert" to open the menu in game or use Hotkeys below
```

## Hotkeys
```
Insert = Open Menu

Home = Increase Max Speed
End = Decrease Max Speed

PageUp = Increase Gravity
PageDown = Decrease Gravity
```

## Known Bugs/Issues
```
- Windows 11 Insider builds are not currently working due to an issue with the vulnerable Intel driver KDMapper uses being revoked in the newest Windows 11 Insider builds.
- If you are receiving the error "VCRUNTIME140.dll and MSVCP140.dll is missing or similar" please install Visual C++ Redistributable - https://aka.ms/vs/17/release/vc_redist.x64.exe
```

## Credits
- KnPCore - https://github.com/da772/KnP/
- KDMapper - https://github.com/TheCruZ/kdmapper
- zLib - https://zlib.net/
- ImGui - https://github.com/ocornut/imgui
