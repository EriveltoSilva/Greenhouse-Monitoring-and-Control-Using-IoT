<h1 align="center" style="font-weight: bold;">Greenhouse Monitoring and Control Using IoT 💻</h1>

[![Maintenance](https://img.shields.io/badge/Maintained%3F-no-red.svg)](https://bitbucket.org/lbesson/ansi-colors) &ensp; &nbsp;[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)&ensp; &nbsp;[![Ask Me Anything !](https://img.shields.io/badge/Ask%20me-anything-1abc9c.svg)](http://www.linkedin.com/in/erivelto-silva-39a61a275) &ensp; &nbsp;![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)


`Menu:`
<ol>
 <li><a href="#started">Getting Started</a> </li> 
 <li><a href="#features">Features</a>       </li> 
 <li><a href="#benefits">Benefits</a>       </li> 
 <li><a href="#usedComponents">Used Components</a> </li>
 <li><a href="#electricalDiagram">Electrical Diagram</a>  </li> 
 <li><a href="#simulation">Simulation</a></li>
 <li><a href="#collaborators">Collaborators</a></li> 
 <li><a href="#license">License</a></li> 
 <li><a href="#keyWords">Key words</a> </li> 
</ol>

<p align="left">
  <strong>The goal of this project is to develop a greenhouse monitoring and control system using the IoT platform. The system aims to optimize the greenhouse environment for plant growth by automatically controlling temperature, humidity, and irrigation, in addition to providing remote monitoring and safety alerts.</strong>
</p>

<h2 id="features">⚙️ Project  Features</h2>

- <strong>Environmental Monitoring:</strong>
    - Temperature and humidity sensors monitor conditions inside the greenhouse in real time.
    - Collected data is displayed on a local LCD display and sent to the Blynk platform for remote viewing.

- <strong>Climate Control:</strong>
  - Temperature is maintained between 22°C and 28°C:
    - An incandescent lamp is activated when the temperature drops below 22°C.
    - A fan and windows are opened when the temperature exceeds 28°C.

- <strong>Irrigation Control:</strong>
  - Soil moisture is monitored using a specific sensor.
  - A water pump is automatically activated to irrigate the soil when moisture drops below 65%.
  - The pump is deactivated when soil moisture reaches 65% or higher.

- <strong>Fire Safety:</strong>
  - Smoke and fire sensors detect the presence of fire in the greenhouse.
  - In case of fire, the water pump is automatically activated to assist in extinguishing the flames.

- <strong>Remote Monitoring:</strong>
  - The Blynk platform allows remote monitoring of greenhouse conditions in real time.
  - Users can view graphs of temperature, humidity, and system status (irrigation, ventilation, etc.).
  - Fire alerts are sent to users via Blynk push notifications.

<h2 id="benefits">🗒️ Benefits</h2>

- <strong>Optimized Plant Growth:</strong> Precise control of temperature, humidity, and irrigation ensures an ideal environment for plant growth.

- <strong>Reduced Losses:</strong> Proactive monitoring and control minimize the risk of diseases and pests, optimizing greenhouse productivity.

- <strong>Resource Savings:</strong> The system utilizes resources efficiently, reducing water and energy consumption.

- <strong>Enhanced Safety:</strong> Fire sensors and automatic water pump activation ensure greater safety for the greenhouse and plants.

- <strong>Convenience and Remote Control:</strong> Remote monitoring via Blynk allows users to track greenhouse health and make timely decisions from anywhere.


<h2 id="usedComponents">🗒️ Used Components</h2>

- ESP32 (main microcontroller)
- Temperature and humidity sensors (DHT11)
- Smoke sensors
- Fire sensors
- Servo motors
- Soil moisture sensor
- Fan
- Water pump
- Incandescent lamp
- LCD display
- Blynk platform


<h2 id="started">🚀 Getting started</h2>

1. Clone this repository to your local directory.
2. Open the project in `Programa/Programa.ino`  in the Arduino IDE.
3. Configure the ESP32 board and the correct serial port.
4. Upload the code to the ESP32 board.
5. Connect the sensors, LEDs and buzzers, lcd, lights  to the ESP32. ESP32 board according to the instructions in the code.
7. The system will be ready to use!



<h2 id="electricalDiagram"> 🖼️ Electrical Diagram</h2>
<img src="./Docs/Esquema electrico.png"  alt="Electrical Diagram" />

<h2 id="simulation">Simulation</h2>

[See it simulation working on my youtube channel](#) 


<h2 id="collaborators">🤝 Collaborators</h2>

Special thank you for all people that contributed for this project.

<table>
  <tr>
    <td align="center">
      <a href="#">
        <img src="https://avatars.githubusercontent.com/u/125351173?s=400&u=d6b335b5e253bb45f3ed82ec322684ab203b5243&v=4" width="100px;" alt="Erivelto Silva Profile Picture"/><br>
        <sub>
          <b>Erivelto Silva</b>
        </sub>
      </a>
    </td>
    <td align="center">
      <a href="#">
        <img src="" width="100px;" alt="Osvaldo Manual Picture"/><br>
        <sub>
          <b>Osvaldo Napoleão Manual</b>
        </sub>
      </a>
    </td>
  </tr>
</table>

<h2 id="license"> LICENSE</h2>

This project is licensed under the [MIT License](LICENSE.txt) © Erivelto Silva.


<h2 id="keyWords">Key Words</h2>

Keys: ESP32, greenhouse, monitoring, control, soil moisture, sensors, LEDs, alarm, Blynk.

