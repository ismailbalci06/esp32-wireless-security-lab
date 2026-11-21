#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ============================================================
// ESP32 WIRELESS SECURITY LAB 6.0
// Passive Wi-Fi + BLE observation dashboard
// History is stored by the browser (MacBook Chrome), not in NVS.
// ============================================================

// Change this password before using the AP outside your own lab.
const char* AP_SSID = "ESP32-Wireless-Lab";
const char* AP_PASSWORD = "ChangeThis_AP_Password_2026!";

const int MAX_WIFI = 40;
const int MAX_BLE = 30;

WebServer server(80);
BLEScan* bleScanner = nullptr;

struct WiFiRecord {
  String ssid;
  int rssi;
  int channel;
  String security;
};

struct BLERecord {
  String id;       // Hashed BLE address; raw address is never sent to browser
  String name;
  int rssi;
};

WiFiRecord wifiRecords[MAX_WIFI];
BLERecord bleRecords[MAX_BLE];
int wifiCount = 0;
int bleCount = 0;
int channelCounts[14] = {0};

// ---------------- JSON HELPERS ----------------

String escapeJSON(const String& input) {
  String out;
  out.reserve(input.length() + 8);

  for (size_t i = 0; i < input.length(); i++) {
    uint8_t c = (uint8_t)input[i];
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else if (c < 0x20) {
      char buf[7];
      snprintf(buf, sizeof(buf), "\\u%04x", c);
      out += buf;
    } else {
      out += (char)c;
    }
  }
  return out;
}

const char* signalLevel(int rssi) {
  if (rssi >= -60) return "Excellent";
  if (rssi >= -70) return "Good";
  if (rssi >= -80) return "Medium";
  return "Weak";
}

// Create a short pseudonymous ID from the BLE address.
// This lets the dashboard compare observations without exposing raw MACs.
String makeBleId(const String& address) {
  uint32_t hash = 2166136261UL;  // FNV-1a 32-bit offset basis
  for (size_t i = 0; i < address.length(); i++) {
    hash ^= (uint8_t)address[i];
    hash *= 16777619UL;
  }
  char idBuffer[16];
  snprintf(idBuffer, sizeof(idBuffer), "BLE-%08lX", (unsigned long)hash);
  return String(idBuffer);
}

// ---------------- WI-FI SCANNER ----------------

void scanWiFi() {
  wifiCount = 0;
  for (int i = 0; i < 14; i++) channelCounts[i] = 0;

  Serial.println("[WIFI] Passive network scan started...");
  int found = WiFi.scanNetworks(false, false, true, 120);

  if (found < 0) {
    Serial.println("[WIFI] Scan failed.");
    WiFi.scanDelete();
    return;
  }

  Serial.printf("[WIFI] %d network(s) observed.\n", found);

  for (int i = 0; i < found; i++) {
    int channel = WiFi.channel(i);
    if (channel >= 1 && channel <= 13) channelCounts[channel]++;

    if (wifiCount >= MAX_WIFI) continue;

    wifiRecords[wifiCount].ssid = WiFi.SSID(i);
    if (wifiRecords[wifiCount].ssid.length() == 0) {
      wifiRecords[wifiCount].ssid = "<Hidden SSID>";
    }
    wifiRecords[wifiCount].rssi = WiFi.RSSI(i);
    wifiRecords[wifiCount].channel = channel;
    wifiRecords[wifiCount].security =
      (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured";
    wifiCount++;
  }

  // Strongest RSSI first (e.g. -50 dBm before -85 dBm).
  for (int i = 0; i < wifiCount - 1; i++) {
    for (int j = 0; j < wifiCount - i - 1; j++) {
      if (wifiRecords[j].rssi < wifiRecords[j + 1].rssi) {
        WiFiRecord temp = wifiRecords[j];
        wifiRecords[j] = wifiRecords[j + 1];
        wifiRecords[j + 1] = temp;
      }
    }
  }

  WiFi.scanDelete();
}

// ---------------- BLE PASSIVE SCANNER ----------------

void scanBLE() {
  bleCount = 0;
  Serial.println("[BLE] Passive advertisement scan started...");

  if (bleScanner == nullptr) {
    Serial.println("[BLE] Scanner is not initialized.");
    return;
  }

  BLEScanResults* results = bleScanner->start(4, false);
  if (results == nullptr) {
    Serial.println("[BLE] Scan failed.");
    return;
  }

  int found = results->getCount();
  Serial.printf("[BLE] %d advertisement(s) observed.\n", found);

  for (int i = 0; i < found && bleCount < MAX_BLE; i++) {
    BLEAdvertisedDevice device = results->getDevice(i);

    String rawAddress = device.getAddress().toString();
    bleRecords[bleCount].id = makeBleId(rawAddress);
    bleRecords[bleCount].name = device.haveName()
      ? String(device.getName().c_str())
      : "Isimsiz BLE cihaz";
    bleRecords[bleCount].rssi = device.getRSSI();
    bleCount++;
  }

  // Strongest RSSI first.
  for (int i = 0; i < bleCount - 1; i++) {
    for (int j = 0; j < bleCount - i - 1; j++) {
      if (bleRecords[j].rssi < bleRecords[j + 1].rssi) {
        BLERecord temp = bleRecords[j];
        bleRecords[j] = bleRecords[j + 1];
        bleRecords[j + 1] = temp;
      }
    }
  }

  bleScanner->clearResults();
}

// ---------------- ONE COMBINED SCAN ----------------

String performScan() {
  Serial.println("\n====================================");
  Serial.println(" ESP32 WIRELESS SECURITY LAB 6.0");
  Serial.println("====================================");

  // Run BLE and Wi-Fi scans sequentially to reduce radio contention.
  scanBLE();
  scanWiFi();

  String json;
  json.reserve(14000);
  json = "{\"wifiCount\":" + String(wifiCount);
  json += ",\"bleCount\":" + String(bleCount);

  json += ",\"wifi\":[";
  for (int i = 0; i < wifiCount; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + escapeJSON(wifiRecords[i].ssid) + "\"";
    json += ",\"rssi\":" + String(wifiRecords[i].rssi);
    json += ",\"channel\":" + String(wifiRecords[i].channel);
    json += ",\"level\":\"" + String(signalLevel(wifiRecords[i].rssi)) + "\"";
    json += ",\"security\":\"" + wifiRecords[i].security + "\"}";
  }

  json += "],\"ble\":[";
  for (int i = 0; i < bleCount; i++) {
    if (i > 0) json += ",";
    json += "{\"id\":\"" + bleRecords[i].id + "\"";
    json += ",\"name\":\"" + escapeJSON(bleRecords[i].name) + "\"";
    json += ",\"rssi\":" + String(bleRecords[i].rssi) + "}";
  }

  json += "],\"channels\":[";
  for (int ch = 1; ch <= 13; ch++) {
    if (ch > 1) json += ",";
    json += "{\"channel\":" + String(ch);
    json += ",\"count\":" + String(channelCounts[ch]) + "}";
  }
  json += "]}";

  Serial.println("[SYSTEM] Combined scan completed.");
  return json;
}

// ---------------- WEB DASHBOARD ----------------

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Wireless Security Lab 6.0</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:22px;background:#0b1220;color:#e5e7eb;font-family:Arial,sans-serif}
main{max-width:980px;margin:auto}
h1{font-size:25px;margin-bottom:5px}
h3{margin-top:0}
.muted{color:#94a3b8;font-size:13px;line-height:1.5}
.card{background:#172033;border:1px solid #29374d;border-radius:13px;padding:18px;margin:14px 0}
.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:12px}
.stat span{color:#94a3b8;font-size:12px}.stat strong{display:block;font-size:28px;margin-top:10px;overflow-wrap:anywhere}
button{padding:11px 17px;border:0;border-radius:8px;background:#2563eb;color:white;cursor:pointer;font-weight:bold;margin:2px}
button.secondary{background:#334155}button.danger{background:#7f1d1d}button:disabled{opacity:.5;cursor:wait}
.tabs{display:flex;gap:8px;flex-wrap:wrap;margin:18px 0}.tabs button.active{background:#0f766e}
.network{border-bottom:1px solid #334155;padding:14px 0}.network:last-child{border:0}
.head{display:flex;justify-content:space-between;gap:10px;flex-wrap:wrap}.name{font-weight:bold;overflow-wrap:anywhere}
.meta{font-size:12px;color:#94a3b8;margin-top:7px}
.track{background:#334155;height:8px;border-radius:20px;overflow:hidden;margin-top:10px}
.fill{background:#38bdf8;height:100%;border-radius:20px}
.channel{display:grid;grid-template-columns:80px 1fr 25px;align-items:center;gap:9px;margin:13px 0;font-size:13px}
.status{color:#94a3b8;font-size:13px;margin-top:12px}
.event{padding:9px 0;border-bottom:1px solid #334155;font-size:13px}.event:last-child{border:0}
.tag{display:inline-block;border-radius:6px;padding:3px 7px;font-size:11px;margin-right:6px;background:#334155}
canvas{display:block;width:100%;height:210px}
[hidden]{display:none!important}
@media(max-width:600px){body{padding:12px}.stats{grid-template-columns:1fr}.stat strong{font-size:24px}}
</style>
</head>
<body>
<main>
<h1>ESP32 Wireless Security Lab 6.0</h1>
<p class="muted">Wi-Fi & Bluetooth Low Energy passive observation</p>

<div class="card">
  <button id="scanBtn" onclick="scanAll()">Tumunu Tara</button>
  <button class="secondary" onclick="exportCSV()">Tum Gecmisi CSV Indir</button>
  <button class="danger" onclick="clearHistory()">Gecmisi Temizle</button>
  <div class="status" id="status">Sistem hazir.</div>
</div>

<div class="stats">
  <div class="card stat"><span>Wi-Fi aglari</span><strong id="wifiCount">--</strong></div>
  <div class="card stat"><span>BLE yayinlari</span><strong id="bleCount">--</strong></div>
  <div class="card stat"><span>En guclu Wi-Fi</span><strong id="strongest">--</strong></div>
</div>

<div class="tabs">
  <button id="wifiTab" class="active" onclick="showTab('wifi')">Wi-Fi</button>
  <button id="bleTab" class="secondary" onclick="showTab('ble')">Bluetooth BLE</button>
  <button id="channelTab" class="secondary" onclick="showTab('channels')">Kanallar</button>
  <button id="historyTab" class="secondary" onclick="showTab('history')">Gecmis</button>
</div>

<div class="card" id="wifiPanel"><h3>Wi-Fi Ag Listesi</h3><div id="wifiList">Tarama bekleniyor...</div></div>
<div class="card" id="blePanel" hidden><h3>BLE Cihaz Listesi</h3><div id="bleList">Tarama bekleniyor...</div><p class="muted">Yalnizca BLE reklam paketleri pasif olarak gozlemlenir. Ham BLE adresleri panelde gosterilmez.</p></div>
<div class="card" id="channelPanel" hidden><h3>Wi-Fi Kanal Dagilimi</h3><div id="channelList"></div><p class="muted">Sayilar gorulen erisim noktalarini ifade eder; radyo parazit olcumu degildir.</p></div>

<div class="card" id="historyPanel" hidden>
  <h3>En Guclu Wi-Fi Sinyalinin Gecmisi</h3>
  <p class="muted">Her taramadaki en guclu Wi-Fi RSSI degeri. Zaman, MacBook tarayicisindan alinmistir.</p>
  <canvas id="signalChart" height="210"></canvas>
  <div class="muted" id="historySummary">Henuz gecmis kaydi yok.</div>
  <h3 style="margin-top:22px">BLE Degisimleri (Son iki tarama)</h3>
  <div id="bleChanges" class="muted">Karsilastirma icin en az iki tarama gerekir.</div>
</div>

<div class="card">
  <h3>Son Tarama Bilgisi</h3>
  <div class="muted" id="lastScan">Henuz tarama yapilmadi.</div>
  <div class="muted" id="historyCount">Kayitli tarama: 0</div>
</div>
<p class="muted">ESP32 Wireless Security Lab 6.0 | Son 40 tarama Chrome tarayicisinda saklanir.</p>
</main>

<script>
const HISTORY_KEY = "esp32WirelessLabHistoryV6";
const MAX_HISTORY = 40;
let latest = null;
let historyData = loadHistory();
let busy = false;

function loadHistory(){
  try{
    const value = JSON.parse(localStorage.getItem(HISTORY_KEY) || "[]");
    return Array.isArray(value) ? value.slice(-MAX_HISTORY) : [];
  }catch(e){ return []; }
}

function saveHistory(){
  try{
    localStorage.setItem(HISTORY_KEY, JSON.stringify(historyData.slice(-MAX_HISTORY)));
    return true;
  }catch(e){
    document.getElementById("status").textContent = "Tarama tamamlandi; tarayici depolamasi yazilamadi. Chrome site verisi ayarlarini kontrol et.";
    return false;
  }
}

function showTab(tab){
  const panels={wifi:"wifiPanel",ble:"blePanel",channels:"channelPanel",history:"historyPanel"};
  Object.keys(panels).forEach(key=>{
    document.getElementById(panels[key]).hidden = key !== tab;
    const id = key === "channels" ? "channelTab" : key + "Tab";
    const btn = document.getElementById(id);
    btn.className = key === tab ? "active" : "secondary";
  });
  if(tab === "history") drawSignalChart();
}

function addSignalBar(parent,rssi){
  const track=document.createElement("div");track.className="track";
  const fill=document.createElement("div");fill.className="fill";
  fill.style.width=Math.max(0,Math.min(100,(rssi+100)*100/70))+"%";
  track.appendChild(fill);parent.appendChild(track);
}

function renderWiFi(data){
  const root=document.getElementById("wifiList");root.replaceChildren();
  data.forEach((n,i)=>{
    const item=document.createElement("div");item.className="network";
    const head=document.createElement("div");head.className="head";
    const name=document.createElement("span");name.className="name";name.textContent=(i+1)+". "+n.ssid;
    const signal=document.createElement("b");signal.textContent=n.rssi+" dBm";
    head.append(name,signal);
    const meta=document.createElement("div");meta.className="meta";meta.textContent="Kanal: "+n.channel+" | "+n.level+" | "+n.security;
    item.append(head,meta);addSignalBar(item,n.rssi);root.appendChild(item);
  });
  if(!data.length)root.textContent="Wi-Fi agi bulunamadi.";
}

function renderBLE(data){
  const root=document.getElementById("bleList");root.replaceChildren();
  data.forEach((device,i)=>{
    const item=document.createElement("div");item.className="network";
    const head=document.createElement("div");head.className="head";
    const name=document.createElement("span");name.className="name";name.textContent=(i+1)+". "+device.name;
    const signal=document.createElement("b");signal.textContent=device.rssi+" dBm";
    head.append(name,signal);
    const meta=document.createElement("div");meta.className="meta";meta.textContent="Yerel gozlem kimligi: "+device.id;
    item.append(head,meta);addSignalBar(item,device.rssi);root.appendChild(item);
  });
  if(!data.length)root.textContent="BLE reklam yayinlayan cihaz bulunamadi.";
}

function renderChannels(data){
  const root=document.getElementById("channelList");root.replaceChildren();
  const max=Math.max(1,...data.map(c=>c.count));
  data.filter(c=>c.count>0).forEach(c=>{
    const row=document.createElement("div");row.className="channel";
    const label=document.createElement("span");label.textContent="Kanal "+c.channel;
    const track=document.createElement("div");track.className="track";
    const fill=document.createElement("div");fill.className="fill";fill.style.width=(c.count/max*100)+"%";
    track.appendChild(fill);
    const count=document.createElement("span");count.textContent=c.count;
    row.append(label,track,count);root.appendChild(row);
  });
  if(!data.some(c=>c.count>0))root.textContent="Kanal verisi yok.";
}

function renderCurrent(data){
  latest=data;
  document.getElementById("wifiCount").textContent=data.wifiCount;
  document.getElementById("bleCount").textContent=data.bleCount;
  document.getElementById("strongest").textContent=data.wifi.length?data.wifi[0].rssi+" dBm":"--";
  renderWiFi(data.wifi);renderBLE(data.ble);renderChannels(data.channels);
  document.getElementById("lastScan").textContent="Son tarama: "+new Date(data.timestamp).toLocaleString("tr-TR");
  document.getElementById("historyCount").textContent="Kayitli tarama: "+historyData.length+" / "+MAX_HISTORY;
  renderBLEChanges();drawSignalChart();
}

function renderBLEChanges(){
  const root=document.getElementById("bleChanges");root.replaceChildren();
  if(historyData.length<2){root.textContent="Ilk tarama kaydedildi. Degisimleri gormek icin bir sonraki tarama gerekli.";return;}
  const previous=historyData[historyData.length-2];
  const current=historyData[historyData.length-1];
  const prevMap=new Map((previous.ble||[]).map(d=>[d.id,d]));
  const currMap=new Map((current.ble||[]).map(d=>[d.id,d]));
  const added=[...currMap.values()].filter(d=>!prevMap.has(d.id));
  const notSeen=[...prevMap.values()].filter(d=>!currMap.has(d.id));
  const addLine=(label,device,kind)=>{
    const row=document.createElement("div");row.className="event";
    const tag=document.createElement("span");tag.className="tag";tag.textContent=label;
    const value=document.createElement("span");value.textContent=device.name+" ("+device.id+")";
    row.append(tag,value);root.appendChild(row);
  };
  added.forEach(d=>addLine("Yeni goruldu",d,"new"));
  notSeen.forEach(d=>addLine("Son taramada yok",d,"missing"));
  if(!added.length&&!notSeen.length)root.textContent="Son iki taramada BLE gozlemlerinde degisiklik yok.";
  const note=document.createElement("p");note.className="muted";
  note.textContent="Not: BLE cihazlari gizlilik nedeniyle adreslerini degistirebilir; bu durumda ayni fiziksel cihaz yeni kimlikle gorunebilir. 'Son taramada yok' cihazin kapandigini kanitlamaz.";
  root.appendChild(note);
}

function drawSignalChart(){
  const canvas=document.getElementById("signalChart");
  if(!canvas || canvas.offsetParent===null)return;
  const width=Math.max(300,canvas.clientWidth);const height=210;
  canvas.width=width;canvas.height=height;
  const ctx=canvas.getContext("2d");ctx.clearRect(0,0,width,height);
  const left=42,right=width-12,top=15,bottom=height-30;
  const values=historyData.map(s=>({time:s.timestamp,rssi:(s.wifi&&s.wifi.length)?Number(s.wifi[0].rssi):null}));
  ctx.font="11px Arial";ctx.fillStyle="#94a3b8";ctx.strokeStyle="#334155";ctx.lineWidth=1;
  [-40,-60,-80,-100].forEach(v=>{
    const y=top+((-30-v)/70)*(bottom-top);
    ctx.beginPath();ctx.moveTo(left,y);ctx.lineTo(right,y);ctx.stroke();
    ctx.fillText(String(v),3,y+4);
  });
  if(!values.length){ctx.fillText("Henuz sinyal gecmisi yok.",left,top+25);return;}
  const xAt=i=>values.length===1?(left+right)/2:left+i*(right-left)/(values.length-1);
  const yAt=v=>top+((-30-v)/70)*(bottom-top);
  ctx.strokeStyle="#38bdf8";ctx.lineWidth=2;ctx.beginPath();let started=false;
  values.forEach((p,i)=>{
    if(p.rssi===null)return;
    const x=xAt(i),y=yAt(Math.max(-100,Math.min(-30,p.rssi)));
    if(!started){ctx.moveTo(x,y);started=true;}else ctx.lineTo(x,y);
  });
  if(started)ctx.stroke();
  values.forEach((p,i)=>{
    if(p.rssi===null)return;
    const x=xAt(i),y=yAt(Math.max(-100,Math.min(-30,p.rssi)));
    ctx.fillStyle="#38bdf8";ctx.beginPath();ctx.arc(x,y,3,0,Math.PI*2);ctx.fill();
  });
  ctx.fillStyle="#94a3b8";
  if(values.length){ctx.textAlign="left";ctx.fillText(new Date(values[0].time).toLocaleTimeString("tr-TR"),left,height-8);
    ctx.textAlign="right";ctx.fillText(new Date(values[values.length-1].time).toLocaleTimeString("tr-TR"),right,height-8);ctx.textAlign="left";}
  document.getElementById("historySummary").textContent="Grafikte "+values.length+" tarama var. En son guclu Wi-Fi RSSI: "+(values[values.length-1].rssi===null?"ag yok":values[values.length-1].rssi+" dBm");
}

async function scanAll(){
  if(busy)return;busy=true;
  const btn=document.getElementById("scanBtn");const status=document.getElementById("status");
  btn.disabled=true;status.textContent="BLE ve Wi-Fi taramalari yapiliyor...";
  try{
    const response=await fetch("/api/scan?t="+Date.now(),{cache:"no-store"});
    if(!response.ok)throw new Error("HTTP "+response.status);
    const data=await response.json();if(data.error)throw new Error(data.error);
    data.timestamp=new Date().toISOString();
    historyData.push(data);if(historyData.length>MAX_HISTORY)historyData=historyData.slice(-MAX_HISTORY);
    saveHistory();renderCurrent(data);
    status.textContent="Wi-Fi ve BLE taramalari tamamlandi. Gecmis MacBook tarayicisina kaydedildi.";
  }catch(error){status.textContent="Hata: "+error.message;}
  finally{busy=false;btn.disabled=false;}
}

function csvCell(value){return '"'+String(value??"").replace(/"/g,'""')+'"';}
function exportCSV(){
  if(!historyData.length){alert("Once en az bir tarama yapmalisin.");return;}
  const rows=[["timestamp_local","type","observation_id","name","rssi_dbm","wifi_channel","security"]];
  historyData.forEach(snapshot=>{
    const time=new Date(snapshot.timestamp).toLocaleString("tr-TR");
    (snapshot.wifi||[]).forEach(n=>rows.push([time,"Wi-Fi","",n.ssid,n.rssi,n.channel,n.security]));
    (snapshot.ble||[]).forEach(n=>rows.push([time,"BLE",n.id,n.name,n.rssi,"",""]));
  });
  const csv="\uFEFF"+rows.map(row=>row.map(csvCell).join(",")).join("\r\n");
  const blob=new Blob([csv],{type:"text/csv;charset=utf-8"});
  const url=URL.createObjectURL(blob);const link=document.createElement("a");
  link.href=url;link.download="wireless-security-lab-6-history.csv";link.click();
  setTimeout(()=>URL.revokeObjectURL(url),1000);
}

function clearHistory(){
  if(!confirm("Chrome'da bu panel icin saklanan son 40 tarama silinsin mi?"))return;
  historyData=[];latest=null;
  try{localStorage.removeItem(HISTORY_KEY);}catch(e){}
  document.getElementById("historyCount").textContent="Kayitli tarama: 0 / "+MAX_HISTORY;
  document.getElementById("historySummary").textContent="Gecmis temizlendi.";
  document.getElementById("bleChanges").textContent="Karsilastirma icin en az iki tarama gerekir.";
  document.getElementById("lastScan").textContent="Henuz tarama yapilmadi.";
  drawSignalChart();
}

// Restore the previous browser history after a refresh.
if(historyData.length){renderCurrent(historyData[historyData.length-1]);
  document.getElementById("status").textContent="Onceki tarama gecmisi Chrome'dan yuklendi.";
  document.getElementById("historyCount").textContent="Kayitli tarama: "+historyData.length+" / "+MAX_HISTORY;
}
window.addEventListener("resize",drawSignalChart);
scanAll();
setInterval(scanAll,30000);
</script>
</body>
</html>
)rawliteral";

// ---------------- HTTP HANDLERS ----------------

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleScan() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.send(200, "application/json; charset=utf-8", performScan());
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nStarting Wireless Security Lab 6.0");
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  bool started = WiFi.softAP(AP_SSID, AP_PASSWORD, 1, false, 4);
  if (started) {
    Serial.println("Access Point started!");
    Serial.print("Network: "); Serial.println(AP_SSID);
    Serial.print("Dashboard: http://"); Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Access Point failed!");
  }

  BLEDevice::init("");
  bleScanner = BLEDevice::getScan();
  bleScanner->setActiveScan(false);  // Passive BLE scan; no scan requests sent.
  bleScanner->setInterval(100);
  bleScanner->setWindow(99);
  Serial.println("BLE passive scanner initialized.");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Web server started!");
  Serial.println("Wireless Security Lab 6.0 READY!");
}

void loop() {
  server.handleClient();
  delay(2);
}
