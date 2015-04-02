
# Initial Setup

Loaded the debian beagle bone black image on to the internal SD using an extra sd card.

Modified the /etc/network/interfaces file to
* uncomment "auto wlan0"
* have correct wpa-ssid and wpa-psk values
Ran `ifup wlan0`. May have to run `ifconfig wlan0 up` first.
Needed to add a default gateway with `route add default gw 192.168.1.1`.
Or use ethernet w/ autoconfiguration. Way easier.

Installed cmake

Make repos directory and cloned safebike repo. Remember that libswiftnav is a submodule. Need to run `git submodule init` and `git submodules update`.

Compile libswiftnav using their directions (on http://docs.swift-nav.com/libswiftnav/install.html).

Resources:
http://thethingsystem.com/dev/Bootstrapping-the-BeagleBone-Black-with-Debian.html


# Piksi Setup

Since we want to use the piksis to find ground truth on more than one moving object at the same time, we would like to set it up to have a base station that only transmits corrections and multiple rovers. To do this, we need to disable sending corrections back from the rovers. On each rover in settings, set the UARTA mask to 0 (64 by default to send position messages). Optionally, enable base station mode on the base station. Save the settings to flash.

Resources:
https://groups.google.com/forum/#!topic/swiftnav-discuss/CrtmUthvdVA
