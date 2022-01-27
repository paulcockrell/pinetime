#include <lvgl.h>

#include "app.h"

#define APP_STACK_SIZE      1024
#define APP_PRIORITY        5

static struct app_context context;

int app_init(app_t *app, lv_obj_t * parent) {
    return app->spec->init(app, parent);
}

int app_update(app_t *app) {
    return app->spec->update(app);
}

int app_gesture(app_t *app, enum appGestures gesture) {
    return app->spec->gesture(app, gesture);
}

int app_close(app_t *app) {
    return app->spec->close(app);
}

static void run_app(app_t *app)
{
	if (app == pinetimecosapp.runningApp) {
		return;
	}
	pinetimecos.lvglstate = Sleep;
	lv_timer_pause(app_timer);
	if (pinetimecosapp.runningApp) {
		UNUSED_VARIABLE(app_close(pinetimecosapp.runningApp));
	}
	pinetimecosapp.runningApp = app;
	UNUSED_VARIABLE(app_init(pinetimecosapp.runningApp, main_scr));
	lv_timer_set_period(app_timer, pinetimecosapp.runningApp->spec->updateInterval);

	pinetimecos.lvglstate = Running;
	lv_timer_resume(app_timer);
}

static void return_app(enum apps app/*, enum appGestures gesture, enum RefreshDirections dir*/)
{
	pinetimecosapp.returnApp = app;
	//pinetimecosapp.returnAnimation = dir;
	//pinetimecosapp.returnDir = gesture;
}

void load_application(enum apps app/*, enum RefreshDirections dir*/)
{
    //set_refresh_direction(dir);
    //pinetimecosapp.activeApp = app;
	switch (app) {
/*
	case Passkey:
		run_app(APP_PASSKEY);
		return_app(Clock, DirBottom, AnimNone);
		break;

	case Debug:
		run_app(APP_DEBUG);
		return_app(Menu, DirTop, AnimUp);
		break;

	case Clock:
		run_app(APP_CLOCK);
		return_app(Clock, DirNone, AnimNone);
		break;

	case Menu:
		run_app(APP_MENU);
		return_app(Clock, DirTop, AnimUp);
		break;

	case Notification:
		run_app(APP_NOTIFICATION);
		return_app(Clock, DirBottom, AnimDown);
		break;
*/
	default:
		break;
	}
}

void app_push_message(enum appMessages msg)
{

}

void app_thread(void* arg1, void *arg2, void *arg3)
{
	struct app_context *context = (struct app_context*)arg1;
	const struct device *display_dev = NULL;

	display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);
	if (display_dev == NULL) {
		LOG_ERR("device %s not found", CONFIG_LVGL_DISPLAY_DEV_NAME);
		return;
	}

	display_blanking_off(display_dev);

	context->state = APP_RUNNING;

	while (1) {
		k_sleep(K_MSEC(500));
	}
}
K_THREAD_DEFINE(app, APP_STACK_SIZE, app_thread, &context, NULL, NULL, APP_PRIORITY, 0, 0);
