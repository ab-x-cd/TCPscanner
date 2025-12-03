# Utiliser une image de base Linux (Ubuntu)
# Cela garantit que l'environnement est le même sur Windows, Mac et Linux
FROM ubuntu:22.04

# Éviter les interactions lors de l'installation de paquets
ENV DEBIAN_FRONTEND=noninteractive

# 1. Installer les dépendances nécessaires
# build-essential : contient g++, make, etc.
# cmake : pour la configuration du projet
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# 2. Créer le répertoire de travail dans le conteneur
WORKDIR /app

# 3. Copier les fichiers du projet dans le conteneur
COPY . .

# 4. Construire le projet
# On crée un dossier build, on configure avec cmake et on compile
RUN mkdir -p build && cd build && \
    cmake .. && \
    cmake --build .

# 5. Définir la commande par défaut
# On lance l'exécutable compilé
ENTRYPOINT ["./build/tcp_scanner"]
# Arguments par défaut (peuvent être surchargés)
CMD []
