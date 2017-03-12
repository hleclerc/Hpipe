* end of `if` instruction, condition that depends on a user code (It works only when marks are not necessary).
* uninterrupted add_str[...] to produce CbStringPtr instead of copying the data
* read and store `n` bytes (n known at runtime)

* optimisations en rewind
  => ça serait cool de tout baser sur des optimisations a posteriori.
    Pb: quand on fait un rewind, on sait où on atteri. Prop: on fait des contextes spéciaux () uniquement si le contexte non spécial donne des possibilités de chemins supplémentaires
    La question semble se poser de la même manière pour les ok.

Autre prop: on fait un mode clone rewind dans make_inst.

* si on démarre une marque, il faut aller jusqu'au bout et faire l'analyse des rewinds.

Prop: on fait des rewinds comme avant, sauf qu'ils s'arrêtent au niveau des marques.

Différents cas de rewind:
  - on part du début + offset
  - on part de la fin - offset 

  * on a des instructions à exécuter en des points précis
  * il faut donner le contexte de départ