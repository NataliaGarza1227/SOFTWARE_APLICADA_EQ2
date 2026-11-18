/*
 * Created by ArduinoGetStarted.com
 * Modificado y rediseñado por Gemini - Panel de Rehabilitación con Pestañas e Historial Editable
 */

const char *HTML_CONTENT = R"=====(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Panel de Rehabilitación</title>
<style>
    :root {
        --primary-color: #4A90E2;
        --secondary-color: #50E3C2;
        --background-color: #F4F7F6;
        --card-bg: #FFFFFF;
        --text-main: #333333;
        --text-muted: #777777;
        --border-radius: 12px;
        --shadow: 0 4px 6px rgba(0,0,0,0.1);
        --font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    }

    body {
        font-family: var(--font-family);
        background-color: var(--background-color);
        color: var(--text-main);
        margin: 0;
        padding: 20px;
        display: flex;
        flex-direction: column;
        align-items: center;
    }

    h1 {
        color: var(--primary-color);
        margin-bottom: 5px;
    }

    .subtitle {
        color: var(--text-muted);
        margin-top: 0;
        margin-bottom: 20px;
    }

    /* Sistema de Pestañas */
    .tab-bar {
        display: flex;
        gap: 10px;
        width: 100%;
        max-width: 1100px;
        margin-bottom: 20px;
    }

    .tab-btn {
        background-color: #E2E8F0;
        color: var(--text-main);
        border: none;
        padding: 10px 20px;
        border-radius: 8px;
        cursor: pointer;
        font-weight: bold;
        flex: 1;
        transition: background-color 0.3s;
    }

    .tab-btn.active {
        background-color: var(--primary-color);
        color: white;
    }

    .view-container {
        width: 100%;
        max-width: 1100px;
        display: none;
    }

    .view-container.active {
        display: block;
    }

    .card {
        background-color: var(--card-bg);
        border-radius: var(--border-radius);
        padding: 20px;
        box-shadow: var(--shadow);
        margin-bottom: 20px;
    }

    /* Vista Monitoreo (Grid) */
    .monitor-grid {
        display: grid;
        grid-template-columns: 1fr 3fr;
        gap: 20px;
    }

    @media (max-width: 768px) {
        .monitor-grid { grid-template-columns: 1fr; }
    }

    input[type="text"] {
        width: 100%;
        padding: 10px;
        border: 1px solid #ccc;
        border-radius: 6px;
        box-sizing: border-box;
        font-family: inherit;
        margin-bottom: 10px;
    }

    button {
        background-color: var(--primary-color);
        color: white;
        border: none;
        padding: 12px;
        border-radius: 6px;
        font-size: 15px;
        cursor: pointer;
        font-weight: bold;
        transition: background-color 0.3s;
        width: 100%;
    }

    button:hover { background-color: #357ABD; }
    button.secondary { background-color: var(--secondary-color); color: #000; }
    button.secondary:hover { background-color: #38C6A5; }
    button.danger { background-color: #E74C3C; }
    button.danger:hover { background-color: #C0392B; }
    button.small { padding: 6px 10px; font-size: 12px; width: auto; }

    /* Listas y Tablas de Pacientes */
    .patient-table {
        width: 100%;
        border-collapse: collapse;
        margin-top: 10px;
    }

    .patient-table th, .patient-table td {
        padding: 12px;
        text-align: left;
        border-bottom: 1px solid #eee;
    }

    .patient-table th { background-color: #F8FAFC; color: var(--text-muted); }

    .history-card {
        background: #FAFAFA;
        border: 1px solid #E2E8F0;
        border-radius: 8px;
        padding: 15px;
        margin-bottom: 10px;
        display: flex;
        justify-content: space-between;
        align-items: center;
    }

    /* Dashboard de Métricas y Gráficas */
    .dashboard {
        display: flex;
        flex-direction: column;
        gap: 20px;
    }

    .metrics-container {
        display: flex;
        gap: 15px;
        justify-content: space-between;
    }

    .metric-box {
        background: var(--card-bg);
        border: 2px solid var(--primary-color);
        border-radius: var(--border-radius);
        padding: 15px;
        flex: 1;
        text-align: center;
    }
    
    .metric-box.flex-sensor { border-color: var(--secondary-color); }

    .metric-title {
        font-size: 13px;
        color: var(--text-muted);
        text-transform: uppercase;
        letter-spacing: 1px;
    }

    .metric-value {
        font-size: 26px;
        font-weight: bold;
        margin: 8px 0;
    }

    .metric-unit {
        font-size: 13px;
        color: var(--text-muted);
    }

    .canvas-container {
        width: 100%;
        height: 380px;
        position: relative;
    }

    canvas {
        width: 100%;
        height: 100%;
        border-radius: 8px;
        background-color: #FAFAFA;
        border: 1px solid #eee;
    }

    .ws-status {
        text-align: center;
        margin-top: 15px;
        font-size: 14px;
        color: var(--text-muted);
    }

    .status-dot {
        height: 10px;
        width: 10px;
        background-color: #ccc;
        border-radius: 50%;
        display: inline-block;
        margin-right: 5px;
    }
    
    .status-connected { background-color: #2ECC71; }
    .status-disconnected { background-color: #E74C3C; }
</style>
</head>
<body>

    <h1>Panel de Rehabilitación</h1>
    <p class="subtitle">Monitoreo Clínico y Gestión de Pacientes</p>

    <!-- Pestañas de Navegación -->
    <div class="tab-bar">
        <button id="tab-monitor-btn" class="tab-btn active" onclick="switchView('monitor')">Monitoreo en Vivo</button>
        <button id="tab-patients-btn" class="tab-btn" onclick="switchView('patients')">Gestión e Historial de Pacientes</button>
    </div>

    <!-- VISTA 1: MONITOREO -->
    <div id="view-monitor" class="view-container active">
        <div class="monitor-grid">
            
            <!-- Controles Izquierdos -->
            <div class="card">
                <h3>Control de Sesión</h3>
                <div style="font-size: 14px; margin-bottom: 15px;">
                    Paciente Activo: <br><span id="current-patient-label" style="font-weight: bold; color: var(--primary-color); font-size: 16px;">Ninguno</span>
                </div>
                
                <button id="bt_session" onclick="toggleSession()">Iniciar Sesión</button>
                <button id="bt_mcv" onclick="calculateMCV()" class="secondary" style="display:none; margin-top: 10px;">Actualizar MCV</button>
                
                <button id="bt_connect" onclick="connect_onclick();" style="margin-top: 30px;" class="danger">Conectar Dispositivo</button>
                
                <div class="ws-status">
                    <span id="ws_dot" class="status-dot"></span>
                    <span id="ws_state">Desconectado</span>
                </div>
            </div>

            <!-- Gráficas y Métricas Derechas -->
            <div class="dashboard">
                <div class="metrics-container">
                    <div class="metric-box">
                        <div class="metric-title">Señal sEMG</div>
                        <div class="metric-value" id="val_emg_mv" style="color: var(--primary-color);">0.00</div>
                        <div class="metric-unit">Milivoltios (mV)</div>
                    </div>
                    
                    <div class="metric-box">
                        <div class="metric-title">MCV (Paciente)</div>
                        <div class="metric-value" id="val_mcv">0.00</div>
                        <div class="metric-unit">mV Máximos</div>
                    </div>

                    <div class="metric-box flex-sensor">
                        <div class="metric-title">Sensor de Flexión</div>
                        <div class="metric-value" id="val_flex_adc" style="color: var(--secondary-color);">0</div>
                        <div class="metric-unit">ADC / <span id="val_flex_pct" style="font-weight:bold;">0%</span></div>
                    </div>
                </div>

                <div class="card canvas-container" style="padding: 10px; margin-bottom:0;">
                    <canvas id="graph"></canvas>
                </div>
            </div>
        </div>
    </div>

    <!-- VISTA 2: GESTIÓN DE PACIENTES E HISTORIALES -->
    <div id="view-patients" class="view-container">
        <div class="card">
            <h3>Registro de Nuevo Paciente</h3>
            <div style="display: flex; gap: 10px;">
                <input type="text" id="new-patient-name" placeholder="Nombre completo del paciente" style="margin-bottom:0;">
                <button onclick="addPatient()" class="secondary" style="width: 200px;">Agregar Paciente</button>
            </div>
        </div>

        <div class="card">
            <h3>Directorio de Pacientes</h3>
            <table class="patient-table">
                <thead>
                    <tr>
                        <th>Paciente</th>
                        <th>MCV Actual</th>
                        <th>Sesiones Registradas</th>
                        <th>Acciones</th>
                    </tr>
                </thead>
                <tbody id="patient-table-body">
                    <!-- Relleno dinámico -->
                </tbody>
            </table>
        </div>

        <div class="card" id="history-details-card" style="display:none;">
            <h3>Historial de Sesiones: <span id="history-patient-title" style="color:var(--primary-color);"></span></h3>
            <div id="history-sessions-container" style="max-height: 300px; overflow-y: auto; margin-top: 15px;">
                <!-- Relleno dinámico del historial -->
            </div>
        </div>
    </div>

<script>
var WSP_WIDTH = 0;
var WSP_HEIGHT = 0;
var MAX_SAMPLE = 100; 
var COLOR_TEXT = "#333333";
var COLOR_GRIDLINE = "#EAEAEA";
var COLOR_LINE = ["#4A90E2", "#00B894"]; 

var canvas;
var ctx;
var sample_count = 0;
var buffer = "";
var data = [[], []]; 

var ws = null;
var isSessionActive = false;

const ADC_RESOLUTION = 16383.0; 
const VREF = 5000.0; 
var currentPatient = null;
var patientsData = {}; 

function init() {
    canvas = document.getElementById("graph");
    ctx = canvas.getContext("2d");
    
    if(localStorage.getItem('rehab_patients_history')) {
        patientsData = JSON.parse(localStorage.getItem('rehab_patients_history'));
        updatePatientsUI();
    }

    canvas_resize();
    window.addEventListener('resize', canvas_resize);
    setInterval(update_view, 1000 / 30); 
}

// --- Cambio de Pestañas ---
function switchView(viewName) {
    document.getElementById('view-monitor').classList.remove('active');
    document.getElementById('view-patients').classList.remove('active');
    document.getElementById('tab-monitor-btn').classList.remove('active');
    document.getElementById('tab-patients-btn').classList.remove('active');

    if(viewName === 'monitor') {
        document.getElementById('view-monitor').classList.add('active');
        document.getElementById('tab-monitor-btn').classList.add('active');
        setTimeout(canvas_resize, 50); // Recalcular tamaño al volver al canvas
    } else {
        document.getElementById('view-patients').classList.add('active');
        document.getElementById('tab-patients-btn').classList.add('active');
        updatePatientsUI();
    }
}

// --- Gestión de Pacientes ---
function addPatient() {
    const nameInput = document.getElementById('new-patient-name');
    const name = nameInput.value.trim();
    
    if(name === "") { alert("Por favor ingrese un nombre."); return; }
    if(patientsData[name]) { alert("El paciente ya existe."); return; }

    patientsData[name] = { mcv: 0.0, sessions: [] };
    savePatients();
    nameInput.value = "";
    updatePatientsUI();
    selectPatient(name);
    alert("Paciente agregado exitosamente.");
}

function deletePatient(name, event) {
    event.stopPropagation();
    if(confirm(`¿Está seguro de eliminar el expediente completo de ${name}?`)) {
        delete patientsData[name];
        if(currentPatient === name) {
            currentPatient = null;
            document.getElementById('current-patient-label').innerText = "Ninguno";
            document.getElementById('val_mcv').innerText = "0.00";
            document.getElementById('history-details-card').style.display = "none";
        }
        savePatients();
        updatePatientsUI();
    }
}

function selectPatient(name) {
    currentPatient = name;
    document.getElementById('current-patient-label').innerText = name;
    document.getElementById('val_mcv').innerText = patientsData[name].mcv.toFixed(2);
    updatePatientsUI();
    renderHistoryView(name);
}

function savePatients() {
    localStorage.setItem('rehab_patients_history', JSON.stringify(patientsData));
}

function updatePatientsUI() {
    const tbody = document.getElementById('patient-table-body');
    tbody.innerHTML = "";

    const keys = Object.keys(patientsData);
    if(keys.length === 0) {
        tbody.innerHTML = `<tr><td colspan="4" style="text-align: center; color: gray;">No hay pacientes registrados</td></tr>`;
        return;
    }

    for (const name of keys) {
        const p = patientsData[name];
        const tr = document.createElement('tr');
        if(currentPatient === name) tr.style.backgroundColor = "#F0F4F8";
        
        tr.innerHTML = `
            <td><strong>${name}</strong></td>
            <td>${p.mcv.toFixed(2)} mV</td>
            <td>${p.sessions.length} sesiones</td>
            <td>
                <button class="small secondary" onclick="selectPatient('${name}'); switchView('monitor');">Monitorear</button>
                <button class="small secondary" onclick="selectPatient('${name}');">Ver Historial</button>
                <button class="small danger" onclick="deletePatient('${name}', event)">Eliminar</button>
            </td>
        `;
        tbody.appendChild(tr);
    }
}

function renderHistoryView(name) {
    const card = document.getElementById('history-details-card');
    const title = document.getElementById('history-patient-title');
    const container = document.getElementById('history-sessions-container');
    
    card.style.display = "block";
    title.innerText = name;
    container.innerHTML = "";

    const sessions = patientsData[name].sessions;
    if(sessions.length === 0) {
        container.innerHTML = `<p style="color: gray; text-align: center;">Este paciente aún no cuenta con sesiones guardadas.</p>`;
        return;
    }

    for(let i = sessions.length - 1; i >= 0; i--) {
        const s = sessions[i];
        const div = document.createElement('div');
        div.className = "history-card";
        div.innerHTML = `
            <div>
                <strong>Fecha: ${s.date}</strong><br>
                <span style="color: #555;">Pico sEMG Máx: <strong>${s.maxEmg.toFixed(2)} mV</strong> | Flexión Máx: <strong>${s.maxFlexPct}%</strong></span>
            </div>
            <button class="small danger" onclick="deleteSession('${name}', ${i})">Borrar Sesión</button>
        `;
        container.appendChild(div);
    }
}

function deleteSession(patientName, sessionIndex) {
    if(confirm("¿Desea eliminar este registro de sesión?")) {
        patientsData[patientName].sessions.splice(sessionIndex, 1);
        savePatients();
        renderHistoryView(patientName);
        updatePatientsUI();
    }
}

// --- Lógica Clínica y MCV ---
function calculateMCV() {
    if(!currentPatient) { alert("Seleccione un paciente primero."); return; }
    
    if(data[0].length > 0) {
        const currentMax = Math.max(...data[0]);
        if(currentMax > patientsData[currentPatient].mcv) {
            patientsData[currentPatient].mcv = currentMax;
            savePatients();
            document.getElementById('val_mcv').innerText = currentMax.toFixed(2);
            updatePatientsUI();
            alert("Nueva MCV registrada con éxito.");
        } else {
            alert("El pico actual (" + currentMax.toFixed(2) + " mV) no supera la MCV guardada.");
        }
    } else {
        alert("No hay datos suficientes para calcular la MCV.");
    }
}

function updateMetricsUI(emg_raw, flex_raw) {
    let emg_mv = (emg_raw / ADC_RESOLUTION) * VREF;
    let flex_pct = (flex_raw / ADC_RESOLUTION) * 100.0;
    if(flex_pct > 100) flex_pct = 100;
    if(flex_pct < 0) flex_pct = 0;

    document.getElementById('val_emg_mv').innerText = emg_mv.toFixed(2);
    document.getElementById('val_flex_adc').innerText = Math.round(flex_raw);
    document.getElementById('val_flex_pct').innerText = flex_pct.toFixed(1) + "%";
}

// --- Control de Sesión ---
function toggleSession() {
    if(!currentPatient) {
        alert("Por favor, seleccione o cree un paciente antes de iniciar la sesión.");
        switchView('patients');
        return;
    }

    isSessionActive = !isSessionActive;
    const btn = document.getElementById('bt_session');
    const mcvBtn = document.getElementById('bt_mcv');
    
    if(isSessionActive) {
        btn.innerText = "Detener Sesión";
        btn.classList.add("danger");
        mcvBtn.style.display = "block";
        data = [[], []]; 
        sample_count = 0;
        buffer = "";
    } else {
        if(data[0].length > 0) {
            let maxEmgSession = Math.max(...data[0]);
            let maxFlexAdcSession = Math.max(...data[1]);
            let maxFlexPctSession = ((maxFlexAdcSession / ADC_RESOLUTION) * 100).toFixed(1);
            
            const now = new Date();
            const dateStr = now.toLocaleDateString() + " " + now.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'});

            if(!patientsData[currentPatient].sessions) {
                patientsData[currentPatient].sessions = [];
            }

            patientsData[currentPatient].sessions.push({
                date: dateStr,
                maxEmg: maxEmgSession,
                maxFlexPct: maxFlexPctSession
            });

            savePatients();
            updatePatientsUI();
            alert("Sesión finalizada y guardada en el historial del paciente.");
        }

        btn.innerText = "Iniciar Sesión";
        btn.classList.remove("danger");
        mcvBtn.style.display = "none";
    }
}

// --- WebSocket ---
function connect_onclick() {
    if (ws == null) {
        ws = new WebSocket("ws://" + window.location.host + ":81");
        document.getElementById("ws_state").innerText = "Conectando...";
        ws.onopen = ws_onopen;
        ws.onclose = ws_onclose;
        ws.onmessage = ws_onmessage;
    } else {
        ws.close();
    }
}

function ws_onopen() {
    document.getElementById("ws_state").innerText = "Conectado";
    document.getElementById("ws_dot").className = "status-dot status-connected";
    const btn = document.getElementById("bt_connect");
    btn.innerText = "Desconectar";
    btn.classList.remove("danger");
    btn.classList.add("secondary");
}

function ws_onclose() {
    document.getElementById("ws_state").innerText = "Desconectado";
    document.getElementById("ws_dot").className = "status-dot status-disconnected";
    const btn = document.getElementById("bt_connect");
    btn.innerText = "Conectar Dispositivo";
    btn.classList.remove("secondary");
    btn.classList.add("danger");
    ws = null;
    if(isSessionActive) toggleSession();
}

function ws_onmessage(e_msg) {
    if(!isSessionActive) return;

    buffer += e_msg.data;
    buffer = buffer.replace(/\r\n/g, "\n").replace(/\r/g, "\n");

    while(buffer.indexOf("\n") == 0) buffer = buffer.substring(1);
    if(buffer.indexOf("\n") <= 0) return;

    var pos = buffer.lastIndexOf("\n");
    var str = buffer.substring(0, pos);
    var new_sample_arr = str.split("\n");
    buffer = buffer.substring(pos + 1);

    let last_emg = 0;
    let last_flex = 0;

    for(var si = 0; si < new_sample_arr.length; si++) {
        var arr = new_sample_arr[si].trim().split(/[\s\t]+/);
        
        if(arr.length >= 2) {
            let raw_emg = parseFloat(arr[0]);
            let raw_flex = parseFloat(arr[1]);

            if(!isNaN(raw_emg) && !isNaN(raw_flex)) {
                let emg_mv = (raw_emg / ADC_RESOLUTION) * VREF;

                data[0].push(emg_mv);
                data[1].push(raw_flex); 

                last_emg = raw_emg;
                last_flex = raw_flex;
                sample_count++;
            }
        }
    }

    updateMetricsUI(last_emg, last_flex);

    for(var line = 0; line < data.length; line++) {
        while(data[line].length > MAX_SAMPLE) {
            data[line].shift();
        }
    }
}

function canvas_resize() {
    const container = document.querySelector('.canvas-container');
    if(container) {
        WSP_WIDTH = container.clientWidth;
        WSP_HEIGHT = container.clientHeight;
        canvas.width = WSP_WIDTH;
        canvas.height = WSP_HEIGHT;
    }
}

// --- Renderizado del Gráfico con Ejes ---
function update_view() {
    if(!ctx) return;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    const paddingLeft = 65;   
    const paddingBottom = 40; 
    const paddingTop = 25;
    const paddingRight = 20;

    const plotWidth = canvas.width - paddingLeft - paddingRight;
    const plotHeight = canvas.height - paddingTop - paddingBottom;

    if(plotWidth <= 0 || plotHeight <= 0) return;

    let minEMG = 0, maxEMG = 5000;
    let minFlex = 0, maxFlex = 16383;

    if(data[0].length > 0) {
        let m0 = Math.min(...data[0]);
        let M0 = Math.max(...data[0]);
        if(M0 > maxEMG) maxEMG = M0 * 1.1;
        if(m0 < minEMG) minEMG = Math.max(0, m0 * 0.9);
    }
    if(data[1].length > 0) {
        let m1 = Math.min(...data[1]);
        let M1 = Math.max(...data[1]);
        if(M1 > maxFlex) maxFlex = M1 * 1.05;
    }

    ctx.font = "11px " + getComputedStyle(document.body).fontFamily;
    ctx.lineWidth = 1;

    const stepsY = 4;
    for(let i = 0; i <= stepsY; i++) {
        let yPx = paddingTop + (plotHeight / stepsY) * i;
        let valY = maxEMG - (i / stepsY) * (maxEMG - minEMG);

        ctx.strokeStyle = COLOR_GRIDLINE;
        ctx.beginPath();
        ctx.moveTo(paddingLeft, yPx);
        ctx.lineTo(canvas.width - paddingRight, yPx);
        ctx.stroke();

        ctx.fillStyle = COLOR_TEXT;
        ctx.textAlign = "right";
        ctx.textBaseline = "middle";
        ctx.fillText(valY.toFixed(0) + " mV", paddingLeft - 8, yPx);
    }

    for(let i = 0; i <= stepsY; i++) {
        let yPx = paddingTop + (plotHeight / stepsY) * i;
        let valFlexY = maxFlex - (i / stepsY) * (maxFlex - minFlex);

        ctx.fillStyle = COLOR_LINE[1];
        ctx.textAlign = "left";
        ctx.textBaseline = "middle";
        ctx.fillText(valFlexY.toFixed(0), canvas.width - paddingRight + 8, yPx);
    }

    const stepsX = 5;
    let currentSampleIndex = sample_count;
    let startSample = Math.max(0, currentSampleIndex - MAX_SAMPLE);

    for(let i = 0; i <= stepsX; i++) {
        let xPx = paddingLeft + (plotWidth / stepsX) * i;
        let sampleLabel = Math.round(startSample + (i / stepsX) * Math.min(sample_count, MAX_SAMPLE));

        ctx.strokeStyle = COLOR_GRIDLINE;
        ctx.beginPath();
        ctx.moveTo(xPx, paddingTop);
        ctx.lineTo(xPx, canvas.height - paddingBottom);
        ctx.stroke();

        ctx.fillStyle = COLOR_TEXT;
        ctx.textAlign = "center";
        ctx.textBaseline = "top";
        ctx.fillText(sampleLabel, xPx, canvas.height - paddingBottom + 8);
    }

    ctx.save();
    ctx.fillStyle = COLOR_TEXT;
    ctx.font = "bold 12px " + getComputedStyle(document.body).fontFamily;
    
    ctx.textAlign = "center";
    ctx.fillText("Muestras (Tiempo)", paddingLeft + plotWidth / 2, canvas.height - 8);
    
    ctx.translate(15, paddingTop + plotHeight / 2);
    ctx.rotate(-Math.PI / 2);
    ctx.fillStyle = COLOR_LINE[0];
    ctx.fillText("sEMG (mV)", 0, 0);
    ctx.restore();

    ctx.save();
    ctx.translate(canvas.width - 5, paddingTop + plotHeight / 2);
    ctx.rotate(Math.PI / 2);
    ctx.fillStyle = COLOR_LINE[1];
    ctx.fillText("Flexión (ADC)", 0, 0);
    ctx.restore();

    function drawSeries(seriesData, minVal, maxVal, color) {
        if(seriesData.length < 2) return;
        ctx.strokeStyle = color;
        ctx.lineWidth = 2.5;
        ctx.beginPath();

        let len = seriesData.length;
        for(let i = 0; i < len; i++) {
            let val = seriesData[i];
            let x = paddingLeft + (i / (MAX_SAMPLE - 1)) * plotWidth;
            let y = paddingTop + plotHeight - ((val - minVal) / (maxVal - minVal)) * plotHeight;
            
            if(i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        }
        ctx.stroke();
    }

    drawSeries(data[0], minEMG, maxEMG, COLOR_LINE[0]); 
    drawSeries(data[1], minFlex, maxFlex, COLOR_LINE[1]); 
}

window.onload = init;
</script>
</body>
</html>
)=====";