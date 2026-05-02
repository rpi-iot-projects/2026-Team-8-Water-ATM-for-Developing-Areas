# 2026-Team-8-Water-ATM-for-Developing-Areas

A smart water dispenser that monitors user access, tracks the volume of water dispensed, and asses water quality.


## Table of Contents

 

- [Overview](#overview)  

- [Hardware Components](#hardware-components)  

- [Software and Dependencies](#software-and-dependencies)  

- [Usage](#usage)  

- [Results and Demonstration](#results-and-demonstration)  



## Overview


This project aims to deliver safe, clean, and accessible drinking water to underserved communities with a smart water dispenser system, or a "water ATM". This water ATM promotes equitable and sustainable access to clean water, minimizing risks of hoarding, misuse, and contamination.

RFID readers are used to allow users to receive water without having to physically contact the system, improving sanitation. Water volume dispensed for each user is recorded with a water flow sensor, and water quality is measured with a turbidity sensor. In addition, this project integrates an MQTT broker and a PostgreSQL database to collect and record real-time data from the sensors deployed across multiple water ATM units.


## Hardware Components


1 ESP32  

1 Solenoid Valve  

1 RFID Reader (RC522)  

1 Water Flow Sensor (YF-S201)  

1 Turbidity Sensor (TS-300B)  

Resistors  

MOSFET  

Breadbords



## Software and Dependencies

 
Mosquitto MQTT Broker  

PostgreSQL database  

### Server/Client Code  

Python 3.11.3  

Packages and versions:  

paho-mqtt == 2.1.0  

pip == 22.3.1  

psycopg2-binary == 2.9.11  

 

## Usage

In the Server/Client Python script, configure the settings for PostgreSQL and MQTT. 
Ensure that the IP address of the MQTT broker and the topic in this script matches the 
ESP32 code. This script also assumes that the following tables in the database already exist.  
 
Table "public.users":
| Column |  Type   | Collation | Nullable | Default |
|--------|---------|-----------|----------|---------|
| id     | text    |           | not null | |
| count  | integer |           |          | 1 |

  
Table "public.history":
| Column  |            Type             | Collation | Nullable |      Default  |
|---------|-----------------------------|-----------|----------|-------------------  |
| user_id | text                        |           | not null |  |
| time    | timestamp without time zone |           | not null | CURRENT_TIMESTAMP |
| atm_id  | integer                     |           | not null | |
| volume  | integer                     |           |          | |

  
Table "public.water_quality":
| Column |  Type   | Collation | Nullable | Default |
|--------|---------|-----------|----------|---------|
| atm_id | integer |           | not null | |
| ntu    | real    |           |          | |


## Results and Demonstration

After scanning a RFID tag the valve opens and allows water to flow through for 15 seconds. After
that the ESP32 established a connection to the MQTT broker and publishes data from its various 
sensors. If the message is received successfully by the server, its contents will be printed out 
on the terminal along with the changes made to each table.



