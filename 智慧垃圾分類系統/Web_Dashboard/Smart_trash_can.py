import os
import json
import time
import boto3
import numpy as np
from flask import Flask, request
from linebot import LineBotApi, WebhookHandler
from linebot.models import MessageEvent, ImageMessage, TextSendMessage,TextMessage
from boto3.dynamodb.conditions import Attr
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import tensorflow.keras as keras
from PIL import Image, ImageOps

app = Flask(__name__)

LINE_ACCESS_TOKEN = "aKKwbsqCVmUKPR2XY+pyvkju45cf/JSiUk28Kh2V/6U7QOrCdYICncf9P/Kp6dWO22IGdcD2mDrkMYmMQjgXCydr90YJlXZSC1SGMtByDOWnySrJnN2KHV/8YnZF6bCOqjLRUe+WodM93sOjOMXfsAdB04t89/1O/w1cDnyilFU="
LINE_CHANNEL_SECRET = "272c2cc628542a1ca20774e2c2c9905b"
AWS_ENDPOINT = "請自行改成AWS_ENDPOINT(...us-east-1.amazonaws.com)"
CA_PATH = "AmazonRootCA1.pem"
CERT_PATH = "ca41b6dfff4edcf8d9b5072c8633f47a93b2bb57e612589d3743f430b77515cc-certificate.pem.crt"
KEY_PATH = "ca41b6dfff4edcf8d9b5072c8633f47a93b2bb57e612589d3743f430b77515cc-private.pem.key"

ACCESS_KEY = "請自行改成IAM或是自己本身的ACCESS_KEY"
SECRET_KEY = "請自行改成IAM或是自己本身的SECRET_KEY"
SESSION_TOKEN = "請自行改成IAM或是自己本身的SESSION_TOKEN"
myMQTTClient = AWSIoTMQTTClient("PythonControl")
myMQTTClient.configureEndpoint(AWS_ENDPOINT, 8883)
myMQTTClient.configureCredentials(CA_PATH, KEY_PATH, CERT_PATH)
myMQTTClient.connect()
session = boto3.Session(
    aws_access_key_id=ACCESS_KEY,
    aws_secret_access_key=SECRET_KEY,
    aws_session_token=SESSION_TOKEN,
    region_name='us-east-1'
)
dynamodb = session.resource('dynamodb')
table = dynamodb.Table('TrashBinLog')

model = keras.models.load_model('keras_model.h5')

def get_prediction(img_path):
    data = np.ndarray(shape=(1, 224, 224, 3), dtype=np.float32)
    image = Image.open(img_path).convert('RGB')
    image = ImageOps.fit(image, (224, 224), Image.Resampling.LANCZOS)
    normalized_image_array = (np.asarray(image).astype(np.float32) / 127.0) - 1
    data[0] = normalized_image_array
    pred = model.predict(data)
    return "trash" if pred[0][0] > pred[0][1] else "recycle"
def get_stats():
    try:
        recycle_res = table.scan(FilterExpression=Attr('target').eq('recycle'))
        trash_res = table.scan(FilterExpression=Attr('target').eq('trash'))
        
        r_count = recycle_res['Count']
        t_count = trash_res['Count']
        
        return f"目前智慧垃圾桶開蓋統計：\n♻️ 回收開蓋次數：{r_count} 次\n🗑️ 垃圾開蓋次數：{t_count} 次\n🌟 總計開蓋：{r_count + t_count} 次"
    except Exception as e:
        print(f"統計讀取失敗: {e}")
        return "暫時無法讀取統計數據。"
    
def reset_stats():
    try:
        response = table.scan(ProjectionExpression="#ts", ExpressionAttributeNames={"#ts": "timestamp"})
        items = response.get('Items', [])

        if not items:
            return "目前資料庫本來就是空的，無需歸零。"

        with table.batch_writer() as batch:
            for item in items:
                batch.delete_item(Key={'timestamp': item['timestamp']})
        
        return "✅ 所有的開蓋紀錄已成功歸零！"
    except Exception as e:
        print(f"歸零失敗: {e}")
        return f"歸零失敗，錯誤訊息: {str(e)}"
line_bot_api = LineBotApi(LINE_ACCESS_TOKEN)
handler = WebhookHandler(LINE_CHANNEL_SECRET)

@app.route("/callback", methods=['POST'])
def callback():
    body = request.get_data(as_text=True)
    handler.handle(body, request.headers['X-Line-Signature'])
    return 'OK'

@handler.add(MessageEvent, message=TextMessage)
def handle_text(event):
    user_msg = event.message.text
    
    if user_msg == "開蓋統計":
        stats = get_stats()
        line_bot_api.reply_message(event.reply_token, TextSendMessage(text=stats))
    
    elif user_msg == "歸零統計":
        result_msg = reset_stats()
        line_bot_api.reply_message(event.reply_token, TextSendMessage(text=result_msg))

@handler.add(MessageEvent, message=ImageMessage)
def handle_image(event):
    path = f"{event.message.id}.jpg"
    try:
        message_content = line_bot_api.get_message_content(event.message.id)
        with open(path, 'wb') as f:
            for chunk in message_content.iter_content(): f.write(chunk)
        
        result = get_prediction(path)
        current_time = int(time.time())
        
        table.put_item(
            Item={
                'timestamp': current_time,
                'target': result,
                'user': event.source.user_id,
                'count_increment': 1
            }
        )
        print(f"成功記錄到雲端: {result}")

        count_res = table.scan(FilterExpression=Attr('target').eq(result))
        current_count = count_res['Count']

        reply_msg = f"辨識結果：{result}。垃圾桶已開啟！"
        
        if current_count >= 10:
            reply_msg += f"\n\n⚠️ 滿載警告：【{result}】桶已開蓋 {current_count} 次，請通知人員清理！"

        payload = {"timestamp": current_time, "target": result, "action": "open_lid"}   
        myMQTTClient.publish("bin/control", json.dumps(payload), 1)

        line_bot_api.reply_message(
            event.reply_token, 
            TextSendMessage(text=reply_msg)
        )

    except Exception as e:
        print(f"處理失敗: {e}")
    finally:
        if os.path.exists(path): os.remove(path)

if __name__ == "__main__":
    app.run(port=5000)