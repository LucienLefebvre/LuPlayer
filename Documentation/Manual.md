
# Installation
Just download the executable, and launch it.

If, when loaching, you have this message : "VCRUNTIME140_1.dll is not found", you have to download and instal [Microsoft Visual C++ Redistribuable](https://aka.ms/vs/17/release/vc_redist.x64.exe)

# Three differents player mode :
## 1. **One playlist, one cart**
### Playlist (left)
The sound that is ready to be played is colored in orange.
The playlist is controlled either by the first two midi faders, or spacebar. One sound is launched by fader start with one of the two midi faders, and the next will be lauched by the other fader. At the end of one sound, the playlist will skip automatically to the next sound when the fader is brough at 0.

On each player side, a small red square show on which fader is assigned the player (left -> fader 1, right -> fader 2).

Sounds can also be lauched by spacebar. When a sound has finished being played, the playlist skip automatically to the next sound.

To navigate in the playlist, click on the sound number (blue zone) to go at this sound.

Default keyboard shortcuts :

* Page up : previous
* Page down : next 
* Echap : go to first sound

### Cart (right) :

The cart is controlled by midi faders 3 and 4. To assign a fader to a player, click on the assign button at the left (fader 3) or right (fader 4) of a player.

Sounds can also be launched by keyboard shortcuts (by default : Fn1 -> Fn12).

## 2. Eight faders

Simple cart with eight players : each sound is assigned to a midi fader. Sounds can also be launched by keyboard (by default : Fn1 -> Fn8).

### For this two mode :
* Sounds will always be played at the start, or at the in mark if there is one. In order to scrub, click on the cue button and drag cursor. A right click on the cue button will play the last 5 seconds.
* Possibility to drag and drop sounds by dragging from the sound number (blue zone).
* A right click on the high pass filter button (HPF) allows to set the frequency.

## 3. Keyboard Mapped
Cart with up to 30 players, launched by keyboard.
In "Settings -> "Keyboards shortcuts", possibility to choose between differents base keyboard layout : QWERTY, AZERTY and QWERTZ.
Possibility to change each key mappings, by clicking on the button "key" in the clip editor panel.

#### Shortcuts :
* Control + click : play / stop
* Mouse wheel : volume
* Click + drag : volume
* Double click : reset volume at 0dB
* Control + mouse wheel : trim volume
* Alt + click + drag : move a sound from a player to another
* Control + alt + click : delete sound

# Sound Browser
A double click on a sound load it in the first empty player.
Possibility to drag & drop multiple sound : maintain shift to select them, and keep maintening shift while drag & dropping.

# Clip Editor

### Mouse shortcuts :
* Mouse wheel : zoom
* Shift + drag : horizontal scroll
* Control + click : add enveloppe point
* Right click on enveloppe point : delete enveloppe point
* Control + click on the "enveloppe" button : reset the enveloppe

### Default keyboards shortcuts (note that keyboard shortcuts are disabled in keyboard mapped mode) :
* i : set an in mark
* o : set an out mark
* k : delete in mark
* l : delete out mark
* c : launch cue at cursor
* x : launch cue at sound start
* v : launch cue 5 seconds before sound end

# General settings window :
* "Fader maximum value" : set the maximal value of the midi or OSC fader
* "Fader acceleration" : set the acceleration curve of the midi or OSC fader
* "Enable fader start" : enable fader start (sound are launched when a fader is opened)
* "Fader start delay" : add a short delay when launching a sound by fader start, in order to keep transients at the beginning of the sound
* "Fader temporisation" : in playlist mode, time of "false start" : if the sound is stopped before this time, the playlist will not skip to the next sound
* "Auto normalize sounds at 0LU" : if enabled, sound will be automatically normalized in LU when they are loaded in a player
* "Normalization target" : target level for normalization, in LU
* "Show individual meter" : show an individual meter for each player
* "Last seconds warning" : sound will become red from this remaining time
* "Always launch sounds at 0dB" : if enabled, sound level will be bring to 0dB when launched
* "Mouse wheel control volume" : allows mouse wheel to control volume and trim volume of each sound, when hoovering the parameter
* "Key mapped soundboard colums / rows" : set the number of players in "keyboard mapped" mode
* "OSC outgoing port, incoming port, IP" : see OSC paragraphe below.
* "Audio output mode" : Mono : main output (mono-reduced L+R) left, cue output right. Stereo : main output and cue on same stereo output.

# OSC
In order to control LuPlayer with a smartphone, you will need an OSC control app (for exemple, templates provided for [touchOSC mk1](https://hexler.net/touchosc-mk1) )
The OSC app should be on the same network than the PC.

### Configuration :
#### PC side
Three parameters in general settings : outgoing port, incoming port, and IP adress of OSC controller.

For outgoing and incoming port, usage is to set 8000 for outgoing and 8001 for incoming.
The IP adress of the OSC controller should be displayed in the app, for example it can be found on the configuration panel of TouchOSC.

It may be needed to open ports in windows firewall :
In Windows, search "Windows Defender Firewall with Advanced Security", click on "Inbound rules", and click on "New rule" on the right.
Select "Port", in the next window select "UDP" and enter the port set previously in incoming port (8000 for the previous example). Click next, select "Allow the connection", next autorize three domains, and give a name.

#### OSC Controller side (example for TouchOSC)
In touch OSC, just cross the parameters : 8001 in outgoing port, 8000 in incoming port, and you can find the PC IP adress by right clicking on network icon, then properties.

### OSC commands (X being the player number) :
#### Playlist
| Function | Command | Type |
| ----- | ----- | ---- |
| Level | /1/faderX | float32 |
| Name | /1/soundnameX | OSC-String |
| Remaining time | /1/remainingX | OSC-String |
| Next Sound| /1/down | float32 |
| Previous sound | /1/up | float32 |
| Go to first sound | /1/reset | float32 |

#### Eight Faders
| Function | Command | Type |
| ----- | ----- | ---- |
| Level | 8fadersgainX | float32 |
| Play | 8faderspushX | float32 |
| Name | 8faderslabelX | OSC-String |
| Remaining time | 8faderstimeX | OSC-String |

#### KeyMap
| Function | Command | Type |
| ----- | ----- | ---- |
| Play | kmpushX | float32 |
| Remaining Time | kmtimeX | OSC-String |
| Name | kmnameX |  OSC-String |
| Shortcut | kmshortcutX | OSC-String |
