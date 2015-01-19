
#Initialisation

* Forker le dépot sur github pour qu'il soit dans votre compte
* `git clone git@github.com:MehdiMhalla/chessfinder.git`
* Rajouter le dépot d'origine dans les dépots distant : `git remote add upstream https://github.com/viroulep/chessfinder.git`

#Contribuer

##Développer

L'idée de laisser la master aussi "propre" que possible, dans l'idéal il faudrait donc créer une branche par fonctionalité implémentée/testée.

Créer une branche :

* À partir du commit courant `git checkout -b manouvellebranche`
* À partir de la master `git checkout -b manouvellebranche master`

##Publier

Une fois le ou les commit effectués, il faut pousser la branche sur votre dépot : `git push -u origin mabranche` (le "-u" est là pour dire à git que votre blanche locale doit "suivre" la branche distante depuis le dépot origin)
À partir de là, sur github, vous devriez avoir une option "pull request" (PR), et en cliquant dessus vous indiquez au dépot d'origine que vous avez des commits à fusionner. (Et là je reçois un mail et je peux voir et intégrer vos modifications)


#Mettre à jour

Un "fork" ne se maintient pas à jour tout seul, il faut régulièrement se synchroniser avec le dépot d'origine "upstream".

Pour récupérer les mise à jour depuis upstream : `git fetch upstream`

Il faut ensuite les fusionner avec votre master courante

* Se placer sur la master : `git checkout master`
* Fusionner les changements d'upstream : `git rebase upstream/master`

Si vous avez effectués des commits sur la master, ceux ci devraient être rejoués par dessus la master upstream.

Une fois la fusion effectuée, vous pouvez pousser la version à jour de votre dépot sur github : `git push origin master`

##En cas de conflit

Si vous avez des conflits lors du rebase, il faut ouvrir les fichiers concernés, corriger de manière à garder "la" bonne version, ajouter les fichiers corrigés avec `git add`, et utiliser `git rebase --continue` pour continuer l'opération.

##Mise à jour forcée

Si vous n'avez que faire de vos modifications sur la master, vous pouvez aussi forcer la synchronisation avec les opérations suivantes :

* git fetch upstream
* git reset --hard upstream/master
* git push origine +master





