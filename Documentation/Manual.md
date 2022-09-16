
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
* "Audio output mode" : * Mono : main output (mono-reduced L+R) left, cue output right. * Stereo : main output and cue on same stereo output.

# OSC
In order to control LuPlayer with a smartphone, you will need an OSC app (for exemple, templates provided for [touchOSC mk1](https://hexler.net/touchosc-mk1) )

Il faut que le téléphone et le PC soient sur le même réseau WIFI / Ethernet.

### Configuration :
#### Côté PC
Dans l’application, cliquer sur “Settings”. Il faut donner 3 paramètres : les ports entrants et sortants, et l’adresse IP du téléphone.

Dans les ports entrants et sortants, l’usage c’est de mettre 8000 pour Outgoing et 8001 pour Incoming.

Pour l’adresse IP du téléphone, elle est affichée dans le panneau de configuration de TouchOSC.

Il se peut qu'il faille ouvrir les ports dans le pare-feu. Dans Windows, chercher “Pare -feu Windows Defender avec fonctions avancées de sécurité”-> Règles de trafic entrant -> nouvelle règle.

Sélectionner “Port”, dans la fenêtre d’après sélectionner “UDP” et mettre port 8001 (pour l’exemple précédent, cela doit correspondre au port Incoming). Faire suivant, “autoriser la connexion”, suivant, autoriser les trois domaines, et enfin donner un nom.

#### Côté Téléphone
Du côté du portable, il faut juste faire l’inverse : dans TouchOSC, mettre l’IP du PC en Host (pour la trouver, dans Windows clic en bas à droite sur l’icône Ethernet ou WIFI, puis propriété, et descendre tout en bas. Elle est affichée aussi dans le panneau d’options de l’applications, mais il arrive aussi que ce ne soit pas la bonne). Pour les ports, 8001 en Outgoing et 8000 en Incoming.

Des templates se trouvent dans le dossier "TouchOSC templates"

### Liste des commandes OSC (X étant le numéro de lecteur) :
#### Playlist
| Fonction | Commande |
| ----- | ----- |
| Niveau | /1/faderX |
| Nom | /1/soundnameX |
| Temps restant | /1/remainingX |
| Son suivant | /1/down |
| Son précédent | /1/up |
| Revenir au début | /1/reset |

#### Eight Faders
| Fonction | Commande |
| ----- | ----- |
| Niveau | 8fadersgainX |
| Lecture | 8faderspushX |
| Nom | 8faderslabelX |
| Temps restant | 8faderstimeX |

#### KeyMap
| Fonction | Commande |
| ----- | ----- |
| Lecture | kmpushX |
| Temps restant | kmtimeX |
| Nom | kmnameX |
| Raccourci | kmshortcutX |
