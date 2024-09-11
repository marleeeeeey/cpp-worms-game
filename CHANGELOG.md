# Change Log

```

*** Start from scratch ***

2024-02-03 1000 Write an OOP wrapper over SDL.
2024-02-04 1030 Separate repository for UTILS.
2024-02-04 1100 Mechanics for scaling level with mouse wheel.
2024-02-05 1230 Mechanics for collisions between cubes in all directions.
2024-02-06 0900 Found a ready set of tilemaps.
2024-02-06 1100 Generate cubes tied to the grid from the tile-map - the base of the level.
2024-02-09 0950 Learn to draw textures on cubes.
2024-02-10 1020 Change the level on the fly to avoid restarting the game because of the editor.
2024-02-11 0800 Divide the level into micro tiles on loading.
2024-02-11 0900 Integrate Box2D for collision calculations.
2024-02-11 1100 Render the angle of inclination of objects from Box2D.
2024-02-11 1215 Make the player movable but subject to physics calculations.
2024-02-11 1330 Add scale between Box2D and SDL.
2024-02-11 1445 Fixed jittering of dynamic tiles clamped between static ones.
2024-02-13 1530 Remove objects that have flown too far away.
2024-02-14 1615 Added InputEventSystem, which helps count the duration of a key press.
2024-02-15 1700 Mechanics of throwing a cube like a grenade (spawn a cube with an angle and applied force).
2024-02-16 0820 Figured out why there are empty lines between tiles (due to fractional scaling).
2024-02-16 0930 Mechanics of aiming at the mouse pointer.
2024-02-16 1045 Added the ability to draw individual tiles by right-clicking the mouse.
2024-02-16 1150 Refactored work with screen, world, and physical world coordinate systems.
2024-02-16 1300 Remove the grenade itself after the explosion.
2024-02-17 1415 Give cubes speed to scatter from the grenade explosion.
2024-02-17 1520 Make a bazooka (included a timer for removing fragments from the map).
2024-02-20 1630 Jump only if there is a cube underfoot.
2024-02-20 1745 Mechanics of smooth return (tracking) of the camera to the moving object.
2024-02-20 1850 Add a background - dark and gloomy.
2024-02-26 2000 Added sounds (voice records for zombie) and music (guitar) to the game.
2024-02-26 2115 Added full screen mode.
2024-02-29 2230 Added filtering of transparent tiles on the map.
2024-02-29 2345 Draw colored pixels instead of figures, this allows taking into account the angle of inclination when rendering.
2024-03-04 0800 Intercept key presses when working in the menu and not send them to the game.
2024-03-04 0915 Calculate the player's capsule in Box2D instead of a rectangle.
2024-03-06 1030 Figured out the technology of Automapping in Tiled.
2024-03-07 1145 Refactored and integrated utils::Config and other utilities.
2024-03-07 1250 Added GameOptions to the game configuration (immediately turned off music by default).
2024-03-07 1350 Indicated background through config.json.
2024-03-08 1450 Support regex or glob for specifying files in config.json.
2024-03-08 1600 Made level destruction like in worms (by pixels).
2024-03-08 1715 Separated program config from assets config.
2024-03-10 1830 Made tag extraction from Aseprite export, and supported Hitbox tag.
2024-03-10 1945 Fix rendering bug of objects with hitboxes.
2024-03-10 2100 Integrated small pixel zombies. Was generated from Aseprite with drafts from ChatGPT.
2024-03-10 2215 Supported randomness in animation using regex expressions for tags in Aseprite.
2024-03-10 2330 Added beautiful exploding particles.
2024-03-13 1015 Define "dead zone" in which the player can move without causing the camera to move.
2024-03-14 1130 Move the player only if there is a cube under him.
2024-03-15 1245 Drop all particles below the level after a specified number of collisions.
2024-03-15 1400 Add shooting with bullets (uzi, pistol).
2024-03-16 1515 Mechanics of switching weapons.
2024-03-16 1630 Added WeaponProps in the code, which allow flexible configuration of weapons.
2024-03-16 1745 Implement TimeToReload for weapons.
2024-03-18 1900 Added AnglePolicy for setting the angle of drawing bullets (VelocityDirection) and player (Fixed).
2024-03-19 2015 Supported updates to Box2D body shapes on the fly during animation change.
2024-03-19 2130 Box2dBodyTuner can now update the body point by point depending on the options provided.
2024-03-20 2245 Started writing naive network code on ASIO directly in the game.
2024-03-24 2355 Compiled network code but realized that I need to look for ready solutions.
2024-03-29 1015 Studied network libraries: studied example of working with SteamNetworkingSockets (Valve).
2024-03-31 1130 Refactored example of working with SteamNetworkingSockets (Valve).
2024-04-03 1245 Integrated SteamNetworkingSockets (Valve) in the game build.
2024-04-04 1400 Studied network libraries: created basic classes for client-server interaction on ASIO.
2024-04-09 1515 Studied network libraries: launched the first version of client-server interaction with handshake on ASIO.
2024-04-10 1630 Drawn static weapon frame.
2024-04-11 1745 Made build under Linux.
2024-04-12 1900 Enabled warnings during the build and fixed several bugs.

*** Start of the LD55 jam ***

2024-04-13 1513 Implement dummy portal catching the player.
2024-04-13 1826 Intergrate evil and portal animations.
2024-04-14 0159 Implement StickyTrap.
2024-04-14 0309 Support DestructionPolicy and ZOrdering: Background, Interiors, Terrain.
2024-04-14 0414 Portal sticky to trap.
2024-04-14 1108 Create fast hybrid explosion fragments.
2024-04-14 1308 Implement portal absorbing fragments.
2024-04-14 1407 Randomize portal speed in time.
2024-04-14 1449 Found a bug with release build during the beta testing (SdCorpse).
2024-04-14 2238 Simplify physic to improve CPU time.
2024-04-14 2316 Fix bug in release with undestructible objects.
2024-04-15 0103 Update collision system. Bullet doesn't collide with Particles.
2024-04-15 0212 Scatter portals if they bump each other.
2024-04-15 1616 Refactor the code. Update documentation with important notes.
2024-04-15 1751 Portal eats player emmediately if it's close enough.
2024-04-15 2338 Fix bug with different speed for players.
2024-04-15 2353 Fix bug when portal searching players incorrectly.
2024-04-16 0105 Portal burst when eats enought. Family member summoing from the portal. All players moving sinchronized.
2024-04-16 0217 Integrate fire animation for bullet and grenade. Remove other weapons. Jump - Space.
2024-04-16 0245 Draw animation for building block.
2024-04-16 0324 One eating counter for all portals. Decrease portal sleep time.
2024-04-16 0411 Create game intruction on start.

*** End of the LD55 jam development. Bug fixing and improvements ***

2024-04-16 1250 Fix start menu bug.
2024-04-16 1317 Remove background from the game.
2024-04-23 1300 Prepare build for web (WASM Emscripten).
2024-04-23 1400 Remove music fade-in/fade-out.
2024-04-26 0215 Add maps from easy to dificult level.
2024-04-27 0228 Improve sound system. Add several sounds (fire, explosion, eating).
2024-04-29 1300 Add layer with indestructible tiles.
2024-04-29 1400 Fix bug with control buttons sticking when reloading the level.
2024-04-29 1500 Improve player jump. Now it depends on the duration of pressing the key.
2024-04-29 1600 Add debug functions for drawing sensors and bounding boxes.
2024-04-29 1700 Enable VSync.
2024-04-30 1800 Disable bullet collisions with the player.
2024-05-01 1900 Major improvement of debugging tools: added the ability to track objects.
2024-05-01 2000 Fix bugs in the explosion mechanics.
2024-05-01 2100 Expand the building block.

*** New iteration of the game development ***

2024-05-02 0100 Update repository with latest changes from LD55.

```
