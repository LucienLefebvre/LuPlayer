# Trois modes de lecteurs de sons différents :
## One playlist, one cart
#### Liste de lecture (gauche)
Le son prêt à être joué est coloré en orange.
La liste de lecture se contrôle soit avec les faders 1 et 2, soit avec la barre d'espace. Un son se lance avec un des deux
faders, le son suivant se lancera avec l'autre fader. À la fin de la lecture, la liste de lecture passera au son suivant
lorsque l'on ramènera le fader à 0.
On peut également lancer un son avec la barre d’espace, lorsque la lecture est terminée, la liste de lecture passe au
son suivant.
#### Cartouchier (droite) :
Le cartouchier se contrôle avec les faders 3 et 4. Pour assigner un fader à un son, cliquer sur le bouton d'assignation
à droite ou à gauche du son.
Les sons peuvent également se lancer avec des raccourcis clavier (par défaut : Fn1 -> Fn12) , pour les 12 premiers sons. 
Le son prêt à être joué est coloré en orange, il devient vert lorsqu’il est en lecture, et rouge lorsqu’il reste moins de 5
secondes

## Eight faders
Cartouchier simple : chaque son de un à huit est assigné à un fader. Possibilité d'utiliser des raccourcis (par défaut : Fn1 -> Fn8)

### Pour ces deux premiers modes :
* Le son se lancera toujours au début de celui-ci, ou au point d'entrée si il a été définit. Pour pouvoir se déplacer rapidement dans le son, cliquer sur le bouton de pré-écoute (cue). Un click droit sur ce bouton jouera les 5 dernières secondes.
* Possibilité de glisser / déposer les sons entre les lecteurs en faisaint un glisser / déposer à partir du numéro du son (zone bleue)
* Un click droit sur le bouton "HPF" permet de régler la fréquence du filtre coupe-bas.

## Keyboard Mapped
Cartouchier se basant des les touches du clavier, peut aller jusque 30 lecteurs.
Dans le panneau "Key Mapping", possibilité de choisir entre trois disposition de clavier : QWERTY
#### Raccourcis :
* Control + click : play / stop
* Molette souris : volume
* Click + glisser : volume
* Double click : réinitialise le volume à 0dB
* Control + click + glisser : trim volume
* Alt + click + glisser : déplace un son d'un lecteur à un autre
* Control + alt + click : supprime le son

## Enveloppe editor
![Capture 1](https://github.com/LucienLefebvre/LuPlayer/blob/630cbe81b4bb0aebbb4800ef041b2ef893128627/Screenshots/enveloppe.PNG "Capture 1")

#### Raccourcis :
* Molette souris : zoom
* Control + click : ajoute un point d'enveloppe
* Click droit sur un point : supprime ce point
* Control + click sur le bouton "enveloppe" : réinitialise l'enveloppe

## Settings
* "Fader maximum value" : définit la valeur maxium du fader du controlleur MIDI ou OSC
* "Fader acceleration" : définit la courbe d'accélération du controleur MIDI ou OSC.
* "Enable fader start" : active ou désactive le lancement des sons en levant le fader du controlleur MIDI.
* "Fader temporisation" : en mode "Playlist", temps de "faux départ". Si le fader est baissé avant cette durée, la liste de lecture ne passe pas au son suivant.
* "OSC outgoing port, incoming port, IP" : voir paragraphe OSC.
* "Auto normalize sounds at 0LU" : si activé, les sons se normaliseront automatiquement à 0LU lors de leurs chargement dans un lecteur.
* "Show individual meter" : affiche un indicateur de niveau pour chaque lecteur.
* "Always launch sounds at 0dB" : si activé, le niveau sera ramené à 0dB à chaque lancement de son.
* "Mouse wheel control volume" : si activé, la molette de la souris peut modifier le niveau et le niveau de trim de chaque son, lorsqu'elle est sur le paramètre.
* "Key mapped soundboard colums" : définit le nombre de lecteurs en mode "Key Mapped Soundboard".
* "Select converted sounds folder" : définit le dossier où seront plaçés les sons convertis.
* "Audio output mode" : Mono : sortie principale à gauche, pré-écoute (cue) à droite. Réduction mono : L+R. Stéréo : sortie principale et pré-écoute sur la même sortie.

## OSC
Pour pouvoir contrôler avec un Smartphone, il faut une application (5€ sur le Play Storeou sur l’App Store).
Il faut que le téléphone et le PC soit sur le même réseau WIFI / Ethernet.
Configuration : Côté PC :Dans l’application, cliquer sur “Settings”. Il faut donner 3 paramètres : les ports entrants et sortants, et l’adresse IP du téléphone. Dans les ports entrants et sortants, l’usage c’est de mettre 8000 pour Outgoing et 8001 pour Incoming.
Pour l’adresse IP du téléphone, elle est affichée dans le panneau de configuration de TouchOSC.
llfaut ensuite ouvrir les ports dans le pare-feu. Dans Windows, chercher “Pare -feu Windows Defender avec fonctions avancées de sécurité”-> Règles de trafic entrant -> nouvelle règle.Sélectionner “Port”, dans la fenêtre d’après sélectionner “UDP” et mettre port 8001 (pour l’exemple précédent, cela doit correspondre au port Incoming). Faire suivant, “autoriser la connexion”, suivant, autoriser les trois domaines, et enfin donner un nom.
Du côté du portable, il faut juste faire l’inverse : dans TouchOSC, mettre l’IP du PC en Host (pour la trouver, dans Windows clic en bas à droite sur l’icône Ethernet ou WIFI, puis propriété, et descendre tout en bas. Elle est affichée aussi dans le panneau d’options de l’applications, mais il arrive aussi que ce ne soit pas la bonne). Pour les ports, 8001 en Outgoing et 8000 en Incoming.