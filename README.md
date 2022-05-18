# pkt_tx_rx_gr740_test

## About this project 

A simple project to transmit and receive packets on the spacewire ports of the GR740 board with RTEMS Cross Compiler (RCC)
<br>
This project is mainly based on the grspw-test folder available in the rcc distribution.<br>
The example has been simplified in order to provide a clear interface where the user can choose:
- the source port;
- the destination port;
- the number of packets to transmit.

## Example

Here is an example of an output (18/05/22):

```

Setting up SpaceWire router
Configuring Router
Hardware Configuration of SpaceWire Router:
 Number of SpW ports:           8
 Number of AMBA ports:          4
 Number of FIFO ports:          0
 Timers available:              YES
 Plug and Play available:       YES
 MAJOR Version:                 1
 MINOR Version:                 2
 PATCH Version:                 0
 Current Instance ID:           19
ROUTER CFG/STS: 0x4100c013
PORT[00]:  TYPE=CFG  CTRL=0x00000200  STATUS=0x00000000  LINKSTATE=N/A
PORT[01]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[02]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[03]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[04]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[05]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[06]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[07]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[08]:  TYPE=SpW  CTRL=0x2700022c  STATUS=0x0000a000  LINKSTATE=Ready
PORT[09]:  TYPE=AMBA CTRL=0x00000228  STATUS=0x40000000  LINKSTATE=N/A
PORT[10]:  TYPE=AMBA CTRL=0x00000228  STATUS=0x40000000  LINKSTATE=N/A
PORT[11]:  TYPE=AMBA CTRL=0x00000228  STATUS=0x40000000  LINKSTATE=N/A
PORT[12]:  TYPE=AMBA CTRL=0x00000228  STATUS=0x40000000  LINKSTATE=N/A
Activating all Links/Ports
PORT[01]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x0000b000   LINK-STATE=Started
PORT[02]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x00009000   LINK-STATE=Error Wait
PORT[03]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x0000d000   LINK-STATE=Run
PORT[04]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x00009000   LINK-STATE=Error Wait
PORT[05]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x00008000   LINK-STATE=Error Reset
PORT[06]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x0000d000   LINK-STATE=Run
PORT[07]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x0000b000   LINK-STATE=Started
PORT[08]:  TYPE=SpW    CTRL=0x0100002e   STATUS=0x00008000   LINK-STATE=Error Reset
 Initializing SpaceWire device 0
 After Link Start: 0
 Initializing SpaceWire device 1
 After Link Start: 0
 Initializing SpaceWire device 2
 After Link Start: 0
 Initializing SpaceWire device 3
 After Link Start: 0


Starting SpW DMA channels
Starting GRSPW0: DMA Started Successfully
Starting GRSPW1: DMA Started Successfully
Starting GRSPW2: DMA Started Successfully
Starting GRSPW3: DMA Started Successfully
Started link control task
ROUTER SpW PORT3: link state entering run-state
ROUTER SpW PORT6: link state entering run-state
Started DMA control task

***********  PKT TX/RX TEST  **************

SPW src port : 3
SPW dest port : 6
4 pkts are waiting for transmission

------ PKT 1 ------
-------------------
TX on GRSPW device 0 (AMBA port 1)
GRSPW0: Sending 1 packets
 PKT of length 33 bytes:  0x03 0x06 0x9b 0xde 0xad 0xfa 0xce 0xde...
GRSPW2: Received 1 packets
 PKT of length 30 bytes: 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 0xfa 0xce 0xde 0xad 

------ PKT 2 ------
-------------------
TX on GRSPW device 3 (AMBA port 4)
GRSPW3: Sending 1 packets
 PKT of length 33 bytes:  0x03 0x06 0x9b 0xac 0xe0 0xfc 0xea 0xac...
GRSPW2: Received 1 packets
 PKT of length 30 bytes: 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 0xfc 0xea 0xac 0xe0 

------ PKT 3 ------
-------------------
TX on GRSPW device 2 (AMBA port 3)
GRSPW2: Sending 1 packets
 PKT of length 33 bytes:  0x03 0x06 0x9b 0xbe 0x0a 0xca 0xde 0xbe...
GRSPW2: Received 1 packets
 PKT of length 30 bytes: 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 0xca 0xde 0xbe 0x0a 

------ PKT 4 ------
-------------------
TX on GRSPW device 1 (AMBA port 2)
GRSPW1: Sending 1 packets
 PKT of length 33 bytes:  0x03 0x06 0x9b 0xca 0xfe 0xfa 0xde 0xca...
GRSPW2: Received 1 packets
 PKT of length 30 bytes: 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 0xfa 0xde 0xca 0xfe 

 Link control task shutdown
 DMA task shutdown


EXAMPLE COMPLETED.

```
