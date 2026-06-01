# Autonomous Desktop-Cleaning Robot with Touchless Start and Edge Detection

A self-directed hardware and firmware project built to demonstrate hands-on skills in power systems, embedded programming, and autonomous robotics navigation. The robot operates completely autonomously on an elevated desk, lifting debris via a custom vacuum system, and uses real-time distance calculations to avoid dropping off table edges.

## Demo

<video src="path/to/your/robot_demo.mp4" controls width="100%"></video>

## Features
- **Touchless Activation** — Uses an APDS-9960 gesture sensor to read hand waves, initializing the robot without causing accidental displacement on startup.
- **Autonomous Edge Avoidance** — Built-in ultrasonic arrays monitor depth continuously, prioritizing front-to-side logic blocks to safely alter course.
- **Dual Power Rails** — Motor load power and sensitive microcontroller logic power are isolated from one another to filter voltage drops and electrical noise.
- **Custom Cleaning Core** — Operates a dedicated internal vacuum suction motor funneling particles into an onboard removable debris compartment.

## Hardware

### Robot Components
| Component | Purpose |
| :--- | :--- |
| Arduino Uno REV3 | Main microcontroller, runs real-time sensor processing and motor sequence control. |
| H-Bridge Motor Driver Control | Drives dual independent DC wheel assemblies (supports differential steering control). |
| Gikfun Type 130 DC Motor | High-rpm micro drive motors (2-unit configuration for main wheel movement). |
| DC Fan | Built-in vacuum fan assembly dedicated to generating continuous negative suction air pressure. |
| HC-SR04 Ultrasonic Sensor | Calculates real-time distance boundaries (3 units around perimeter for front, left, right edge scans). |
| Adafruit APDS9960 Gesture Sensor | Reads close-range proximity and direction vectors to trigger contactless starting states. |
| 12V Battery Pack | Dedicated high-capacity supply rail for driver boards, steering motors, and general logic. |
| Allmax 9V Alkaline Battery | Isolated power cell source committed strictly to the separate suction fan motor circuit loop. |
| Chassis / Frame | Laser-cut wood base structural frame. |
| Caster Wheel | 1 unit configuration used at the rear balance point for pivot stability. |
| Vacuum Nozzle | Custom 3D-printed nozzle designed to maximize and concentrate airflow at the desk surface. |
| Mesh Filter Screen | Keeps collected debris contained inside the removable container. |

## Power System Architecture
12V Battery Pack (1200 mAh)  
├── Motor Driver (VM RAW Motor Power)  
└── Arduino Uno Power Line  
&emsp;&emsp;├── APDS9960 Gesture Sensor (3.3V Logic Rail)  
&emsp;&emsp;└── HC-SR04 Sensors Array (5V Logic Rail)

9V Alkaline Battery (720 mAh)
└── DC Vacuum Suction Fan Motor

### Key Design Decisions
- **Separated Motor and Logic Power Rails** — Prevents voltage surges or drops caused by inductive motor spikes from corrupting sensor reads or forcing system micro-resets.
- **Non-Contact Start Sequence** — Safeguards the machine's initial layout trajectory by completely removing physical click pressure.
- **H-Bridge Differential Steering** — Allows localized directional pivots and quick reversing behaviors upon arriving at sudden platform borders.

## Edge Detection Logic
Robot Navigates Forward  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;↓  
Arduino Reads HC-SR04 Array (Front Depth)  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;↓  
Front Edge Detected > 40cm? (Drop-off/Edge Encountered)  
&emsp;&emsp;&emsp;&emsp;&emsp;├── YES → Back up (4s) → Stop → Scan Side Panels  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;↓  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp; Right Edge Detected > 40cm? (Drop-off/Edge Encountered)  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;├── YES → Scan Left Edge  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;↓  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;Left Edge Detected > 40cm? (Drop-off/Edge Encountered)  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;├── YES → Back up (4s) → Stop → Scan Side Panels → Repeat Cycle  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;└── NO → Rotate 180 degrees to the left -> Repeat Cycle  
&emsp;&emsp;&emsp;&emsp;&emsp; &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;└── NO → Rotate 180 degrees to the right -> Repeat Cycle  
&emsp;&emsp;&emsp;&emsp;&emsp;└── NO → Move Forward until front edge is detected

## Wiring

### Arduino UNO Pin Connections
| Pin | Connection |
| :--- | :--- |
| **5V** | Ultrasonic Sensors VCC Supply Line |
| **3.3V** | APDS-9960 Gesture Sensor VCC |
| **GND** | Common System Ground Bus Channel |
| **D13** | Status Indicator LED Node |
| **A3** | HC-SR04 TRIG (Front Sensor Trigger Output) |
| **A2** | HC-SR04 ECHO (Front Sensor Echo Input) |
| **D4** | HC-SR04 TRIG (Left Sensor Trigger Output) |
| **D12** | HC-SR04 ECHO (Left Sensor Echo Input) |
| **A1** | HC-SR04 TRIG (Right Sensor Trigger Output) |
| **A0** | HC-SR04 ECHO (Right Sensor Echo Input) |
| **D9** | H-Bridge Motor Driver IN1 Control Direction |
| **D8** | H-Bridge Motor Driver IN2 Control Direction |
| **D6** | H-Bridge Motor Driver IN3 Control Direction |
| **D5** | H-Bridge Motor Driver IN4 Control Direction |
| **D10** | H-Bridge Motor Driver ENA Speed Governor Pin |
| **D3** | H-Bridge Motor Driver ENB Speed Governor Pin |
| **A4 (SDA)** | APDS-9960 Gesture Sensor Serial Data Interconnect |
| **A5 (SCL)** | APDS-9960 Gesture Sensor Serial Clock Interconnect |

## Software
- **Language:** C++ (Arduino Sketch Architecture)
- **Framework:** Bare-metal polling loop with asynchronous, non-blocking time intervals.
- **Key Libraries:** `Wire.h`, `SparkFun_APDS9960.h`

### Operational Status Codes (LED Indicators)
- **LED Completely Off:** Device is in standby idle state. Wave hand **LEFT → RIGHT** over the gesture node to activate.
- **LED Solidly Illuminating:** Active cleaning session running. The micro-timer tracking is actively executing down from its 2-minute limit threshold.
- **LED Actively Blinking:** Cleaning cycle completed successfully. The system has disengaged running wheels and vacuum motors safely.
