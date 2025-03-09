# TP systèmes de particules

Lancez la commande suivante dans les répertoires des exercices : 
```sh
./run.sh
```

## Exercice 1

Version 1 : fontaine de particules avec geometry shader, sans compute shader, sans collisions entre particules, méthode d'Euler côté CPU

Version 2 et 3 : fontaine de particules avec geometry et compute shader, méthode d'Euler dans le compute shader, collisions entre particules activable ou désactivable avec la touche 'c'

## Exercice 2

Intégration de Verlet pour aider à stabiliser le système, loi de Hooke pour les déformations dans le compute shader, le vent reste fixe