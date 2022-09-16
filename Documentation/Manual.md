
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
Cartouchier se basant des les touches du clavier, peut aller jusque 30 lecteurs.
Dans le panneau "Key Mapping", possibilité de choisir entre trois disposition de clavier : QWERTY, AZERTY, QWERTZ
Possibilité de changer le raccourci clavier, en cliquant sur le bouton "Key" dans le panneau "Clip Editor".
#### Raccourcis :
* Control + click : play / stop
* Molette souris : volume
* Click + glisser : volume
* Double click : réinitialise le volume à 0dB
* Control + molette souris : trim volume
* Alt + click + glisser : déplace un son d'un lecteur à un autre
* Control + alt + click : supprime le son

# Sound Browser
Un double click sur un son le charge dans le premier lecteur vide.
Possibilité de glisser - déposer plusieurs sons : maintenir shift pour les sélectionner, et toujours maintenir shift lors du glisser - déposer.

# Clip Editor

### Raccourcis souris :
* Molette souris : zoom
* Shift + glisser : défilement horizontal
* Control + click : ajoute un point d'enveloppe
* Click droit sur un point : supprime ce point
* Control + click sur le bouton "enveloppe" : réinitialise l'enveloppe

### Raccourcis clavier par défault :
* i : place le point d'entrée
* o : place le point de sortie
* k : supprime le point d'entrée
* l : supprime le point de sortie
* c : lance la pré-écoute au curseur
* x : lance la pré-écoute au début du son
* v : lance la pré-écoute cinq secondes avant la fin du son

# Fenêtre de paramètres (General Settings)
* "Fader maximum value" : définit la valeur maxium du fader du controlleur MIDI ou OSC
* "Fader acceleration" : définit la courbe d'accélération du controleur MIDI ou OSC.
* "Enable fader start" : active ou désactive le lancement des sons en levant le fader du controlleur MIDI.
* "Fader start delay" : ajoute un court délai lors du lancement en fader start via midi, de manière à ne pas "manger" le début du son
* "Fader temporisation" : en mode "Playlist", temps de "faux départ". Si le son est arrêté avant cette durée, la liste de lecture ne passe pas au son suivant.
* "Auto normalize sounds at 0LU" : si activé, les sons se normaliseront automatiquement à 0LU lors de leurs chargement dans un lecteur.
* "Normalisation target" : niveau auquel les sons sont normalisés, en LU.
* "Show individual meter" : affiche un indicateur de niveau pour chaque lecteur.
* "Last seconds warning" : à partir de cette durée restante, le son deviendra rouge
* "Always launch sounds at 0dB" : si activé, le niveau sera ramené à 0dB à chaque lancement de son.
* "Mouse wheel control volume" : si activé, la molette de la souris peut modifier le niveau et le niveau de trim de chaque son, lorsqu'elle est sur le paramètre.
* "Key mapped soundboard colums" : définit le nombre de lecteurs en mode "Key Mapped Soundboard".
* "OSC outgoing port, incoming port, IP" : voir paragraphe OSC.
* "Audio output mode" : Mono : sortie principale à gauche, pré-écoute (cue) à droite. Réduction mono : L+R. Stéréo : sortie principale et pré-écoute sur la même sortie.

# OSC
Pour pouvoir contrôler avec un Smartphone, il faut une [application](https://hexler.net/touchosc-mk1) (5€ sur le Play Store ou sur l’Apple Store).

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
