// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>
#include <osvr/PluginKit/AnalogInterfaceC.h>
#include <osvr/PluginKit/ButtonInterfaceC.h>

// Generated JSON header file
#include "je_nourish_wiimote_json.h"

// Library/third-party includes
#include "wiiuse.h"

// Standard includes
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#define MAX_WIIMOTES 4
#define TRACKERS_PER_WIIMOTE 2
#define BUTTONS_PER_WIIMOTE 13
#define ANALOG_PER_WIIMOTE 5

#define IS_PRESSED_OSVR(device, button) (IS_PRESSED((device),(button)) ? OSVR_BUTTON_PRESSED : OSVR_BUTTON_NOT_PRESSED)

// Anonymous namespace to avoid symbol collision
namespace {

	class WiimoteDevice {
	public:
		WiimoteDevice(OSVR_PluginRegContext ctx) {
			int found, connected;

			m_wiimotes = wiiuse_init(MAX_WIIMOTES);
			found = wiiuse_find(m_wiimotes, MAX_WIIMOTES, 5);
			connected = wiiuse_connect(m_wiimotes, MAX_WIIMOTES);

			wiiuse_set_leds(m_wiimotes[0], WIIMOTE_LED_1);
			wiiuse_set_leds(m_wiimotes[1], WIIMOTE_LED_2);
			wiiuse_set_leds(m_wiimotes[2], WIIMOTE_LED_3);
			wiiuse_set_leds(m_wiimotes[3], WIIMOTE_LED_4);

			wiiuse_motion_sensing(m_wiimotes[0], 1);
			wiiuse_motion_sensing(m_wiimotes[1], 1);
			wiiuse_motion_sensing(m_wiimotes[2], 1);
			wiiuse_motion_sensing(m_wiimotes[3], 1);

			/// Create the initialization options
			OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

			osvrDeviceTrackerConfigure(opts, &m_tracker);
			osvrDeviceAnalogConfigure(opts, &m_analog, 20);
			osvrDeviceButtonConfigure(opts, &m_button, 52);

			/// Create the device token with the options
			m_dev.initAsync(ctx, "WiimoteDevice", opts);

			/// Send JSON descriptor
			m_dev.sendJsonDescriptor(je_nourish_wiimote_json);

			/// Register update callback
			m_dev.registerUpdateCallback(this);
		}

		~WiimoteDevice() {
			wiiuse_cleanup(m_wiimotes, MAX_WIIMOTES);
		}

		OSVR_ReturnCode update() {
			struct wiimote_t* wm;
			OSVR_Vec3 wm_rpy;
			OSVR_Quaternion wm_quaternion;
			OSVR_Vec3 nc_rpy;
			OSVR_Quaternion nc_quaternion;

			osvrVec3Zero(&wm_rpy);
			osvrQuatSetIdentity(&wm_quaternion);
			osvrVec3Zero(&nc_rpy);
			osvrQuatSetIdentity(&nc_quaternion);

			if (wiimote_connected()) {
				wiiuse_poll(m_wiimotes, MAX_WIIMOTES);

				OSVR_ButtonState buttons[BUTTONS_PER_WIIMOTE * MAX_WIIMOTES];
				OSVR_AnalogState analog[ANALOG_PER_WIIMOTE * MAX_WIIMOTES];

				for (int i = 0; i < MAX_WIIMOTES; i++) {
					wm = m_wiimotes[i];

					if (wm->event == WIIUSE_EVENT) {
						buttons[BUTTONS_PER_WIIMOTE*i + 0] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_A);
						buttons[BUTTONS_PER_WIIMOTE*i + 1] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_B);
						buttons[BUTTONS_PER_WIIMOTE*i + 2] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_UP);
						buttons[BUTTONS_PER_WIIMOTE*i + 3] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_DOWN);
						buttons[BUTTONS_PER_WIIMOTE*i + 4] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_LEFT);
						buttons[BUTTONS_PER_WIIMOTE*i + 5] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_RIGHT);
						buttons[BUTTONS_PER_WIIMOTE*i + 6] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_ONE);
						buttons[BUTTONS_PER_WIIMOTE*i + 7] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_TWO);
						buttons[BUTTONS_PER_WIIMOTE*i + 8] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_MINUS);
						buttons[BUTTONS_PER_WIIMOTE*i + 9] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_PLUS);
						buttons[BUTTONS_PER_WIIMOTE*i + 10] = IS_PRESSED_OSVR(wm, WIIMOTE_BUTTON_HOME);

						if (WIIUSE_USING_ACC(wm)) {

							osvrVec3SetX(&wm_rpy, -wm->orient.roll * M_PI / 180);
							osvrVec3SetY(&wm_rpy, -wm->orient.pitch * M_PI / 180);
							osvrVec3SetZ(&wm_rpy, -wm->orient.yaw * M_PI / 180);

							WiimoteDevice::quaternionFromRPY(&wm_rpy, &wm_quaternion);

							osvrDeviceTrackerSendOrientation(m_dev, m_tracker, &wm_quaternion, (TRACKERS_PER_WIIMOTE * i));
						}

						if (WIIUSE_USING_IR(wm)) {
							analog[ANALOG_PER_WIIMOTE*i + 0] = wm->ir.x;
							analog[ANALOG_PER_WIIMOTE*i + 1] = wm->ir.y;
							analog[ANALOG_PER_WIIMOTE*i + 2] = wm->ir.z;
						}
						else {
							analog[ANALOG_PER_WIIMOTE*i + 0] = 0;
							analog[ANALOG_PER_WIIMOTE*i + 1] = 0;
							analog[ANALOG_PER_WIIMOTE*i + 2] = 0;
						}

						if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
							struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

							buttons[BUTTONS_PER_WIIMOTE*i + 11] = IS_PRESSED_OSVR(nc, NUNCHUK_BUTTON_C);
							buttons[BUTTONS_PER_WIIMOTE*i + 12] = IS_PRESSED_OSVR(nc, NUNCHUK_BUTTON_Z);

							analog[ANALOG_PER_WIIMOTE*i + 3] = nc->js.x;
							analog[ANALOG_PER_WIIMOTE*i + 4] = nc->js.y;

							osvrVec3SetX(&nc_rpy, -nc->orient.roll * M_PI / 180);
							osvrVec3SetY(&nc_rpy, -nc->orient.pitch * M_PI / 180);
							osvrVec3SetZ(&nc_rpy, -nc->orient.yaw * M_PI / 180);

							WiimoteDevice::quaternionFromRPY(&nc_rpy, &nc_quaternion);

							osvrDeviceTrackerSendOrientation(m_dev, m_tracker, &nc_quaternion, (TRACKERS_PER_WIIMOTE * i) + 1);
						} 
						else {
							buttons[BUTTONS_PER_WIIMOTE*i + 11] = OSVR_BUTTON_NOT_PRESSED;
							buttons[BUTTONS_PER_WIIMOTE*i + 12] = OSVR_BUTTON_NOT_PRESSED;
							analog[ANALOG_PER_WIIMOTE*i + 3] = 0;
							analog[ANALOG_PER_WIIMOTE*i + 4] = 0;
						}
					} 
				}

				osvrDeviceAnalogSetValues(m_dev, m_analog, analog, ANALOG_PER_WIIMOTE * MAX_WIIMOTES);
				osvrDeviceButtonSetValues(m_dev, m_button, buttons, BUTTONS_PER_WIIMOTE * MAX_WIIMOTES);
			}

			return OSVR_RETURN_SUCCESS;
		}

	private:
		bool wiimote_connected() {
			for (int i = 0; i < MAX_WIIMOTES; i++) {
				if (m_wiimotes[i] && WIIMOTE_IS_CONNECTED(m_wiimotes[i])) {
					return true;
				}
			}
			return false;
		}
		static void quaternionFromRPY(OSVR_Vec3* rpy, OSVR_Quaternion* quaternion) {
			double r = osvrVec3GetX(rpy);
			double p = osvrVec3GetY(rpy);
			double y = osvrVec3GetZ(rpy);

			osvrQuatSetZ(quaternion, sin(r / 2) * cos(p / 2) * cos(y / 2) - cos(r / 2) * sin(p / 2) * sin(y / 2));
			osvrQuatSetX(quaternion, cos(r / 2) * sin(p / 2) * cos(y / 2) + sin(r / 2) * cos(p / 2) * sin(y / 2));
			osvrQuatSetY(quaternion, cos(r / 2) * cos(p / 2) * sin(y / 2) - sin(r / 2) * sin(p / 2) * cos(y / 2));
			osvrQuatSetW(quaternion, cos(r / 2) * cos(p / 2) * cos(y / 2) + sin(r / 2) * sin(p / 2) * sin(y / 2));
		}

		wiimote** m_wiimotes;
		osvr::pluginkit::DeviceToken m_dev;
		OSVR_TrackerDeviceInterface m_tracker;
		OSVR_AnalogDeviceInterface m_analog;
		OSVR_ButtonDeviceInterface m_button;
	};

	class HardwareDetection {
	public:
		HardwareDetection() : m_found(false) {}
		OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {

			if (!m_found) {
				m_found = true;

				osvr::pluginkit::registerObjectForDeletion(
					ctx, new WiimoteDevice(ctx));
			}
			return OSVR_RETURN_SUCCESS;
		}

	private:
		bool m_found;

	};
} // namespace

OSVR_PLUGIN(je_nourish_wiimote) {
	osvr::pluginkit::PluginContext context(ctx);

	/// Register a detection callback function object.
	context.registerHardwareDetectCallback(new HardwareDetection());

	return OSVR_RETURN_SUCCESS;
}
