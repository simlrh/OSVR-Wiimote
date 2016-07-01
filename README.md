# OSVR Wiimote Plugin

An OSVR plugin providing up to four Wii Remote + Nunchuk devices via the wiiuse library. There's one orientation only tracker per wiimote or nunchuk, although yaw is only available if using the sensor bar.

Default mappings are for one wiimote in the right hand and nunchuk in the left, but it's more fun with a nunchuk in each hand!

    git clone https://github.com/simlrh/OSVR-Wiimote
	cd OSVR-Wiimote
	git submodule init
	git submoule update
	
Then follow the [standard OSVR plugin build instructions](http://resource.osvr.com/docs/OSVR-Core/TopicWritingDevicePlugin.html).
