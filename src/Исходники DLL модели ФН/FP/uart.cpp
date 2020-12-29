// Leonid Moguchev (c) 2020
#include "pch.h"
#include "uart.h"

Uart::Uart() {
	reset();
}

void Uart::reset() {
	rxCompleted = false;
	rxData = 0x0;
	rxState = 0;

	txCompleted = true;
	txData = 0x0;
	txState = 0;
}

void Uart::txDat(uint8_t data) {
	txData = data;
	txState = 1;
}

bool Uart::txBit() {
	if (!txCompleted) { // передача данных
		if (txState == 0) { // start bit
			txState++;
			return false; // start bit = 0
		}
		if (txState <= 8) { // data bit
			// получение нужного бита байта
			return 0 != (txData & (1 << (txState++ - 1)));
		} else { // stop bit
			txCompleted = true;
			txState = 0;
			return true;
		}
	} else { // высокий уровень сигнала (передачи нет)
		return true;
	}
}

void Uart::rxBit(bool bit) {
	if (rxState > 0) { // мы читаем
		if (rxState <= 8) {
			rxData |= (bit << (rxState - 1));
			rxState++;
		} else { // закончили чтение
			rxState = 0;
			rxCompleted = true;
		}
	} else {
		if (!bit) { // получили старт бит
			rxState = 1; // режим чтения
			rxData = 0x0;
		}
	}
}
