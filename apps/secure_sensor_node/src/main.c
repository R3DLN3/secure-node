#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(secure_sensor_node, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Secure Sensor Node starting...");
    LOG_INF("Milestone 2: app builds from repo.");

    int counter = 0;

    while (1) {
        LOG_INF("Heartbeat %d", counter++);
        k_sleep(K_SECONDS(2));
    }
}
