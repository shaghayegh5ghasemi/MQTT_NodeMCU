import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("on_connect: " + mqtt.connack_string(rc))
    client.subscribe("userid", 2)


mqtt_host = dict({"hostname": "172.20.10.2", "port": 1883})
topic_name = "userid"

# called when the client loses its connection
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection: {}".format(rc))


# called when a message arrives
def on_message(client, userdata, msg):
    userid = str(msg.payload)
    if(userid==b'174ccf5f'):
        client.publish("userdata", "6015")
    else:
        client.publish("userdata", "9940")


def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: mid=" + str(mid) + " QoS=" + str(granted_qos))


def on_publish(client, userdata, mid):
    print("Published: mid=" + str(mid))


client = mqtt.Client("node2")
client.on_connect = on_connect
client.on_message = on_message
client.on_subscribe = on_subscribe
client.on_disconnect = on_disconnect

client.connect(mqtt_host["hostname"],mqtt_host["port"], 60)

client.loop_forever()
