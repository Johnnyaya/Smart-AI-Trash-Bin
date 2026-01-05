#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h> 

//設定
const char* ssid = "cics";
const char* password = "12345678";
const char* aws_endpoint = "a10leg692w5t0y-ats.iot.us-east-1.amazonaws.com";
Servo servoTrash;   // D1
Servo servoRecycle; // D2
const int pinTrash = 5;
const int pinRecycle = 4;

//證書
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUWbjUZYrGGo8M7JRfxVkxFffLeOQwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTIzMDAyMTU0
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALa5bdje8kOG6InCJI4O
yWUHxEvWF2Ev2tdca790thzzb+/tcrvE6rYioMgfL8zbLmuvb7XTF7NNe83uY/1H
iVq7vaWoQMubvcflrko1TojX3YvE6OnijDubYXNMqxgQ761wYBW5l3nVoYYgCjlX
ELDAxO2cwfQf57dSBjHDBNhFETSbn8PPKnK43DbBa9C8BBq8EgXZifgtFuo8i6e9
G/hqz/OKNMXaMdNPQ/wVn4cOknQMS3WDGcGMHYrZJOkItk7aFuDqSSPDx+1EOPQr
8qBB+wLfLps0eO4RwLda3trtEDctrWm+CV5e5KnZhhqNksF4Hh4RsYjGfZWCT5b5
wlECAwEAAaNgMF4wHwYDVR0jBBgwFoAUJTlVNDWxLpk+Salr66QzKLeyVgcwHQYD
VR0OBBYEFFd0nZJHPbjjlhGDlkrWyHbEDgHEMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQANtIjV+YxkHiPjT+jCAABH87LU
Y2cez4HMQIt9F95QWUalCW84yTiuKabrn7d6GrNZ/9bpUSx3lw65ufdSw7nV8leD
Q6DIECWrQN/YOh1yRcTg9mpup18M9ud9IPD895jzLknwTZnM/sy/G2eNep+xTPQm
iTvjviNA1xx17q+FFjgFXmjZNxiPKjVM9ZcVqMtwvxS/+N/K+84Gu06DhuLMT5Tu
k1JC8G08RrO5Lfg90aCluW6my/9nX2NKPNuK8hLAiZKbPmcloleXs4D3BsjA+2pi
IvCI1ayecjw+qjBiJVyDflklqIcS/j6TtBwWeQqbLPjpJBRY98GCZplC/9kv
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAtrlt2N7yQ4boicIkjg7JZQfES9YXYS/a11xrv3S2HPNv7+1y
u8TqtiKgyB8vzNsua69vtdMXs017ze5j/UeJWru9pahAy5u9x+WuSjVOiNfdi8To
6eKMO5thc0yrGBDvrXBgFbmXedWhhiAKOVcQsMDE7ZzB9B/nt1IGMcME2EURNJuf
w88qcrjcNsFr0LwEGrwSBdmJ+C0W6jyLp70b+GrP84o0xdox009D/BWfhw6SdAxL
dYMZwYwditkk6Qi2TtoW4OpJI8PH7UQ49CvyoEH7At8umzR47hHAt1re2u0QNy2t
ab4JXl7kqdmGGo2SwXgeHhGxiMZ9lYJPlvnCUQIDAQABAoIBABisvaq1oJ2B0FEa
CHmugX/FJrre9FVJcxH2Bw7ZoeYS8aDCveWhV9i0oGWl4HyXdJNSI5G4KdEqu3c5
pAMF8ARyqc9/AxD7vQsLdxfKTwqnq+E9+/3pe+potyYsykX7s4IGVNNaNpxbPu6v
0PGhUj2sKmuz6XUgAdh9mRyLK1JQ9rw/pRJ9evYrfcYeUmsX4T0VRq9eY2Wf6Noe
1ECQNHCQUXh2s+yMEdiAX52zPsS/Ye5sxeUY0ETY4P3Nn6ElphzMk8Dnxx9DQ83p
/v7BGYT1x4rfbl21ejGLcmrIP1Im8cXwHTayfuu1VjNXjdSvRcwfAYFoAkKYFzR0
42sILqkCgYEA62uGRQD8yU24CpvjgB6Jg3P7cInhpcp4ko++p1v7CE4LTdCih3BF
FDHs+PYCai6WITKFFhezipWobLk3x6Zvzdx+sW8OreM1YgLVs4L06J8qhsu6qRuc
YTutjM1Xm8AFaI7Tgl0/wh2sRbv7F9fFl5MeeEmAIRwrlOCxBFsVlqsCgYEAxrKg
Gx5jJNBZ2zhITAnWCy6E0UCUej/GfttIMX1TM4U1nyfu5cNFrAxZF00JL60a7qEU
xAk1g8jA+8baE6YRUlzfoeqNj9Mz4o4am6LZp//W0Ug9MF0PckPnAoA88E5FUj2s
oiUW2Rgxb/2crPd7fETU8YxS+gvsI/1rpkwLOvMCgYBhQSBM7WWAu+yl5VSMnxDs
D0P2b9+aIBTDZ0K53NPN0H/2+5PJI+ZFu4JML0U8HFbFDBvSzqPLQpw7niCfB/D0
OHzWTCziRRTOkqE58YW/e9GTsiJArjliKcc+fa4Nl7wkP6y9FEy8oomCnW/Tn73v
KkXSJ3sZMowGivRV+lLm1QKBgQCRG2ArjdyPxQzqOMcrqL9FOTrL6mLEiJC4kgG+
Cq1ZYb4QNkSUTTAzfm0NxDeiNsQFY2Pb6nHHhTPSa51VXiEmSicTg0zQrUgl+utZ
n+teckt6WUw+ZqfiJc6uF+LpB/A+KEZLv9RYxP9NryPdeXsX/NiacoUikNRqUCdu
sfg9mwKBgQDkrMuJpKzgypwm3YqqnzAzw8gUSG94GgWEaqiWYj1LPd758+QgpFek
Y6AdWx8CqeSeGXkirwMgP3AWl5WdkEIO/bxVswJ6Bctc75K1RZyFfJBMhKUiqZfH
Vu3malXmiiKdqH9yOBJG+3wUtBxss4gS/GuPyeqxZCQ797e6pI/RyQ==
-----END RSA PRIVATE KEY-----
)EOF";

BearSSL::X509List ca_cert(AWS_CERT_CA);
BearSSL::X509List client_cert(AWS_CERT_CRT);
BearSSL::PrivateKey client_priv_key(AWS_CERT_PRIVATE);

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
 // 3.馬達接腳、WiFi
  servoTrash.attach(pinTrash);
  servoRecycle.attach(pinRecycle);
  servoTrash.write(0); 
  servoRecycle.write(0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi 已連線");

  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 10000) { delay(500); }

  espClient.setTrustAnchors(&ca_cert);
  espClient.setClientRSACert(&client_cert, &client_priv_key);

  client.setServer(aws_endpoint, 8883);
  client.setCallback(callback);
}

// 4.收到指令轉動馬達
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  const char* target = doc["target"]; // 這裡必須與 Python payload 一致

  if (strcmp(target, "trash") == 0) {
    Serial.println(">>> 開啟一般垃圾桶");
    servoTrash.write(180);
    delay(5000);           
    servoTrash.write(0); 
  } 
  else if (strcmp(target, "recycle") == 0) {
    Serial.println(">>> 開啟資源回收桶");
    servoRecycle.write(180);
    delay(5000);
    servoRecycle.write(0);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("SmartTrashBin")) { 
      client.subscribe("bin/control"); 
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
