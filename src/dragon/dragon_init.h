#ifndef DRAGON_INIT_H
#define DRAGON_INIT_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "dragon.h"

int init_key(Dragon_Ctx *key, Dragon_Ctx *iv, Dragon_Ctx *W, int n);

#endif