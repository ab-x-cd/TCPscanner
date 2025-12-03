# TCP Port Scanner (C++)

Un scanner de ports TCP performant développé en C++. Ce projet explore la programmation système bas niveau, la gestion des sockets et le protocole TCP/IP.
Il est désormais **compatible Windows, Linux et Mac** grâce à Docker.

## Fonctionnalités

* **Scan de plage de ports :** Analyse séquentielle d'une plage définie par l'utilisateur.
* **Sockets Non-Bloquants :** Utilisation de `fcntl` et `select` pour gérer des timeouts précis (2 secondes) et éviter les blocages.
* **Banner Grabbing :** Récupération automatique des bannières de service (ex: versions SSH, Apache).
* **Validation des entrées :** Vérification rigoureuse des adresses IP et des plages de ports.
* **Dockerisé :** Fonctionne sur n'importe quel OS via Docker.
* **CI/CD :** Tests automatisés via GitHub Actions.

## 🚀 Utilisation Rapide avec Docker (Recommandé)

C'est la méthode la plus simple pour faire tourner le scanner sur **Windows, Mac ou Linux** sans installer de compilateur.

### 1. Construire l'image
```bash
docker build -t tcp-scanner .
```

### 2. Lancer un scan
**Scanner une IP externe (ex: Google DNS) :**
```bash
docker run --rm tcp-scanner 8.8.8.8 53 53
```

**Scanner votre machine locale (Windows) :**
Trouvez votre IP locale avec `ipconfig` (ex: `192.168.1.50`) puis :
```bash
docker run --rm tcp-scanner 192.168.1.50 135 135
```

**Scanner votre machine locale (Linux) :**
Utilisez l'option `--network host` :
```bash
docker run --rm --network host tcp-scanner 127.0.0.1 20 80
```

## 🛠 Compilation Manuelle (Linux uniquement)

Si vous êtes sous Linux et souhaitez modifier le code :

### Prérequis
* CMake et G++ (`sudo apt install cmake g++`)

### Compilation
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Exécution
```bash
./tcp_scanner <IP_CIBLE> <PORT_DEBUT> <PORT_FIN>
```

## Avertissement
Cet outil est conçu uniquement à des fins éducatives et d'audit de sécurité légitime. L'auteur décline toute responsabilité en cas d'utilisation malveillante ou illégale. Assurez-vous d'avoir l'autorisation explicite avant de scanner une cible réseau.
