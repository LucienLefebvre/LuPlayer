# Trois modes de lecteurs de sons différents :
## One playlist, one cart
Liste de lecture
La liste de lecture se contrôle soit avec les faders 1 et 2, soit avec la barre d'espace. Un son se lance avec un des deux
faders, le son suivant se lancera avec l'autre fader. À la fin de la lecture, la liste de lecture passera au son suivant
lorsque l'on ramènera le fader à 0.\
On peut également lancer un son avec la barre d’espace, lorsque la lecture est terminée, la liste de lecture passe au
son suivant.\
Cartouchier :
Le cartouchier se contrôle avec les faders 3 et 4. Pour assigner un fader à un son, cliquer sur le bouton d'assignation
à droite ou à gauche du son.
Les sons peuvent également se lancer avec des raccourcis clavier (par défaut : Fn1 -> Fn12) , pour les 12 premiers sons. 
Le son prêt à être joué est coloré en orange, il devient vert lorsqu’il est en lecture, et rouge lorsqu’il reste moins de 5
secondes

## Eight faders
Cartouchier simple : chaque son de un à huit est assigné à un fader. Possibilité d'utiliser des raccourcis (par défaut : Fn1 -> Fn8)

Pour ces deux premiers modes :
* Le son se lancera toujours au début de celui-ci, ou au point d'entrée si il a été définit. Pour pouvoir se déplacer rapidement dans le son, cliquer sur le bouton de pré-écoute (cue). Un click droit sur ce bouton jouera les 5 dernières secondes.
* Possibilité de glisser / déposer les sons entre les lecteurs en faisaint un glisser / déposer à partir du numéro du son (zone bleue)
* Un click droit sur le bouton "HPF" permet de régler la fréquence du filtre coupe-bas.

## Keyboard Mapped
Cartouchier se basant des les touches du clavier, peut aller jusque 30 lecteurs.
#### Raccourcis :
* Control + click : play / stop
* Molette souris : volume
* Click + glisser : volume
* Double click : réinitialise le volume à 0dB
* Control + click + glisser : trim volume
* Alt + click + glisser : déplace un son d'un lecteur à un autre
* Control + alt + click : supprime le son

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
