# Totk Unexplored
Easily view what hasn't been discovered in your Tears of the Kingdom savefile on your Nintendo Switch.

The korok seeds that haven't been collected, the locations that are left to visit and the 18 other types of unexplored collectibles are shown on a map, where it's easy to see what you've missed.

<img src="https://github.com/lud99/totk-unexplored/blob/master/map3_2.0.jpg?raw=true" width=600>

## Main Features
- 20 types of collectibles can be tracked
- Legend to easily toggle what collectibles to see, and also switch between showing missing, showing completed or showing all
- Scan a Qr Code in the app to see an image of the map on your phone to look at while playing 
- Object info about collectibles. Name, position, and what type of korok it is (if korok)
- Korok paths are drawn to easily complete flower trails, races and carry the lonely korok back to its friend
- All entrances to caves are shown on the map and has a line drawn between them
- Backup system to be able to use the app while playing the game (homebrew cant read savefiles of running games)
- Quickly swap between the Depths, Surface and Sky layers
- Optimized the interface and experience for both touch and controller 

## Why?
This is an updated version to BotW Unexplored and has a few larger features but also so many small quality of life improvements
## Usage
Download ```totk-unexplored.nro``` from the [releases](https://github.com/lud99/botw-unexplored/releases/download/2.0.0/botw-unexplored.nro), transfer it to your SD card and launch it. Most things should be self explanitory, but here's some information just in case:

* Use the analog sticks or the touch screen to move around.  
* Press X to open the legend and use either the touch screen or the D-pad to navigate the menu (note that the analog sticks won't work here) 
* Tap a korok to see a guide on how to find it. You can press B to manually mark it as complete. This is useful if you're using the app while playing BotW, as the app won't be able to load your latest savefile in that case. The koroks you've marked as found will be removed once the savefile is able to be loaded it again, so don't worry if you wrongly marked a korok as found.

 - Some caves have multiple entrances. All of them are shown on the map, but only is needed to be explored.
- Can mark anything as completed
- Object info panel

        Koroks
        Shrines,
        Lightroots,
        
        Caves,
        Bubbuls,
        Wells,
        Chasms,
        Locations,

        Hinoxes,
        Taluses,
        Moldugas,
        FluxConstructs,
        Froxes,
        Gleeoks,

        SagesWills,
        OldMaps,
        AddisonSigns,
        SchemaStones,
        YigaSchematics,
        ShowCompleted,
        Count

## Version History

### 2.0
* Added Shrines (+ DLC if it's present), Taluses, Hinoxes and Moldugas.
* Added a legend where you can toggle which of the collectibles you want to see.
* It's now possible to use the app while playing the game. This is achived by making backups of your savefile, which are then loaded while the game is running.
* Added guides for finding the koroks. The text and images are taken directly from Zeldadungeons interactive map. Simply tap a korok and a guide will pop up. Very helpful for certain korok seeds that are hard to find with only a location on a map.
* When a korok is selected, press B to manually mark it as complete. Very helpful if you're using the app while playing as the latest savedata can't be read. Otherwise you would have to remember all the koroks you've found during this play session, which could become difficult. The correct korok progress will be restored once the game has been closed.
* "Korok paths" have also been added to help finding some koroks. If you've used Zeldadungeons interactive map, then you know what they are.
* Added support for Master Mode. Press Y to toggle it.
* Drastically improved performance when a lot of objects are displayed. Can easily run at 60fps now.
* The app remebers your last camera position and zoom. This makes it easy to get back to the korok you looked at last time, if you for example switch between BotW and the app. (You can also press X in the Homebrew menu to star it for quicker access)
* Now tries to load the last used user so you won't have to use the dialog picker every time you start the app.
* Added more error messages and all logs are saved to a text file in case the app doesn't work
### 1.0
* Initial release

## Building
switch-mesa, switch-glad, switch-freetype and switch-glm are required for building the project. Install them with the command ```pacman -S switch-mesa switch-glad switch-glm switch-freetype```  
Then run ```Make``` to build the .nro file.

### Credits
Huge thanks to these kind people / groups for making this project possible:

* https://github.com/marcrobledo/savegame-editors For most of the data used (savefile hashes and coordinates) and savefile parsing. Also thank you for taking your time to find out edge-cases and verifiying the data :)
* https://github.com/zeldamods/objmap-totk For being a great tool for debugging and for getting names for collectibles
* https://github.com/zeldamods/radar-totk For having data on all ingame objects so I could piece together the "korok paths"
* https://zeldadungeon.net For their amazing interactive map

### Libraries
* https://github.com/nothings/stb For image loading and saving
* https://github.com/nayuki/QR-Code-generator For the QR-code creation
* https://github.com/nlohmann/json For parsing the gamedata from a JSON file