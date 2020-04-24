from awscrt import io, mqtt, auth, http
from awsiot import mqtt_connection_builder
import csv
import time
from datetime import datetime


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
            
endPt = "a37g54y6ddcht7-ats.iot.us-east-1.amazonaws.com"
clientId = "samples-client-id"


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

with open('weather_data.csv', mode='r') as file:
    reader = csv.DictReader(file)
    for row in reader:
        sunset = row['sunset']

topic_sunset = "Sunset"
message_sunset = sunset
print("Publishing message to topic '{}': {}".format(topic_sunset, message_sunset))
mqtt_connection.publish(
            topic=topic_sunset,
            payload=message_sunset,
            qos=mqtt.QoS.AT_LEAST_ONCE)

with open('log_file.txt', mode='a') as file:
        file.write('Updated at ' + datetime.now().strftime("%m/%d/%Y, %H:%M:%S") + ' : Publishing topic Sunset Successful \n')

# Disconnect
print("Disconnecting...")
disconnect_future = mqtt_connection.disconnect()
disconnect_future.result()
print("Disconnected!")
