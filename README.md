# BeatShot
This project is a standalone FPS aim trainer that controls target behavior by analyzing user selected music in the background.

**Download Development Build here (BeatShot-Windows.rar) [here](https://github.com/markoleptic/BeatShot/releases/download/v0.2.0/Beatshot-Windows-v0.2.0.rar) or go to Releases**

In Game                    |  Customize Game Mode Menu
:-------------------------:|:-------------------------:
![](https://user-images.githubusercontent.com/86213229/193479200-82c7c70c-517e-4164-908b-2f172d599a75.png)  |  ![](https://user-images.githubusercontent.com/86213229/193479337-fe8a5ed2-0d71-4a5f-b3a2-524c74aa65a3.png)

## BeatShot is designed to enable players to have full control over the game
- standard sensitivity settings (Same as Valorant sensitivity), video & audio settings
- four base gamemodes: continuously spawn targets (MultiBeat), only one at a time (SingleBeat), tracking one target (BeatTrack), and a static grid of activating targets (BeatGrid) that can all be used as templates to create custom game modes with

### Customize the gamemodes using:
- headshot height only mode
- time it takes for target to reach peak color (green) / this is also the point in time that syncs with a beat
- max target lifespan
- min distance between targets
- spawn area height/width
- target spawn cooldown
- mininum and maximum size of targets
- mininum and maximum speed of targets (BeatGrid and BeatGrid derived modes only)
  
 ### Customizing the audio analysis using:
- 1-4 user defined frequency band channels (e.g. 0-87 Hz for a bass channel)
- band limit threshold for each band channel
   - The threshold value changes how sensitive the analyzer is to beats within any given channel. Values greater than Threshold * Average trigger a target to attempt to spawn. higher threshold -> lower analyzer sensitivity -> less targets spawned

## Getting .mp3/.ogg files:
1. To convert Spotify playlists into Youtube playlists, I use [Soundiiz](https://soundiiz.com/) (also works with Apple Music, etc.)
2. You can use [this application](https://github.com/shaked6540/YoutubePlaylistDownloader) to download entire playlists from YouTube. Download and install. *You might need restart the application after you open it for the first time for it to work correctly.*
3. Click on the cog wheel next to the "Insert a youtube link here" text.
4. Select where you want the files to be downloaded to at the top.
5. Under the Audio heading, make sure the box with "Convert videos to" is checked, then select either ogg or mp3.

## What I'm working on / Planned Features:
### Website
- Integrating the game with the [website](https://beatshot.gg)
  - view charts for different game modes by sending http requests to website and opening a browser window in game
    - much easier to create pretty charts using chart js than to try and make them in the game
  - sign up for an account in game to save your scores in a backend database
  - instead of a static patch notes page, replace with in game browser window showing patch notes on website
### Planned changes to base game
- Show example videos for main game modes
- Create AI system tailored to player goals:
  - Choose priority of accuracy, timing, etc.
- Togglable Day/Night mode
- Togglable music visualizations
- Game Optimization (its not great right now)
### Planned game mode changes
- Base game mode difficulties if user doesn't want to mess with customization
  - Needs tuning
- (Maybe) Game mode that cycles between multiple game modes within one song
#### Planned game mode options to add
- spawn targets in quick succession nearby to one another
- change target size based on accuracy/timing accuracy
  - gradually decrease target size if player has hit consecutive targets in a row, but incrementally increase target size when they miss

## More In-Game Screenshots:
Audio Analyzer Settings    |  Customize Game Mode Menu
:-------------------------:|:-------------------------:
![](https://user-images.githubusercontent.com/86213229/193479582-644be4d8-5769-40bd-a018-23b12aa34c4d.png)  |  ![](https://user-images.githubusercontent.com/86213229/193479593-28d8a5d3-e1f8-45e6-a167-679c7985177e.png)

## Collaboration / Copyright
I plan to release this as a free game on Steam in a few months. If you wish to collaborate, I can try to help provide you with more plugin files that might be necessary to run the game in the engine.
Note that copying my work and releasing a game based on my work is a violation of the Unreal Engine UELA and United States Copyright Laws.
