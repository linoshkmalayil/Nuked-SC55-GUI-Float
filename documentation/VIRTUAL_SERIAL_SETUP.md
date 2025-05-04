# Instructions to Setup Virtual Serial I/O

## Linux

1. Install `socat` package on your system.
2. Add your user to `dialout` group in Debian/Fedora based systems or `uucp` group in case of Arch based systems and relogin your user.
3. In the terminal run\
`sudo socat -d -d pty,raw,echo=0,link=/dev/ttyS<port1>,b9600,group-late=<serialgroup>,mode=660 pty,raw,echo=0,link=/dev/ttyS<port2>,b9600,group-late=<serialgroup>,mode=660`\
which will launch `socat` with two virtual serial port pairs `/dev/pts/<vport1>` and `/dev/pts/<vport1>` which symlinks to `/dev/ttyS<port1>` and `/dev/ttyS<port2>`.\
Here `<serialgroup>` must be the group name who owns the serial ports. (`dialout` for Debian/Fedora based , `uucp` for Arch based)
4. Update the permissions of the Serial Ports by running\
`sudo chown -h root:uucp /dev/ttyS<port1>`\
`sudo chown -h root:uucp /dev/ttyS<port2>`
5. Connect one of the `/dev/ttyS` ports to DOSBox, 86Box or other Serial application.
(DOSBox and 86Box only checks for Serial ports in `/dev/ttyS`)
6. Connect Nuked SC-55 to the other port by using `/dev/pts/`.
7. Run Serial MIDI playback on your Serial Application.

## Windows

1. Install Virtual Serial Port driver like [Virtual Serial Port Driver by Electronic Team, Inc.](https://www.eltima.com/products/vspdxp/).
2. Virtual Serial Port driver add a virtual serial port pairs which should create two virtual ports `COM<port1>` and `COM<port2>`. (Usually `COM1` and `COM2` if no other serial ports exist).
3. Connect one of the `COM` to DOSBox, 86Box or other Serial application.
4. Connect Nuked SC-55 to the other `COM` port.
5. Run Serial MIDI playback on your Serial Application.

**NOTE:**
- If you are using DOSBox or MS-DOS in 86Box, [DOSMID](https://dosmid.sourceforge.net/) is one MIDI player that supports Serial Mode MIDI playback.
- If you are running MS-DOS on 86Box make sure to run `MODE COM<port>:<baudrate>,N,8,1,P` to configure the serial port. (Baudrate of 9600 is the common value)
- In Windows incase the playback stutters, please run the following in Powershell: 
```
$port= new-Object System.IO.Ports.SerialPort COM<port>,<baudrate>,None,8,one
$port.Open()
$port.Close()
```
This occurs because Nuked-SC55 doesn't explicitly set the Baudrate when opening the Serial Port. And running these commands in powershell simply sets the serial port mode, opens and closes it. (Baudrate of 9600 is the common value)