/*
 * This file declares a C client library proxy for the interface. The functions
 * are defined in eginterf_client.c.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	/* Device info; must be at the start of all device structures. */
	playerc_device_t info;

	/* Other stuff the proxy should store during its lifetime. */
	uint32_t stuff_count;
	double *stuff;
	int value;
} eginterf_t;

eginterf_t *eginterf_create (playerc_client_t *client, int index);

void eginterf_destroy (eginterf_t *device);

int eginterf_subscribe (eginterf_t *device, int access);

int eginterf_unsubscribe (eginterf_t *device);

int eginterf_cmd (eginterf_t *device, char value);

int eginterf_req (eginterf_t *device, int blah);

#ifdef __cplusplus
}
#endif
