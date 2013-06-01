#ifndef SYSTEMTASK_H_
#define SYSTEMTASK_H_


#include "app_config.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "log.h"

#include "jansson.h"
#include "strbuffer.h"
#include "string_utils.h"

#include "messages.h"
#include "transport/transport.h"
#include "methods/methods.h"


typedef enum _system_msg_type_t {
	MSG_TYPE_LOGGING=1
} system_msg_type_t;

typedef struct _system_msg_t {
	system_msg_type_t	msgType;
	union {
		strbuffer_t *logMsg;
	};
} system_msg_t;

inline void system_msg_destroy(system_msg_t **sysMsg);
inline system_msg_t * system_msg_new(system_msg_type_t msgType);

void tskSystem(void *pvParameters);


#endif /* SYSTEMTASK_H_ */
