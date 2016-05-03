#include <X11/Xlib.h>
#include <libupower-glib/upower.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define arraylen(x) (sizeof((x)) / sizeof((x)[0]))

size_t timeAction(char* buffer, int bufferLen)
{
	const char* defaultTimeFormat = "%H:%M %a %b %d";

	time_t theTime = time(NULL);
	size_t timeLen = strftime(buffer, bufferLen, defaultTimeFormat, localtime(&theTime));
	assert(timeLen != 0);
	return timeLen;
}

UpClient* upClient;

void batteryActionInit()
{
	upClient = up_client_new();
}

size_t batteryAction(char* buffer, int bufferLen)
{
	UpDevice* battery = NULL;
	GPtrArray* devices = up_client_get_devices(upClient);
	for (int i = 0; i < devices->len; i++) {
		UpDevice* device = g_ptr_array_index(devices, i);

		UpDeviceKind kind;
		gboolean isPresent;
		g_object_get(device, "kind", &kind, "is-present", &isPresent, NULL);

		if (kind != UP_DEVICE_KIND_BATTERY || !isPresent)
			continue;

		battery = device;
		break;
	}
	g_ptr_array_unref(devices);

	if (battery == NULL)
		return 0;

	gdouble percentage;
	g_object_get(battery, "percentage", &percentage, NULL);

	guint state;
	g_object_get(battery, "state", &state, NULL);

	const char charging[UP_DEVICE_STATE_LAST - UP_DEVICE_STATE_UNKNOWN] = "?+-!=+-";
	state -= UP_DEVICE_STATE_UNKNOWN;

	return snprintf(buffer, bufferLen, " %02d%%%c", (int)percentage, charging[state > arraylen(charging) ? 0 : state]);
}

int main(int argc, char* argv[])
{
	Display* dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "%s: unable to open display '%s'\n", argv[0], XDisplayName(NULL));
		exit(-1);
	}
	int screen = DefaultScreen(dpy);
	Window root = RootWindow(dpy, screen);

	daemon(0, 0);

	batteryActionInit();

	char buffer[256];
	do {
		char* b = buffer;
		size_t rem = arraylen(buffer);

		size_t count = timeAction(b, rem);
		b = &b[count];
		rem -= count;

		batteryAction(b, rem);

		int status = XStoreName(dpy, root, buffer);
		if (status == 0) {
			fprintf(stderr, "%s: unable to set root window name\n", argv[0]);
			exit(-1);
		}
		XFlush(dpy);

		sleep(60);
	} while (1);

	return 0;
}
