import paho.mqtt.client as mqtt_client


def create(rhconfig):
    client = None
    mqttConfig = rhconfig.MQTT
    if 'BROKER' in mqttConfig:
        client_id = mqttConfig['CLIENT_ID'] if 'CLIENT_ID' in mqttConfig else None
        client = mqtt_client.Client(client_id=client_id)
        if 'USERNAME' in mqttConfig:
            client.username_pw_set(mqttConfig['USERNAME'], mqttConfig['PASSWORD'] if 'PASSWORD' in mqttConfig else None)
        if 'CLIENT_CERT' in mqttConfig and 'PRIVATE_KEY' in mqttConfig:
            client.tls_set(certfile=mqttConfig['CLIENT_CERT'], keyfile=mqttConfig['PRIVATE_KEY'])
        client.connect(mqttConfig['BROKER'], mqttConfig['PORT'] if 'PORT' in mqttConfig else 1883)
        client.loop_start()
    return client
