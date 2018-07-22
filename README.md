# Anomaly detection and monitoring ![Appveyor](https://ci.appveyor.com/api/projects/status/github/djgorillaz/anomaly-detection-and-monitoring?branch=develop&svg=true)

This program uses client-server model.

* Client collects data (traffic data, logon/logoff time) and sends it to server.
* Server (has GUI) receives data and calculates score, which describes user behaviour anomaly.   Anomaly score is between 0 and 100. Also server can obtain keyboard log and screenshots from clients.

## Getting started

### Download [**executable files**](https://github.com/DjGorillaz/anomaly-detection-and-monitoring/releases/latest).

### Prerequisites
- Windows 7/8/10
- Winpcap [driver](https://www.winpcap.org/install/)

### Usage
    server.exe path port
- path - path, used to receive data
- port - TCP port, used by clients to connect
>
    client.exe path local_port ip dest_port
- path - Default path to collect data
- local_port - TCP port, used to send data
- ip - server's ip
- dest_port - server's port

:exclamation: For correct score measuring it's needed to collect data at least for 7 days of normal user behaviour.

## Program structure

- Client
    - Sniffer module
    - Keylogger module
    - Screenshot module
- Server
    - Score calculation module

See architecture [diagram](/img/architecture.png)

### Sniffer

Collects parameters of traffic going through user's PC:
- Size of incoming traffic
- Number of unique incoming IP-addresses
- Size of outgoing traffic
- Number of unique outgoing IP-addresses

### Keylogger

This module uses filter function, which gets keypresses. This function connects to WH_KEYBOARD_LL hook by using WinAPI function SetWindowsHookEx.

All keypresses are saved in fullData.log. This file contains path to process and data:

    C:\ProgramFiles (x86)\Google\Chrome\Application\chrome.exe 18:12:06 22.07.2018
    Hello, world! 

### Screenshot

Filter function connects to WH_MOUSE_LL hook. All screenshots can be saved by:
- Timer timeout (seconds)
- Mouse clicks (right/left/middle mouse buttons)
- Mouse wheel

### Score

This module gets traffic data, login and logoff time and total session time as input and outputs anomaly score and contributions.

Parameters:
- N - number of observations of normal user behaviour
- d<sub>0</sub> - distance of the 50% score
- k - steepness of the curve, which used to calculate score
* weights - vector used to correct score
* one-sided deviations - used to calculate only positive or negative deviations
* contribution - result of algorithm, contribution rate of each feature

See full [flowchart](/img/flowchart.png) of score measuring
and [anomaly detection algorithm .docx [RU]](/docs/anomaly-detection-algorithm.docx)

### Server's GUI

![Server GUI](/img/server-gui.png)

## Built With
- Eigen3[<sup>1</sup>]
- Libtins[<sup>2</sup>]
- Winpcap[<sup>3</sup>]
- WinAPI
- GDIPlus

[<sup>1</sup>]: http://eigen.tuxfamily.org/index.php?title=Main_Page
[<sup>2</sup>]: http://libtins.github.io/
[<sup>3</sup>]: https://www.winpcap.org/devel.htm

## License
This project is licensed under the MIT License - see the [LICENSE.md](/LICENSE) file for details