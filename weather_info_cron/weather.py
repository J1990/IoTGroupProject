import requests, json 
import time
from datetime import datetime
import csv
from awscrt import io, mqtt, auth, http
from awsiot import mqtt_connection_builder

# Callback when connection is accidentally lost.
def on_connection_interrupted(connection, error, **kwargs):
    print("Connection interrupted. error: {}".format(error))

# Callback when an interrupted connection is re-established.
def on_connection_resumed(connection, return_code, session_present, **kwargs):
    print("Connection resumed. return_code: {} session_present: {}".format(return_code, session_present))

    if return_code == mqtt.ConnectReturnCode.ACCEPTED and not session_present:
        print("Session did not persist. Resubscribing to existing topics...")
        resubscribe_future, _ = connection.resubscribe_existing_topics()

        # Cannot synchronously wait for resubscribe result because we're on the connection's event-loop thread,
        # evaluate result with a callback instead.
        resubscribe_future.add_done_callback(on_resubscribe_complete)

def on_resubscribe_complete(resubscribe_future):
    resubscribe_results = resubscribe_future.result()
    print("Resubscribe results: {}".format(resubscribe_results))

    for topic, qos in resubscribe_results['topics']:
        if qos is None:
            sys.exit("Server rejected resubscribe to topic: {}".format(topic))

if __name__ == '__main__':
    
    endPt = "a37g54y6ddcht7-ats.iot.us-east-1.amazonaws.com"
    clientId = "samples-client-id"
    
    api_key = "2f30e9abf594e715af4449414b055800"
    base_url = "http://api.openweathermap.org/data/2.5/weather?"
    city_name = "Dublin,IE"
    weather_api_url = base_url + "appid=" + api_key + "&q=" + city_name + "&units=metric"
    response = requests.get(weather_api_url) 

    x = response.json() 
    
    event_loop_group = io.EventLoopGroup(1)
    host_resolver = io.DefaultHostResolver(event_loop_group)
    client_bootstrap = io.ClientBootstrap(event_loop_group, host_resolver)

    mqtt_connection = mqtt_connection_builder.mtls_from_path(
            endpoint=endPt,
            cert_filepath="Certificates/cdc357b6af-certificate.pem.crt",
            pri_key_filepath="Certificates/cdc357b6af-private.pem.key",
            client_bootstrap=client_bootstrap,            
            on_connection_interrupted=on_connection_interrupted,
            on_connection_resumed=on_connection_resumed,
            client_id=clientId,
            clean_session=False,
            keep_alive_secs=6)

    print("Connecting to {} with client ID '{}'...".format(
        endPt, clientId))
    connect_future = mqtt_connection.connect()

    # Future.result() waits until a result is available
    connect_future.result()
    print("Connected!")

    # Parsing the response to get weather attributes
    if x["cod"] != "404": 

        y = x["main"] 

        current_temperature = y["temp"] 
        
        max_temp = y['temp_max']
        
        min_temp = y['temp_min']
        
        feels_like = y['feels_like']

        current_pressure = y["pressure"] 

        current_humidiy = y["humidity"] 

        z = x["weather"] 

        weather_description = z[0]["description"] 

        sun_times = x['sys']

        sunrise = sun_times['sunrise']
        sunset = sun_times['sunset']

        topic="Weather"
        message = {
            'sunrise': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sunrise)),
            'sunset': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sunset)),
            'temperature': current_temperature,
            'max_temperature': max_temp,
            'min_temperature': min_temp,
            'weather_description': weather_description,
            'feels_like': feels_like
        }
        print("Publishing message to topic '{}': {}".format(topic, message))
        mqtt_connection.publish(
            topic=topic,
            payload=str(message),
            qos=mqtt.QoS.AT_LEAST_ONCE)
        
        # Disconnect
        print("Disconnecting...")
        disconnect_future = mqtt_connection.disconnect()
        disconnect_future.result()
        print("Disconnected!")

    else: 
        print("Could not get response from API") 
