/* Standard includes. */
#include <string.h>
#include <stdio.h>


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
#include "system/error.h"

/* The time between cycles of the 'check' task - which depends on whether the
check task has detected an error or not. */
#define taskCHECK_DELAY_NO_ERROR			( ( portTickType ) 5000 / portTICK_RATE_MS )
#define taskCHECK_DELAY_ERROR				( ( portTickType ) 500 / portTICK_RATE_MS )

//typedef struct {
//	int length;
//	char *data;
//} read_string_t;

#define BUFF_SIZE	64

extern void tskLedBlinkTask(void *);


extern void tskParseJson(void *);

extern void tskUART1Reader(void *);

void tskHandleRequests(void*);
void tskHandleResponses(void *);

int sendOutputMessage(packet_t *msgPacket);
int add_new_rpc_request(packet_t *incomePacket);

