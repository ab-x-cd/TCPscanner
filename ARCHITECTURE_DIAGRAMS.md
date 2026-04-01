# Diagramme de Classe - Architecture MVC Proposée

```
┌─────────────────────────────────────────────────────────────────┐
│                         MODÈLE (Model)                          │
└─────────────────────────────────────────────────────────────────┘

┌──────────────────────────┐
│   ScanResult             │
├──────────────────────────┤
│ - ip: string             │
│ - port: int              │
│ - isOpen: bool           │
│ - banner: string         │
│ - timestamp: time        │
├──────────────────────────┤
│ + getters/setters()      │
│ + toString(): string     │
└──────────────────────────┘

┌──────────────────────────────────────────┐
│          Validator                       │
├──────────────────────────────────────────┤
│ - (pas d'état)                           │
├──────────────────────────────────────────┤
│ + isValidIPv4(ip: string): bool          │
│ + isValidPortRange(s,e: int): bool       │
│ + validateInput(ip,s,e): void (throws)   │
└──────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│          Scanner                                │
├─────────────────────────────────────────────────┤
│ - timeout: int = 2                              │
│ - bufferSize: int = 1024                        │
├─────────────────────────────────────────────────┤
│ + scanPort(ip, port): ScanResult                │
│ + scanRange(ip, start, end): vector<Result>    │
│ - scan_port_nonblock(...): bool (private)       │
│ - getSocket(): int (private)                    │
│ - extractBanner(...): string (private)          │
└─────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────┐
│                      CONTROLEUR (Controller)                    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│       ScanController                        │
├─────────────────────────────────────────────┤
│ - scanner: Scanner                          │
│ - validator: Validator                      │
│ - view: ConsoleView                         │
├─────────────────────────────────────────────┤
│ + executeScan(ip, start, end): void         │
│ - validateInputs(ip, s, e): void (private)  │
│ - displayResults(...): void (private)       │
└─────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────┐
│                      VUE (View)                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│      ConsoleView                        │
├─────────────────────────────────────────┤
│ - (pas d'état)                          │
├─────────────────────────────────────────┤
│ + displayStartScan(ip, range): void     │
│ + displayResult(result): void           │
│ + displayError(msg): void               │
│ + displayProgress(i, total): void       │
│ + displaySummary(results): void         │
└─────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────┐
│                     MAIN                                        │
└─────────────────────────────────────────────────────────────────┘

main.cpp
├── Récupère les arguments CLI (argc, argv)
├── Crée ScanController
└── Appelle controller.executeScan(ip, start, end)


────────────────────────── RELATIONS ──────────────────────────────

ScanController ──uses──> Validator        (valide les inputs)
ScanController ──uses──> Scanner          (effectue le scan)
ScanController ──uses──> ConsoleView      (affiche les résultats)

Scanner ────returns───> ScanResult        (chaque port scanné)
ScanResult <─ ScanResult ─ vector<Result> (tous les résultats)

ConsoleView ──displays──> ScanResult      (affiche un résultat)
```

---

# Diagramme de Flux d'Utilisation - Cas d'Usage Actuels

```
┌─────────────────────────────────────────────────────────────────┐
│                     UTILISATEUR (Actor)                         │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │
         ┌────────────────────┴────────────────────┐
         │                                         │
         ▼                                         ▼
    ┌─────────────────┐              ┌──────────────────────┐
    │ Lancer le scan  │              │ Tester la validation │
    │  (Use Case 1)   │              │   (Use Case 2)       │
    └─────────────────┘              └──────────────────────┘
         │                                         │
         │ docker run --rm                         │ docker run --rm
         │ tcp-scanner                             │ tcp-scanner
         │ 1.1.1.1 80 80                           │ INVALID_IP 80 80
         │                                         │
         ▼                                         ▼
    ┌─────────────────────────────┐        ┌─────────────────────┐
    │  main.cpp: parseArgs        │        │ Affichage erreur    │
    │  argc=4, argv valides       │        │ "Adresse invalide" │
    └────────────┬────────────────┘        └─────────────────────┘
                 │
                 ▼
    ┌─────────────────────────────┐
    │ ScanController::executeScan │
    │ (ip, startPort, endPort)    │
    └────────────┬────────────────┘
                 │
                 ├─── validate ──────> Validator::isValidIPv4
                 │                        ↓
                 │                    ✓ IP valide
                 │
                 ├─── validate ──────> Validator::isValidPortRange
                 │                        ↓
                 │                    ✓ Ports valides
                 │
                 ├─── scan ──────────> Scanner::scanRange
                 │                        │
                 │                        ├─► scanPort(ip, 80)
                 │                        │   ├─► socket()
                 │                        │   ├─► fcntl() O_NONBLOCK
                 │                        │   ├─► connect()
                 │                        │   ├─► select() timeout=2s
                 │                        │   ├─► send(HEAD /)
                 │                        │   ├─► recv(banner)
                 │                        │   └─► return ScanResult
                 │                        │
                 │                        └─► ScanResult { ip, 80, true, "..." }
                 │
                 ├─── display ───────> ConsoleView::displayStartScan
                 │                        ↓
                 │                    "[*] Scan sur 1.1.1.1 [80-80]..."
                 │
                 ├─── display ───────> ConsoleView::displayResult
                 │                        ↓
                 │                    "[+] Port 80 OUVERT | Service: ..."
                 │
                 └─► Fin d'exécution (exit 0)


──────────────────────── FLUX ALTERNATIF ──────────────────────────

Cas : Ports fermés ou filtrés

main.cpp (tcp-scanner 8.8.8.8 53 53)
         │
         ▼
    ScanController::executeScan("8.8.8.8", 53, 53)
         │
         ├─► Validator OK
         │
         ├─► Scanner::scanPort("8.8.8.8", 53)
         │   └─► timeout 2s, pas de réponse
         │       └─► return ScanResult { "8.8.8.8", 53, false, "" }
         │
         ├─► ConsoleView::displayResult
         │   └─► "[-] Port 53 FERMÉ (ou timeout)"
         │
         └─► exit 0


─────────────────────── CAS D'ERREUR ──────────────────────────────

Cas 1 : IP invalide
  Input: tcp-scanner INVALID_IP 80 80
         │
         ▼
  Validator::isValidIPv4("INVALID_IP") → false
         │
         ▼
  ConsoleView::displayError("Adresse IP invalide")
         │
         ▼
  exit 1

Cas 2 : Plage de ports invalide
  Input: tcp-scanner 127.0.0.1 80 20  (80 > 20)
         │
         ▼
  Validator::isValidPortRange(80, 20) → false
         │
         ▼
  ConsoleView::displayError("Ports invalides (début <= fin requis)")
         │
         ▼
  exit 1

Cas 3 : Arguments manquants
  Input: tcp-scanner 127.0.0.1 80
         │
         ▼
  main.cpp détecte argc != 4
         │
         ▼
  Affiche usage et exit 1
```

---

# Tableau Récapitulatif - Responsabilités Actuelles

| Composant | Responsabilité | Code Actuel |
|-----------|----------------|-------------|
| **main()** | Parsing args | main.cpp: 23-32 |
| **Validation** | Vérifier IP/ports | main.cpp: 30-47 |
| **Scan** | Sockets réseau | main.cpp: 49-156 |
| **Affichage** | Résultats utilisateur | main.cpp: 49-62 |
| **Gestion erreurs** | Messages d'erreur | main.cpp: divers |

**Problème** : Tout dans 1 fichier = difficile à tester et maintenir.

**Solution MVC** : Séparer ces responsabilités en 3 composants indépendants.
