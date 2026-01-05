AI 智慧垃圾分類回收系統 - AIoT 智慧整合實作

📖專案簡介 (Introduction) 本專案結合 AI 影像辨識 (TensorFlow/Keras) 與 IoT (ESP8266) 運算，旨在解決校園與家庭中垃圾分類精準度不足的問題。

透過Line Bot作為使用者互動介面，使用者只需拍下垃圾照片並上傳，系統便會自動進行影像辨識，並透過 AWS IoT Core 遠端驅動硬體端(ESP8266)的伺服馬達開啟對應的垃圾桶蓋，同時將各類開蓋紀錄儲存於AWS DynamoDB雲端資料庫中。

🚀核心功能 Line Bot影像辨識：使用者透過Line上傳照片，後端Python(Flask)呼叫AI模型進行即時分類。

IoT自動化控制：ESP8266訂閱AWS IoT訊息，接收辨識結果（Trash/Recycle）後，自動觸發伺服馬達進行對應的開蓋動作。

雲端數據紀錄：使用AWS DynamoDB紀錄每次開蓋的時間、類別與使用者資訊。

即時統計與歸零：透過Line指令（如：「開蓋統計」、「歸零統計」）查詢目前垃圾量與重置數據。

滿載預警機制：當單一類別開蓋次數累計達10次時，系統會自動在Line訊息中發送滿載警告，提醒清潔人員。

🛠️系統架構(System Architecture) 技術堆疊(Tech Stack) AI Model: TensorFlow / Keras (H5 model trained via Teachable Machine)

Backend: Python Flask/ ngrok

IoT Node: ESP8266 (NodeMCU)

Cloud Services: AWS IoT Core (MQTT), AWS DynamoDB

Interface: Line Messaging API

🔌硬體配置 (Hardware Setup) 接線說明 Servo A (一般垃圾): 訊號線接 ESP8266 D1 (GPIO 5)

Servo B (資源回收): 訊號線接 ESP8266 D2 (GPIO 4)

電源 (Power): 馬達紅線接 Vin (5V)，黑線接 GND(須注意馬達與ESP8266需共地)

💻安裝與執行 (Installation)

啟動後端Web伺服器
安裝必要套件
pip install flask line-bot-sdk boto3 tensorflow numpy pillow AWSIoTPythonSDK

執行Flask App
python Smart_trash_can.py

使用ngrok建立Webhook通道
ngrok http 5000 註：請將ngrok產生的https網址填入Line Developer Console的Webhook URL處。

AWS 雲端設定
DynamoDB: 建立名為TrashBinLog的Table，Primary Key設為timestamp (Number)。

IoT Core: 建立 Thin為SmartTrashBin，並下載證書。

ESP8266 設定
使用 Arduino IDE 開啟 Smart_trash_esp8266.ino。

修改ssid與password為你的網路環境。

將下載的AWS證書 (CA, Certificate, Private Key) 填入程式碼中的證書區塊。

編譯並上傳至開發板。

📊實際操作範例 傳送照片：得到「辨識結果：recycle。垃圾桶已開啟！」回覆，且實體馬達轉動 90 度。

查詢統計：輸入「開蓋統計」，回傳目前recycle與trash的累計次數。

預警通知：當次數達10次，回覆訊息會包含「⚠️滿載警告」標語。

👨‍💻作者 開發者：[陳力愷、鍾永平]
