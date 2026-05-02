import paho.mqtt.client as mqtt
import psycopg2
import json

# PostgreSQL settings
conn = psycopg2.connect(
    dbname="water_atm",
    user="username",
    password="password",
    host="host ip",
    port="port number"
)

# MQTT settings
broker = "broker ip address"
topic = "esp32/wateratm"

# Create a cursor
cur = conn.cursor()

def on_message(client, userdata, msg) -> None:
    """
    Called whenever a message arrives.
    Parses input JSON payload received from MQTT broker
    and stores values into their respective tables in the
    PostgreSQL database.

    :param client: MQTT client instance
    :param userdata: custom data attached to the client
    :param msg: object of received message
    """

    data = msg.payload.decode()
    print("Received: ", data)
    

    # --- parse message ---

    status = ""
    uid = ""
    atmid = 0
    mL = 0
    turbitity_ntu = 0.0
    
    try:
        parsed = json.loads(data)
        status = parsed.get("status")
        uid = parsed.get("uid")
        atmid = parsed.get("atmid")
        mL = parsed.get("mL")
        turbitity_ntu = parsed.get("turbitity_ntu")

        print("status: ", status) 
        print("uid: ", uid)
        print("atmid: ", atmid)
        print("mL: ", mL)
        print("turbitity_ntu: ", turbitity_ntu)

    except json.JSONDecodeError:
        print("Invalid JSON received")

    # --- check if user exists in database ---
    cur.execute("SELECT 1 FROM users WHERE id = %s;", (uid,))
    exists = cur.fetchone()

    # --- update count ---
    if exists:
        # increment number of times user has accessed this atm
        print("Updating user count")
        cur.execute(
            "UPDATE users SET count = count + 1 WHERE id = %s;",
            (uid,)
        )
    else:
        # user does not exist in database yet
        # create user and set count to 1
        print("Creating row for userid ", (uid))
        cur.execute(
            "INSERT INTO users (id, count) VALUES (%s, 1);",
            (uid,) 
        )


    # --- log timestamp and volume --- 
    cur.execute(
        "INSERT INTO history (user_id, atm_id, volume) VALUES (%s, %s, %s);",
        (uid, atmid, mL)
    )

    # --- check if atm already exists in database ---
    cur.execute("SELECT 1 FROM water_quality WHERE atm_id = %s;", (atmid,))
    exists = cur.fetchone()

    # --- update turbitity readings ---
    if exists:
        print("Updating turbitity reading")
        cur.execute(
            "UPDATE water_quality SET ntu = %s WHERE atm_id = %s;",
            (turbitity_ntu, atmid)
        )
    else:
        # atm is not listed in database yet
        print("Creating row for atmid ", (atmid))
        cur.execute(
            "INSERT INTO water_quality (atm_id, ntu) VALUES (%s, %s);",
            (atmid, turbitity_ntu) 
        )

    # Commit the transaction
    conn.commit()

    print("success!")

client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.connect(broker, 1883, 60)
client.subscribe(topic)
client.on_message = on_message

client.loop_forever()
