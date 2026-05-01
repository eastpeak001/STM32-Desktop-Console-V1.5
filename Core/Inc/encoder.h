#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
  ENCODER_NONE = 0,
  ENCODER_CW = 1,
  ENCODER_CCW = -1
} EncoderDirection_t;

void Encoder_Init(void);
void Encoder_Scan(void);
EncoderDirection_t Encoder_GetRotation(void);
uint8_t Encoder_GetButtonPressed(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H */
