# Installation
Téléchargez simplement l'éxecutable, et lancez le.

Si, au lancement, le message d'erreur "VCRUNTIME140_1.dll est introuvable" apparaît, il faut télécharger et installer [Microsoft Visual C++ Redistribuable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
# Trois modes de lecteurs de sons différents :
## 1. **One playlist, one cart**
### Liste de lecture (gauche)
Le son prêt à être joué est coloré en orange.
La liste de lecture se contrôle soit avec les faders 1 et 2, soit avec la barre d'espace. Un son se lance avec un des deux
faders, le son suivant se lancera avec l'autre fader. À la fin de la lecture, la liste de lecture passera au son suivant
lorsque l'on ramènera le fader à 0.

De chaque côté du lecteur, un petit carré rouge indique sur quel fader est assigné le lecteur.

On peut également lancer un son avec la barre d’espace, lorsque la lecture est terminée, la liste de lecture passe au
son suivant.

Pour naviguer dans la liste de lecture :
* Cliquer sur le numéro d'un son pour aller directement à ce son.

Raccourcis clavier par défault : 
* Page up : précédent
* Page down : suivant 
* Echap : retour au début

### Cartouchier (droite) :
Le cartouchier se contrôle avec les faders 3 et 4. Pour assigner un fader à un son, cliquer sur le bouton d'assignation
à gauche (fader 3) ou à droite (fader 4) du son.

Les sons peuvent également se lancer avec des raccourcis clavier (par défaut : Fn1 -> Fn12) , pour les 12 premiers sons. 

Le son prêt à être joué est coloré en orange, il devient vert lorsqu’il est en lecture, et rouge lorsqu’il reste moins de 5
secondes

## 2. Eight faders

Cartouchier simple : chaque son de un à huit est assigné à un fader. Possibilité d'utiliser des raccourcis (par défaut : Fn1 -> Fn8)

### Pour ces deux premiers modes :
* Le son se lancera toujours au début de celui-ci, ou au point d'entrée si il a été définit. Pour pouvoir se déplacer rapidement dans le son, cliquer sur le bouton de pré-écoute (cue). Un click droit sur ce bouton jouera les 5 dernières secondes.
* Possibilité de placer un point d'entrée ou de sortie : placer le curseur de pré-écoute, puis cliquer sur le  bouton bleu ou jaune (clique droit pour supprimer).
* Possibilité de glisser / déposer les sons entre les lecteurs en faisaint un glisser / déposer à partir du numéro du son (zone bleue)
* Un click droit sur le bouton "HPF" permet de régler la fréquence du filtre coupe-bas.

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

