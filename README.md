# TCP Port Scanner (C++)

Un scanner de ports TCP développé en C++ natif sous Linux. Ce projet a été réalisé dans un but d'apprentissage personnel pour explorer la programmation système bas niveau, la gestion des sockets et le protocole TCP/IP.
/!\ Attention : ce projet n'est pas encore adapté à Windows !
## Fonctionnalités

* **Scan de plage de ports :** Analyse séquentielle d'une plage définie par l'utilisateur.
* **Sockets Non-Bloquants :** Utilisation de `fcntl` et `select` pour gérer des timeouts précis (1 seconde) et éviter les blocages indéfinis sur les ports filtrés.
* **Banner Grabbing :** Récupération automatique des bannières de service (ex: versions SSH, Apache, MySQL).
* **Déclenchement HTTP :** Envoi d'une requête `HEAD / HTTP/1.0` pour forcer les serveurs web à s'identifier.
* **Sortie propre :** Affichage formaté des ports ouverts et des services détectés.

## Prérequis

* Environnement Linux (Testé sur Linux Mint).
* Compilateur GCC (`g++`).
* Aucune librairie tierce requise (uniquement la bibliothèque standard C++ et les headers système Linux).

## Installation et Compilation

Cloner le dépôt :

```bash
git clone https://github.com/ab-x-cd/TCPscanner.git
cd TCPscanner
```

Compiler le projet : 

```bash
g++ -Wall main.cpp -o scanner
```

Syntaxe d'utilisation 

```bash
./scanner <IP_CIBLE> <PORT_DEBUT> <PORT_FIN>
```

Exemple : 

```bash
./scanner 127.0.0.1 1 1024
```

## Avertissement
Ce outil est conçu uniquement à des fins éducatives et d'audit de sécurité légitime. L'auteur décline toute responsabilité en cas d'utilisation malveillante ou illégale. Assurez-vous d'avoir l'autorisation explicite avant de scanner une cible réseau.
