// Leonid Moguchev (c) 2020
#pragma once

#include "pch.h"

class Uart
{
public:
	Uart();
	void reset();
	void rxBit(bool bit);
	bool txBit();
	void txDat(uint8_t dat);
	bool rxCompleted;
	bool txCompleted;
	uint8_t rxData;
	uint8_t txData;
private:
	uint8_t rxState;
	uint8_t txState;
};

