#ifndef _EVENTIO_H_
#define _EVENTIO_H_

#include <stdint.h>

#define EVT_BEGIN           1
#define EVT_NEW_LABEL       2
#define EVT_OPCODE          4
#define EVT_REG_LITERAL     8
#define EVT_NUM_LITERAL     16
#define EVT_LABEL_REF       32
#define EVT_BEGIN_ADDRESS   64
#define EVT_END_ADDRESS     128
#define EVT_STR_LITERAL     256
#define EVT_END             0
#define EVT_ERROR           511

int consume_whitespace(FILE *f);
int read_event_content(FILE *f, char *buf, int len);
int get_event_type(char *contents);

#endif
