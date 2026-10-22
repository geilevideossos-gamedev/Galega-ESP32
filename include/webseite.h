#pragma once

const char WEBSEITE[] = R"seite(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Galaga</title>
<style>
* {
  box-sizing: border-box;
}
body {
  margin: 0;
  background: #1e1e2e;
  color: white;
  font-family: Arial, sans-serif;
}
.seite {
  max-width: 960px;
  margin: 0 auto;
  padding: 24px 16px;
}
.kopf {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 10px;
  margin-bottom: 16px;
}
h1 {
  margin: 0;
  font-size: 28px;
}
h2 {
  margin: 0 0 10px;
}
.verbindung {
  padding: 6px 14px;
  border-radius: 20px;
  background: #c92a2a;
  font-size: 14px;
  font-weight: bold;
}
.verbindung.ok {
  background: #2f9e44;
}
.karte {
  background: #2a2a3d;
  padding: 24px;
  border-radius: 20px;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
}
#spiel {
  display: block;
  width: 100%;
  border-radius: 12px;
  background: black;
}
.tasteranzeige {
  display: flex;
  flex-wrap: wrap;
  justify-content: center;
  gap: 10px;
  margin-top: 14px;
  font-size: 14px;
}
.punkt {
  padding: 6px 14px;
  border-radius: 20px;
  background: #444;
  transition: 0.1s;
}
.punkt.an {
  background: #1c7ed6;
}
.reihe {
  display: flex;
  flex-wrap: wrap;
  gap: 24px;
  margin-top: 24px;
}
.reihe .karte {
  flex: 1 1 300px;
  text-align: center;
}
.lampe {
  width: 100px;
  height: 100px;
  margin: 20px auto;
  border-radius: 50%;
  background: #444;
  transition: 0.3s;
}
.lampe.an {
  background: #ffd43b;
  box-shadow: 0 0 60px #ffd43b;
}
#ledStatus {
  font-size: 22px;
  font-weight: bold;
}
.knoepfe {
  display: flex;
  justify-content: center;
  gap: 20px;
  margin: 24px 0 16px;
}
button {
  width: 100px;
  padding: 14px;
  font-size: 18px;
  border: none;
  border-radius: 10px;
  color: white;
  cursor: pointer;
}
#an {
  background: #2f9e44;
}
#aus {
  background: #c92a2a;
}
kbd {
  background: white;
  color: black;
  padding: 2px 8px;
  border-radius: 5px;
  font-weight: bold;
}
#temperatur {
  margin: 10px 0 4px;
  font-size: 40px;
  font-weight: bold;
  color: #ff6b6b;
}
#faktor {
  margin-bottom: 14px;
  color: #aaa;
}
#diagramm {
  width: 100%;
  border-radius: 10px;
  background: #1e1e2e;
}
.hinweis {
  color: #aaa;
  font-size: 14px;
}
</style>
</head>
<body>
<div class="seite">
  <div class="kopf">
    <h1>ESP32 Galaga</h1>
    <div id="verbindung" class="verbindung">getrennt</div>
  </div>

  <div class="karte">
    <canvas id="spiel" width="900" height="520"></canvas>
    <div class="tasteranzeige">
      <div id="anzeigeLinks" class="punkt">Links: Taster GPIO 7 / Pfeil links</div>
      <div id="anzeigeSchuss" class="punkt">Schuss: Taster GPIO 2 / Leertaste</div>
    </div>
  </div>

  <div class="reihe">
    <div class="karte">
      <h2>LED</h2>
      <div id="lampe" class="lampe"></div>
      <div id="ledStatus">LED aus</div>
      <div class="knoepfe">
        <button id="an">An</button>
        <button id="aus">Aus</button>
      </div>
      <p class="hinweis">Tastatur: <kbd>E</kbd> = an &nbsp; <kbd>O</kbd> = aus</p>
    </div>

    <div class="karte">
      <h2>Temperatur</h2>
      <div id="temperatur">-- °C</div>
      <div id="faktor">Speed 100 %</div>
      <canvas id="diagramm" width="400" height="200"></canvas>
      <p class="hinweis">Letzte 60 Sekunden</p>
    </div>
  </div>
</div>

<script>
const spielfeld = document.getElementById('spiel');
const pinsel = spielfeld.getContext('2d');
const BREITE = spielfeld.width;
const HOEHE = spielfeld.height;
const PIXEL = 3;
const SCHIFF_Y = HOEHE - 60;
const SCHIFF_TEMPO = 400;
const SCHUSS_TEMPO = 700;
const MAX_SCHUESSE = 2;
const START_LEBEN = 3;

const BILDER = {
  schiff: [
    '......w......',
    '......w......',
    '.....www.....',
    '.....www.....',
    '..r..www..r..',
    '..r.wwwww.r..',
    '..wwwrwrwww..',
    'w.wwwrrrwww.w',
    'wwwwwwwwwwwww',
    'www.rwwwr.www',
    'w....w.w....w'
  ],
  biene: [
    '..b.....b..',
    '...b...b...',
    '..yyyyyyy..',
    '.bbyrrryybb',
    'bbbyyyyybbb',
    'b.byrrryb.b',
    '...yyyyy...',
    '....y.y....'
  ],
  schmetterling: [
    '..w.....w..',
    '...w...w...',
    'rr..bbb..rr',
    'rrr.bwb.rrr',
    '.rrrbbbrrr.',
    '..rrbbbrr..',
    '.rr.bbb.rr.',
    'rr...b...rr'
  ],
  boss: [
    '...g...g...',
    '....ggg....',
    '.ggoooooogg',
    'ggoowoowoog',
    'ggooooooogg',
    '.gg.ooo.gg.',
    'gg...o...gg',
    'g.........g'
  ]
};

const FARBEN = {
  w: 'white',
  r: '#e03131',
  b: '#1c7ed6',
  y: '#fcc419',
  g: '#2f9e44',
  o: '#ffa94d'
};
const FARBEN_BOSS_GETROFFEN = Object.assign({}, FARBEN, { g: '#ae3ec9' });

const TYPEN = {
  biene: { leben: 1, punkte: 50, farbe: '#fcc419' },
  schmetterling: { leben: 1, punkte: 80, farbe: '#e03131' },
  boss: { leben: 2, punkte: 150, farbe: '#2f9e44' }
};

let ws;
let ledIstAn = false;
let temperatur = null;
let werte = [];

let tasterLinks = false;
let tasteLinks = false;

let gestartet = false;
let gameOver = false;
let gameOverSeit = 0;
let punkte = 0;
let leben = START_LEBEN;
let welle = 1;
let welleAnzeige = 0;
let wellePause = 0;
let schiffX = BREITE / 2;
let unverwundbar = 0;
let schwenk = 0;
let naechsterAngriff = 0;
let schuesse = [];
let gegnerSchuesse = [];
let gegner = [];
let funken = [];
let sterne = [];
let highscore = highscoreLaden();
let letzteZeit = null;

function verbinden() {
  ws = new WebSocket('ws://' + location.hostname + ':81/');
  ws.onopen = function() {
    document.getElementById('verbindung').innerText = 'verbunden';
    document.getElementById('verbindung').className = 'verbindung ok';
  };
  ws.onclose = function() {
    document.getElementById('verbindung').innerText = 'getrennt';
    document.getElementById('verbindung').className = 'verbindung';
    tasterLinks = false;
    tasterAnzeigen();
    setTimeout(verbinden, 2000);
  };
  ws.onmessage = function(nachricht) {
    nachrichtVerarbeiten(nachricht.data);
  };
}

function senden(text) {
  if (ws && ws.readyState == WebSocket.OPEN) {
    ws.send(text);
  }
}

function nachrichtVerarbeiten(text) {
  let teile = text.split(':');
  let name = teile[0];
  let wert = teile[1];

  if (name == 'led') {
    ledAnzeigen(wert == '1');
  } else if (name == 'temp') {
    temperaturAnzeigen(parseFloat(wert));
  } else if (name == 'links') {
    tasterLinks = wert == '1';
    tasterAnzeigen();
  } else if (name == 'schuss') {
    document.getElementById('anzeigeSchuss').classList.toggle('an', wert == '1');
    if (wert == '1') {
      schiessen();
    }
  }
}

function ledSchalten(an) {
  if (an != ledIstAn) {
    senden(an ? 'led:1' : 'led:0');
  }
}

function ledAnzeigen(an) {
  ledIstAn = an;
  document.getElementById('ledStatus').innerText = an ? 'LED an' : 'LED aus';
  document.getElementById('lampe').className = an ? 'lampe an' : 'lampe';
}

function temperaturAnzeigen(wert) {
  if (wert < -100) {
    temperatur = null;
    document.getElementById('temperatur').innerText = 'Sensor fehlt';
  } else {
    temperatur = wert;
    document.getElementById('temperatur').innerText = wert.toFixed(1) + ' °C';
  }
  document.getElementById('faktor').innerText = 'Speed ' + Math.round(tempoFaktor() * 100) + ' %';
}

function tempoFaktor() {
  if (temperatur === null) {
    return 1;
  }
  let faktor = 1 + (temperatur - 25) * 0.1;
  return Math.min(2.5, Math.max(0.1, faktor));
}

function spielTempo() {
  return tempoFaktor() * (1 + (welle - 1) * 0.15);
}

function diagrammZeichnen() {
  let leinwand = document.getElementById('diagramm');
  let stift = leinwand.getContext('2d');
  let breite = leinwand.width;
  let hoehe = leinwand.height;
  let rand = 50;

  stift.clearRect(0, 0, breite, hoehe);
  if (werte.length == 0) {
    return;
  }

  let min = Math.floor(Math.min(...werte) - 1);
  let max = Math.ceil(Math.max(...werte) + 1);

  stift.strokeStyle = '#444';
  stift.lineWidth = 1;
  stift.fillStyle = '#aaa';
  stift.font = '12px Arial';
  for (let i = 0; i <= 4; i++) {
    let y = 10 + i * (hoehe - 20) / 4;
    let wert = max - i * (max - min) / 4;
    stift.beginPath();
    stift.moveTo(rand, y);
    stift.lineTo(breite, y);
    stift.stroke();
    stift.fillText(wert.toFixed(1) + ' °C', 0, y + 4);
  }

  stift.strokeStyle = '#ff6b6b';
  stift.lineWidth = 3;
  stift.beginPath();
  for (let i = 0; i < werte.length; i++) {
    let x = rand + i * (breite - rand) / 59;
    let y = 10 + (max - werte[i]) * (hoehe - 20) / (max - min);
    if (i == 0) {
      stift.moveTo(x, y);
    } else {
      stift.lineTo(x, y);
    }
  }
  stift.stroke();
}

function wertSpeichern() {
  if (temperatur === null) {
    return;
  }
  werte.push(temperatur);
  if (werte.length > 60) {
    werte.shift();
  }
  diagrammZeichnen();
}

function highscoreLaden() {
  try {
    return parseInt(localStorage.getItem('galagaHighscore')) || 0;
  } catch (fehler) {
    return 0;
  }
}

function highscoreSpeichern() {
  try {
    localStorage.setItem('galagaHighscore', highscore);
  } catch (fehler) {
  }
}

function zufall(von, bis) {
  return von + Math.random() * (bis - von);
}

function umlauf(wert, laenge) {
  return ((wert % laenge) + laenge) % laenge;
}

function sterneErstellen() {
  let farben = ['white', '#ffd43b', '#74c0fc', '#ff8787'];
  for (let i = 0; i < 90; i++) {
    sterne.push({
      x: Math.random() * BREITE,
      y: Math.random() * HOEHE,
      tempo: zufall(20, 90),
      farbe: farben[i % farben.length]
    });
  }
}

function welleErstellen() {
  let reihen = [
    { typ: 'boss', anzahl: 4 },
    { typ: 'schmetterling', anzahl: 8 },
    { typ: 'schmetterling', anzahl: 8 },
    { typ: 'biene', anzahl: 10 },
    { typ: 'biene', anzahl: 10 }
  ];
  gegner = [];
  for (let reihe = 0; reihe < reihen.length; reihe++) {
    let typ = reihen[reihe].typ;
    let anzahl = reihen[reihe].anzahl;
    for (let spalte = 0; spalte < anzahl; spalte++) {
      let slotX = BREITE / 2 + (spalte - (anzahl - 1) / 2) * 48;
      gegner.push({
        typ: typ,
        leben: TYPEN[typ].leben,
        slotX: slotX,
        slotY: 70 + reihe * 38,
        x: BREITE - slotX,
        y: -40 - reihe * 60 - spalte * 20,
        zustand: 'zurueck'
      });
    }
  }
  welleAnzeige = 2;
  wellePause = 2;
  naechsterAngriff = 4;
}

function neustart() {
  gestartet = true;
  gameOver = false;
  punkte = 0;
  leben = START_LEBEN;
  welle = 1;
  schiffX = BREITE / 2;
  unverwundbar = 0;
  schwenk = 0;
  schuesse = [];
  gegnerSchuesse = [];
  funken = [];
  welleErstellen();
}

function schiessen() {
  if (!gestartet) {
    neustart();
    return;
  }
  if (gameOver) {
    if (performance.now() - gameOverSeit > 1000) {
      neustart();
    }
    return;
  }
  if (schuesse.length < MAX_SCHUESSE) {
    schuesse.push({ x: schiffX, y: SCHIFF_Y - 18 });
  }
}

function formationX(g) {
  return g.slotX + Math.sin(schwenk) * 80;
}

function angriffStarten() {
  let bereit = gegner.filter(g => g.zustand == 'formation');
  let angreifer = gegner.filter(g => g.zustand == 'tauchen');
  if (bereit.length == 0 || angreifer.length >= 1 + welle) {
    return;
  }
  let g = bereit[Math.floor(Math.random() * bereit.length)];
  g.zustand = 'tauchen';
  g.zeit = 0;
  g.startX = g.x;
  g.startY = g.y;
  g.zielX = schiffX;
  g.geschossen = Math.random() < 0.3;
  g.schussY = zufall(g.startY + 40, SCHIFF_Y - 120);
}

function gegnerSchiessen(g) {
  gegnerSchuesse.push({
    x: g.x,
    y: g.y + 12,
    tempoX: Math.max(-150, Math.min(150, (schiffX - g.x) * 0.6)),
    tempoY: 320 * spielTempo()
  });
}

function gegnerBewegen(g, dt, faktor) {
  if (g.zustand == 'formation') {
    g.x = formationX(g);
    g.y = g.slotY;
  } else if (g.zustand == 'zurueck') {
    let dx = formationX(g) - g.x;
    let dy = g.slotY - g.y;
    let abstand = Math.hypot(dx, dy);
    let schritt = 260 * faktor * dt;
    if (abstand <= schritt) {
      g.zustand = 'formation';
      g.x = formationX(g);
      g.y = g.slotY;
    } else {
      g.x += dx / abstand * schritt;
      g.y += dy / abstand * schritt;
    }
  } else if (g.zustand == 'tauchen') {
    g.zeit += dt * faktor;
    g.y += 200 * faktor * dt;
    let anteil = Math.min(1, (g.y - g.startY) / (SCHIFF_Y - g.startY));
    g.x = g.startX + (g.zielX - g.startX) * anteil + Math.sin(g.zeit * 4) * 60;
    if (!g.geschossen && g.y >= g.schussY) {
      g.geschossen = true;
      gegnerSchiessen(g);
    }
    if (g.y > HOEHE + 30) {
      g.zustand = 'zurueck';
      g.x = formationX(g);
      g.y = -30;
    }
  }
}

function explosion(x, y, farbe) {
  for (let i = 0; i < 16; i++) {
    let winkel = Math.random() * Math.PI * 2;
    let tempo = zufall(60, 220);
    funken.push({
      x: x,
      y: y,
      tempoX: Math.cos(winkel) * tempo,
      tempoY: Math.sin(winkel) * tempo,
      zeit: 0.6,
      farbe: farbe
    });
  }
}

function trifft(ax, ay, bx, by, halbeBreite, halbeHoehe) {
  return Math.abs(ax - bx) < halbeBreite && Math.abs(ay - by) < halbeHoehe;
}

function trifftSchiff(x, y, halbeBreite, halbeHoehe) {
  for (let versatz of [-BREITE, 0, BREITE]) {
    if (trifft(x, y, schiffX + versatz, SCHIFF_Y, halbeBreite + 16, halbeHoehe + 14)) {
      return true;
    }
  }
  return false;
}

function gegnerAbgeschossen(g) {
  let wert = TYPEN[g.typ].punkte;
  if (g.zustand == 'tauchen') {
    wert *= 2;
  }
  punkte += wert;
  explosion(g.x, g.y, TYPEN[g.typ].farbe);
}

function schiffGetroffen() {
  leben--;
  explosion(schiffX, SCHIFF_Y, 'white');
  explosion(schiffX, SCHIFF_Y, '#e03131');
  gegnerSchuesse = [];
  schuesse = [];
  if (leben <= 0) {
    gameOver = true;
    gameOverSeit = performance.now();
    if (punkte > highscore) {
      highscore = punkte;
      highscoreSpeichern();
    }
  } else {
    unverwundbar = 2;
  }
}

function schuesseAufGegner() {
  for (let schuss of schuesse) {
    for (let g of gegner) {
      if (!schuss.weg && g.leben > 0 && trifft(schuss.x, schuss.y, g.x, g.y, 18, 16)) {
        schuss.weg = true;
        g.leben--;
        if (g.leben == 0) {
          gegnerAbgeschossen(g);
        }
      }
    }
  }
  schuesse = schuesse.filter(s => !s.weg);
  gegner = gegner.filter(g => g.leben > 0);
}

function schiffKollision() {
  for (let schuss of gegnerSchuesse) {
    if (trifftSchiff(schuss.x, schuss.y, 2, 6)) {
      return true;
    }
  }
  for (let g of gegner) {
    if (g.zustand == 'tauchen' && trifftSchiff(g.x, g.y, 14, 10)) {
      g.leben = 0;
      gegnerAbgeschossen(g);
      gegner = gegner.filter(rest => rest.leben > 0);
      return true;
    }
  }
  return false;
}

function hintergrundBewegen(dt) {
  for (let stern of sterne) {
    stern.y = umlauf(stern.y + stern.tempo * dt, HOEHE);
  }
  for (let funke of funken) {
    funke.x += funke.tempoX * dt;
    funke.y += funke.tempoY * dt;
    funke.zeit -= dt;
  }
  funken = funken.filter(f => f.zeit > 0);
}

function aktualisieren(dt) {
  hintergrundBewegen(dt);
  if (!gestartet || gameOver) {
    return;
  }

  let faktor = spielTempo();
  schwenk += dt * faktor;
  if (welleAnzeige > 0) {
    welleAnzeige -= dt;
  }
  if (unverwundbar > 0) {
    unverwundbar -= dt;
  }

  if (tasterLinks || tasteLinks) {
    schiffX = umlauf(schiffX - SCHIFF_TEMPO * dt, BREITE);
  }

  for (let schuss of schuesse) {
    schuss.y -= SCHUSS_TEMPO * dt;
  }
  schuesse = schuesse.filter(s => s.y > -20);

  for (let schuss of gegnerSchuesse) {
    schuss.x += schuss.tempoX * dt;
    schuss.y += schuss.tempoY * dt;
  }
  gegnerSchuesse = gegnerSchuesse.filter(s => s.y < HOEHE + 20);

  for (let g of gegner) {
    gegnerBewegen(g, dt, faktor);
  }

  naechsterAngriff -= dt * faktor;
  if (naechsterAngriff <= 0) {
    angriffStarten();
    naechsterAngriff = zufall(0.8, 2.5);
  }

  schuesseAufGegner();
  if (unverwundbar <= 0 && schiffKollision()) {
    schiffGetroffen();
  }

  if (gegner.length == 0) {
    wellePause -= dt;
    if (wellePause <= 0) {
      welle++;
      welleErstellen();
    }
  }
}

function bildZeichnen(bild, mitteX, mitteY, farben, groesse) {
  let links = mitteX - bild[0].length * groesse / 2;
  let oben = mitteY - bild.length * groesse / 2;
  for (let zeile = 0; zeile < bild.length; zeile++) {
    for (let spalte = 0; spalte < bild[zeile].length; spalte++) {
      let zeichen = bild[zeile][spalte];
      if (zeichen != '.') {
        pinsel.fillStyle = farben[zeichen];
        pinsel.fillRect(links + spalte * groesse, oben + zeile * groesse, groesse, groesse);
      }
    }
  }
}

function schiffZeichnen() {
  if (gameOver) {
    return;
  }
  if (unverwundbar > 0 && Math.floor(unverwundbar * 10) % 2 == 0) {
    return;
  }
  for (let versatz of [-BREITE, 0, BREITE]) {
    bildZeichnen(BILDER.schiff, schiffX + versatz, SCHIFF_Y, FARBEN, PIXEL);
  }
}

function gegnerZeichnen() {
  for (let g of gegner) {
    let farben = FARBEN;
    if (g.typ == 'boss' && g.leben == 1) {
      farben = FARBEN_BOSS_GETROFFEN;
    }
    bildZeichnen(BILDER[g.typ], g.x, g.y, farben, PIXEL);
  }
}

function schuesseZeichnen() {
  for (let schuss of schuesse) {
    pinsel.fillStyle = '#ffd43b';
    pinsel.fillRect(schuss.x - 1.5, schuss.y - 6, 3, 12);
    pinsel.fillStyle = '#e03131';
    pinsel.fillRect(schuss.x - 1.5, schuss.y - 6, 3, 3);
  }
  for (let schuss of gegnerSchuesse) {
    pinsel.fillStyle = '#ff8787';
    pinsel.fillRect(schuss.x - 2, schuss.y - 5, 4, 10);
    pinsel.fillStyle = 'white';
    pinsel.fillRect(schuss.x - 1, schuss.y - 3, 2, 6);
  }
}

function funkenZeichnen() {
  for (let funke of funken) {
    pinsel.globalAlpha = funke.zeit / 0.6;
    pinsel.fillStyle = funke.farbe;
    pinsel.fillRect(funke.x - 2, funke.y - 2, 4, 4);
  }
  pinsel.globalAlpha = 1;
}

function textMitte(text, y, groesse) {
  pinsel.textAlign = 'center';
  pinsel.font = 'bold ' + groesse + 'px Arial';
  pinsel.fillText(text, BREITE / 2, y);
}

function anzeigeZeichnen() {
  let temperaturText = temperatur === null ? 'kein Sensor' : temperatur.toFixed(1) + ' °C';
  pinsel.fillStyle = 'white';
  pinsel.font = 'bold 18px Arial';
  pinsel.textAlign = 'left';
  pinsel.fillText(temperaturText + '   Speed ' + Math.round(tempoFaktor() * 100) + ' %', 16, 30);
  pinsel.textAlign = 'right';
  let anzeigeHighscore = Math.max(highscore, punkte);
  pinsel.fillText('HI ' + String(anzeigeHighscore).padStart(6, '0') + '   ' + String(punkte).padStart(6, '0'), BREITE - 16, 30);
  pinsel.fillStyle = '#e03131';
  textMitte('WELLE ' + welle, 30, 18);

  for (let i = 0; i < leben - 1; i++) {
    bildZeichnen(BILDER.schiff, 30 + i * 34, HOEHE - 18, FARBEN, 2);
  }
}

function zeichnen() {
  pinsel.fillStyle = 'black';
  pinsel.fillRect(0, 0, BREITE, HOEHE);

  for (let stern of sterne) {
    pinsel.fillStyle = stern.farbe;
    pinsel.fillRect(stern.x, stern.y, 2, 2);
  }

  gegnerZeichnen();
  schuesseZeichnen();
  schiffZeichnen();
  funkenZeichnen();
  anzeigeZeichnen();

  if (!gestartet) {
    pinsel.fillStyle = '#ffd43b';
    textMitte('ESP32 GALAGA', 200, 56);
    pinsel.fillStyle = 'white';
    textMitte('Links-Taster halten = nach links fliegen', 260, 20);
    textMitte('Links raus = rechts wieder rein', 290, 20);
    textMitte('Schuss-Taster oder Leertaste = Start', 340, 20);
  } else if (gameOver) {
    pinsel.fillStyle = 'rgba(30, 30, 46, 0.6)';
    pinsel.fillRect(0, 0, BREITE, HOEHE);
    pinsel.fillStyle = 'white';
    textMitte('GAME OVER', 230, 48);
    textMitte('Schuss-Taster oder Leertaste = Neustart', 275, 20);
  } else if (welleAnzeige > 0) {
    pinsel.fillStyle = '#74c0fc';
    textMitte('WELLE ' + welle, 330, 36);
  }
}

function tasterAnzeigen() {
  document.getElementById('anzeigeLinks').classList.toggle('an', tasterLinks || tasteLinks);
}

function schleife(zeit) {
  if (letzteZeit === null) {
    letzteZeit = zeit;
  }
  let dt = Math.min((zeit - letzteZeit) / 1000, 0.05);
  letzteZeit = zeit;

  aktualisieren(dt);
  zeichnen();
  requestAnimationFrame(schleife);
}

document.getElementById('an').onclick = function() {
  ledSchalten(true);
  this.blur();
};
document.getElementById('aus').onclick = function() {
  ledSchalten(false);
  this.blur();
};

document.addEventListener('keydown', function(taste) {
  if (taste.code == 'Space' || taste.key == 'ArrowLeft') {
    taste.preventDefault();
  }
  if (taste.repeat) {
    return;
  }
  let buchstabe = taste.key.toLowerCase();
  if (buchstabe == 'e') {
    ledSchalten(true);
  } else if (buchstabe == 'o') {
    ledSchalten(false);
  } else if (taste.key == 'ArrowLeft') {
    tasteLinks = true;
    tasterAnzeigen();
  } else if (taste.code == 'Space') {
    schiessen();
  }
});

document.addEventListener('keyup', function(taste) {
  if (taste.key == 'ArrowLeft') {
    tasteLinks = false;
    tasterAnzeigen();
  }
});

window.addEventListener('blur', function() {
  tasteLinks = false;
  tasterAnzeigen();
});

sterneErstellen();
verbinden();
setInterval(wertSpeichern, 1000);
requestAnimationFrame(schleife);
</script>
</body>
</html>
)seite";
