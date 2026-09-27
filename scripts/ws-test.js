#!/usr/bin/env node
// Client WebSocket non-interactif pour tests d'intégration CI : contrairement à
// wscat (qui se ferme immédiatement si stdin n'est pas un TTY, comme dans un job
// GitLab CI), ce script écoute réellement les messages pendant la durée impartie.
// Il réessaie aussi la connexion (retry) car microcks-async-minion peut encore
// être en train de démarrer/charger le contrat AsyncAPI juste après l'injection.
'use strict';

const WebSocket = require('ws');

const [, , url, timeoutMsArg, expectedSubstring] = process.argv;
const timeoutMs = parseInt(timeoutMsArg || '8000', 10);
const retryDelayMs = 1000;

if (!url) {
  console.error('Usage: node ws-test.js <url> [timeoutMs] [expectedSubstring]');
  process.exit(2);
}

const deadline = Date.now() + timeoutMs;
let matched = false;

function connect() {
  const ws = new WebSocket(url);

  ws.on('open', () => {
    console.error(`Connecté à ${url}, en attente de message...`);
  });

  ws.on('message', (data) => {
    const text = data.toString();
    console.log(text);
    if (!expectedSubstring || text.includes(expectedSubstring)) {
      matched = true;
      ws.close();
      process.exit(0);
    }
  });

  ws.on('error', (err) => {
    const remainingMs = deadline - Date.now();
    if (remainingMs <= 0) {
      console.error(`Erreur WebSocket: ${err.message} (délai dépassé, abandon)`);
      process.exit(1);
    }
    console.error(`Erreur WebSocket: ${err.message} (nouvelle tentative dans ${retryDelayMs}ms, ${remainingMs}ms restants)`);
    ws.terminate();
    setTimeout(connect, retryDelayMs);
  });

  ws.on('close', () => {
    if (!matched && Date.now() >= deadline) {
      console.error(`Timeout (${timeoutMs}ms) sans message correspondant reçu sur ${url}`);
      process.exit(1);
    }
  });
}

connect();
