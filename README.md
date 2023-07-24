# Totk Unexplored
Easily view what hasn't been discovered in your Tears of the Kingdom savefile on your Nintendo Switch.

The korok seeds that haven't been collected, the locations that are left to visit and the 16 other types of unexplored collectibles are shown on a map, where it's easy to see what you've missed.

<img src="https://github.com/lud99/totk-unexplored/blob/master/map3_2.0.jpg?raw=true" width=600>

## Main Features
- 18 types of collectibles can be tracked
    - Koroks
    - Shrines
    - Lightroots
    -   Caves
    -    Bubbuls
    -    Wells
    -    Chasms
    -    Locations
    -    Hinoxes
    -    Taluses
    -    Moldugas
    -    Flux Constructs
    -    Froxes
    -    Gleeoks
    -    Sage's Wills
    -    Old Maps
    -    Addison Signs
    -    Schema Stones
    -    Yiga Schematics

- Legend to easily toggle what collectibles to see, and also to switch between showing missing, showing completed or showing all
- Scan a Qr Code in the app to see an image of the map on your phone to look at while playing 
- Object info about collectibles. Name, position, and what type of korok it is (if korok) to help find it
- Korok paths are drawn so it is easy to find koroks that involve flower trails, races and transporting a lonely korok back to its friend
- All entrances to caves are shown on the map and has a line drawn between them?
- Backup system to be able to use the app while playing the game, as homebrew can't read savefiles of running games
- Quickly swap between the Depths, Surface and Sky layers
- Optimized interface and experience for both touch and controller 

## Why?
This is an updated version to BotW Unexplored and has a few larger features but also so many small quality of life improvements
## Usage
Download ```totk-unexplored.nro``` from the [releases](https://github.com/lud99/botw-unexplored/releases/download/2.0.0/botw-unexplored.nro), transfer it to your SD card and launch it. Most things should be self explanitory, but here's some information just in case:

* Use the analog sticks or the touch screen to move around.  
* Press X to open the legend and use either the touch screen or the D-pad to navigate the menu (note that the analog sticks won't work here) 
* Tap a korok to see a guide on how to find it. You can press B to manually mark it as complete. This is useful if you're using the app while playing BotW, as the app won't be able to load your latest savefile in that case. The koroks you've marked as found will be removed once the savefile is able to be loaded it again, so don't worry if you wrongly marked a korok as found.

 - Some caves have multiple entrances. All of them are shown on the map, but only is needed to be explored.
- Can mark anything as completed

## Version History

### 1.0
* Initial release

## Building
switch-mesa, switch-glad, switch-freetype and switch-glm are required for building the project. Install them with the command ```pacman -S switch-mesa switch-glad switch-glm switch-freetype```  
Then run ```Make``` to build the .nro file.

### Credits
Huge thanks to these kind people / groups for making this project possible:

* https://github.com/marcrobledo/savegame-editors For most of the data used (savefile hashes and coordinates) and savefile parsing. Also thank you for taking your time to find out edge-cases and verifiying the data
* https://github.com/zeldamods/objmap-totk For being a great tool for debugging and for getting names for collectibles
* https://github.com/zeldamods/radar-totk For having data on all ingame objects so I could piece together the "korok paths"
* https://zeldadungeon.net For their amazing interactive map

##### Libraries
* https://github.com/nothings/stb For image loading and saving
* https://github.com/nayuki/QR-Code-generator For the QR-code creation
* https://github.com/nlohmann/json For parsing the gamedata from a JSON file