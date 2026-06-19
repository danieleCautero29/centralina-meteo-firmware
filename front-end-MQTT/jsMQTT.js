
// CENTRALINA METEO — MQTT
 
// 1. CONFIGURAZIONE MQTT
 
const BROKER    = "broker.hivemq.com";
const PORTA_WS  = 8884;                      // browser usa WebSocket
const CLIENT_ID = "browser-centralina-" + Math.random().toString(16).slice(2, 8);
 
// Topic 
const TOPIC_SENSORI = "centralina/sensori";
const TOPIC_LED     = "centralina/led";
const TOPIC_RGB     = "centralina/rgb";
 
// 2. CONNESSIONE AL BROKER
// mqtt.connect() crea il client e apre la connessione WebSocket
const client = mqtt.connect(`wss://${BROKER}:${PORTA_WS}/mqtt`);
 
client.on('connect', () => {
    console.log("Connesso al broker MQTT!");
 
    // Il browser si iscrive solo ai sensori per ricevere i dati
    // LED e RGB li pubblica lui stesso, non ha bisogno di ascoltarli
    client.subscribe(TOPIC_SENSORI, (err) => {
        if (!err) console.log("Iscritto a: " + TOPIC_SENSORI);
    });
});
 
client.on('error', (err) => {
    console.error("Errore MQTT:", err);
});
 
// 3. RICEZIONE MESSAGGI
// scatta automaticamente ogni volta che arriva un messaggio su un topic sottoscritto
 
client.on('message', (topic, payload) => {
    // payload arriva come Buffer (array di byte), lo convertiamo in stringa
    const messaggio = payload.toString();
 
    if (topic === TOPIC_SENSORI) {
        // Il messaggio è un JSON: {"co2":450,"temperatura":23,"luminosita":600,"umidita":55}
        const dati = JSON.parse(messaggio);
 
        // Aggiorna i testi nella pagina
        document.getElementById('temperatura').textContent = dati.temperatura + ' °C';
        document.getElementById('umidita').textContent     = dati.umidita     + ' %';
        document.getElementById('co2').textContent         = dati.co2         + ' ppm';
        document.getElementById('luminosita').textContent  = dati.luminosita  + ' lux';
 
        // Aggiorna i grafici
        aggiornaGrafico(stato.temperatura, dati.temperatura);
        aggiornaGrafico(stato.umidita, dati.umidita);
        aggiornaGrafico(stato.co2, dati.co2);
        aggiornaGrafico(stato.luminosita, dati.luminosita);
    }
});
 
// 4. STATO DEI GRAFICI
 
const stato = {
    temperatura: { idCanvas: 'graficoTemperatura', etichetta: 'Temp °C',   colore: '#00E676', tipo: 'line', labels: [], data: [], istanza: null },
    umidita:     { idCanvas: 'graficoUmidita',     etichetta: 'Umidità %', colore: '#00B0FF', tipo: 'line', labels: [], data: [], istanza: null },
    co2:         { idCanvas: 'graficoCO2',          etichetta: 'CO2 ppm',   colore: '#FF9100', tipo: 'line', labels: [], data: [], istanza: null },
    luminosita:  { idCanvas: 'graficoLuminosita',  etichetta: 'Lux',       colore: '#FFD600', tipo: 'line', labels: [], data: [], istanza: null },
    led:         { idCanvas: 'graficoLed',          etichetta: 'LED',       colore: '#FFD600', tipo: 'line', labels: [], data: [], istanza: null }
};
 
// 5. CREA (O RICREA) UN GRAFICO (invariato)
 
function creaGrafico(s) {
    if (s.istanza) {
        s.istanza.destroy();
        s.istanza = null;
    }
 
    const ctx = document.getElementById(s.idCanvas).getContext('2d');
    const isLed = (s.idCanvas === 'graficoLed');
 
    let opzioniScale;
    if (s.tipo === 'radar') {
        opzioniScale = {
            r: {
                beginAtZero: true,
                grid:        { color: 'rgba(255,255,255,0.1)' },
                angleLines:  { color: 'rgba(255,255,255,0.1)' },
                pointLabels: { color: '#ffffff', font: { size: 10 } },
                ticks:       { color: '#aaaaaa', backdropColor: 'transparent' }
            }
        };
    } else {
        opzioniScale = {
            y: { beginAtZero: true, grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#aaaaaa' } },
            x: { grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#aaaaaa' } }
        };
        if (isLed) {
            opzioniScale.y.min = 0;
            opzioniScale.y.max = 1;
            opzioniScale.y.ticks.stepSize = 1;
        }
    }
 
    s.istanza = new Chart(ctx, {
        type: s.tipo,
        data: {
            labels: [...s.labels],
            datasets: [{
                label:               s.etichetta,
                data:                [...s.data],
                borderColor:         s.colore,
                backgroundColor:     s.tipo === 'bar' ? s.colore + '88' : s.colore + '22',
                tension:             isLed ? 0 : 0.3,
                stepped:             isLed ? true : false,
                fill:                s.tipo === 'line',
                pointBackgroundColor: s.colore,
                pointRadius:         isLed ? 0 : 4,
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { labels: { color: '#ffffff' } } },
            scales: opzioniScale
        }
    });
}
 
// 6. CAMBIA TIPO GRAFICO (invariato)
 
function cambiaGrafico(s, nuovoTipo) {
    s.tipo = nuovoTipo;
    creaGrafico(s);
}
 
// 7. AGGIORNA I DATI NEL GRAFICO (invariato)
 
function aggiornaGrafico(s, valore) {
    const ora = new Date().toLocaleTimeString([], {
        hour: '2-digit', minute: '2-digit', second: '2-digit'
    });
 
    s.labels.push(ora);
    s.data.push(valore);
 
    if (s.labels.length > 10) {
        s.labels.shift();
        s.data.shift();
    }
 
    if (s.istanza) {
        s.istanza.data.labels          = [...s.labels];
        s.istanza.data.datasets[0].data = [...s.data];
        s.istanza.update();
    }
}
 
// 8. INIZIALIZZAZIONE GRAFICI (invariato)
 
Object.values(stato).forEach(s => creaGrafico(s));
 
// 9. BOTTONI CAMBIO TIPO GRAFICO (invariato)
 
const chiavi   = ['temperatura', 'umidita', 'co2', 'luminosita'];
const tipi     = ['line', 'bar', 'radar'];
const riquadri = document.querySelectorAll('.riquadro');
 
chiavi.forEach((chiave, i) => {
    const bottoni = riquadri[i].querySelectorAll('.grafico');
    bottoni.forEach((btn, j) => {
        btn.addEventListener('click', () => {
            bottoni.forEach(b => b.classList.remove('attivo'));
            btn.classList.add('attivo');
            cambiaGrafico(stato[chiave], tipi[j]);
        });
    });
    bottoni[0].classList.add('attivo');
});
 
// 10. CONTROLLO LED SINGOLO
 
const btnLed   = document.getElementById('btnLed');
const iconaLed = document.getElementById('icona-led');
 
// Teniamo traccia dello stato attuale del LED in una variabile locale
// (con Firebase lo leggevi dal database, ora lo gestiamo in memoria)
let statoLed = false;
 
btnLed.addEventListener('click', () => {
    statoLed = !statoLed;
 
    // Pubblica il nuovo stato sul broker → l'ESP32 lo riceverà e agirà
    client.publish(TOPIC_LED, statoLed ? "true" : "false");
 
    // Aggiorna subito la UI senza aspettare conferma
    aggiornaUIled(statoLed);
    aggiornaGrafico(stato.led, statoLed ? 1 : 0);
});
 
function aggiornaUIled(acceso) {
    if (acceso) {
        btnLed.textContent = 'ACCESO';
        btnLed.className   = 'led-acceso';
        iconaLed.classList.add('acceso');
    } else {
        btnLed.textContent = 'SPENTO';
        btnLed.className   = 'led-spento';
        iconaLed.classList.remove('acceso');
    }
}
 
// 11. CONTROLLO LED RGB
 
const colorPicker = document.getElementById('colorPicker');
const previewRgb  = document.getElementById('preview-rgb');
const spanR       = document.getElementById('valR');
const spanG       = document.getElementById('valG');
const spanB       = document.getElementById('valB');
 
function hexToRgb(hex) {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    return { r, g, b };
}
 
function rgbToHex(r, g, b) {
    return '#' + [r, g, b]
        .map(v => Math.max(0, Math.min(255, v)).toString(16).padStart(2, '0'))
        .join('');
}
 
function aggiornaPreview(r, g, b) {
    const hex = rgbToHex(r, g, b);
    previewRgb.style.backgroundColor = hex;
    previewRgb.style.boxShadow = `0 0 28px rgba(${r}, ${g}, ${b}, 0.6)`;
    spanR.textContent = r;
    spanG.textContent = g;
    spanB.textContent = b;
}
 
// Debounce: aspetta 250ms dopo l'ultimo movimento prima di pubblicare
let debounceTimer;
 
colorPicker.addEventListener('input', () => {
    const { r, g, b } = hexToRgb(colorPicker.value);
 
    // Aggiorna subito la UI (fluido)
    aggiornaPreview(r, g, b);
 
    // Aspetta che l'utente smetta di muovere il picker, poi pubblica
    clearTimeout(debounceTimer);
    debounceTimer = setTimeout(() => {
        const payload = JSON.stringify({ r, g, b });
        client.publish(TOPIC_RGB, payload);
    }, 250);
});
