// Leonid Moguchev (c) 2020
#pragma once

#include "pch.h"

#include <cinttypes>
#include <string>
#include <queue>

#include <stdint.h>
#include <windows.h>

#include "logger.h"
#include "vsm.h"
#include "uart.h"
#include "fiscal.h"

const int RXD_EVENT = 1;
const int TXD_EVENT = 2;

const long long ONE_SECOND = 1000000000000;
const long long UART_BAUDRATE = 10000;


class FP : public IDSIMMODEL {
public:
	INT isdigital(CHAR* pinname);
	VOID setup(IINSTANCE* inst, IDSIMCKT* dsim);
	VOID runctrl(RUNMODES mode);
	VOID actuate(REALTIME time, ACTIVESTATE newstate);
	BOOL indicate(REALTIME time, ACTIVEDATA* data);
	VOID simulate(ABSTIME time, DSIMMODES mode);
	VOID callback(ABSTIME time, EVENTID eventid);
	VOID callback_rxd(ABSTIME time, EVENTID eventid);
	VOID callback_txd(ABSTIME time, EVENTID eventid);
	virtual ~FP();
private:
	IINSTANCE* _inst; // Экземпдяр интерфейса модели
	IDSIMCKT* _ckt;   // Экземпляр ЦИФРОВОГО интерфейса модели
	ABSTIME _time;    // Абсолюьное время относительно старта симуляции

	Logger* _log;
	Uart* _uart;
	Fiscal* _fiscal;

	std::queue<uint8_t> _in_buffer;
	std::queue<uint8_t> _out_buffer;

	IDSIMPIN* _pin_d_plus;
	IDSIMPIN* _pin_d_minus;
	IDSIMPIN* _pin_vbus;
	IDSIMPIN* _pin_select;
	IDSIMPIN* _pin_scl;
	IDSIMPIN* _pin_sda;
	IDSIMPIN* _pin_rxd;
	IDSIMPIN* _pin_txd;
};
