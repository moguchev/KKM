// Leonid Moguchev (c) 2020

#include "pch.h"
#include "fp.h"
#include "logger.h"

#include <string>

const auto HEX = "0123456789ABCDEF";

std::string byte_to_hex(uint8_t c) {
	return string_format("0x%c%c", HEX[(c >> 4) & 0xF], HEX[c & 0xF]);
}

uint8_t getBit(IDSIMPIN* pin) {
	if (pin->isactive()) {
		return 0x00000001;
	}
	return 0x00000000;
}

INT FP::isdigital(CHAR* pinname) {
	return TRUE;
	//--- возможны варианты:
	//---  1 вывод является цифровым или смешанного типа.
	//---  0 вывод является аналоговым или не распознаётся моделью.
	//--- -1 вывод может быть как аналоговым, так и цифровым, в зависимости от контекста.
}

// линковка выходов схемы в Proteus и в коде
//--- Эта функция вызывается PROSPICE как только установлено, что компонент имеет
//--- один или более цифровых выводов.
//--- Модели передаётся указатель на примитив симулятора к который она присоединена
//--- и на DSIM цепь, которая её содержит. По сути - это начальная инициаллизация...
// IINSTANCE* instance - указатель на примитив симулятатора, к которой модель приосоединена.
// IDSIMCKT* ckt - указатель на DSIM цепь, которая включает в себя модель.

VOID FP::setup(IINSTANCE* instance, IDSIMCKT* dsimckt) {
	_inst = instance;
	_ckt = dsimckt;
	_ckt->systime(&_time);
	ABSTIME time = _time;
	_log = new Logger();
	_uart = new Uart();
	_fiscal = new Fiscal(&_in_buffer, &_out_buffer, _log);

	bool required = true;
	bool not_required = false;
	_pin_rxd = _inst->getdsimpin(const_cast<CHAR*>("RXD"), required);
	_pin_txd = _inst->getdsimpin(const_cast<CHAR*>("TXD"), required);
	_pin_d_plus = _inst->getdsimpin(const_cast<CHAR*>("D+"), not_required);
	_pin_d_minus = _inst->getdsimpin(const_cast<CHAR*>("D-"), not_required);
	_pin_vbus = _inst->getdsimpin(const_cast<CHAR*>("VBUS"), not_required);
	_pin_select = _inst->getdsimpin(const_cast<CHAR*>("SELECT"), required);
	_pin_scl = _inst->getdsimpin(const_cast<CHAR*>("SCL"), not_required);
	_pin_sda = _inst->getdsimpin(const_cast<CHAR*>("SDA"), not_required);

	_pin_txd->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_txd->setstate(SHI);
	
	// Вообще нам нафиг не надо, сделано для красоты, типо полное ФН
	_pin_scl->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_scl->setstate(SHI);
	_pin_sda->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_sda->setstate(SHI);
	_pin_vbus->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_vbus->setstate(SHI);
	_pin_d_minus->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_d_minus->setstate(SHI);
	_pin_d_plus->sethandler(NULL, (PINHANDLERFN)NULL); // отключаем реагирование на контакт
	_pin_d_plus->setstate(SHI);

	auto baud_rate = atoi(_inst->getstrval(const_cast <CHAR*>("BAUD_RATE")));
	if (baud_rate == 0) {
		baud_rate = UART_BAUDRATE;
	}

	auto period = ONE_SECOND / baud_rate;

	_ckt->setclockcallback(_time + period, period, this, (CALLBACKHANDLERFN)&FP::callback_rxd, RXD_EVENT);
	_ckt->setclockcallback(_time + period, period, this, (CALLBACKHANDLERFN)&FP::callback_txd, TXD_EVENT);
}

VOID FP::runctrl(RUNMODES mode) {
	// not implemented
}

VOID FP::actuate(REALTIME time, ACTIVESTATE newstate) {
	// not implemented
}

BOOL FP::indicate(REALTIME time, ACTIVEDATA* data) {
	return FALSE;
}

VOID FP::simulate(ABSTIME time, DSIMMODES mode) {
	// DSIMBOOT - самый первый такт при запуске
	// DSIMSETTLE - первоначальная настройка цепей
	// DSIMNORMAL - основной режим симуляции
	// DSIMEND - остановка симуляции
	// DSIMMIXED - каждый шаг аналоговой симуляции
	if (mode != DSIMNORMAL) {
		return;
	}

	if (_pin_select->activity() == -1) {
		_inst->fatal(const_cast<CHAR*>("I2C NOT IMPLEMENTED!"));
		return;
	}

	_time = time;
}

VOID FP::callback_rxd(ABSTIME time, EVENTID eventid) {
	_time = time;
	switch (eventid) {
	case RXD_EVENT:
		_uart->rxBit(ishigh(_pin_rxd->istate())); // обрабатываем сигналы со схемы
		if (_uart->rxCompleted) {
			//if (_uart->rxData != 0x00) {
			//	_log->log(string_format("RECIVE: %c = ", _uart->rxData) + byte_to_hex(_uart->rxData));
			//} else {
			//	_log->log(string_format("RECIVE: %i = ", 0) + byte_to_hex(0));
			//}
			_in_buffer.push(_uart->rxData);
			if (_fiscal) {
				_fiscal->process_byte();
			} else {
				_log->log("WARNING _fiscal IS NULL");
			}
			_uart->rxCompleted = false; // сбрасываем флаг окончания чтения
		}
		break;
	default:
		_log->log(string_format("UNKNOWN EVENT ID: %i", eventid));
		break;
	}
	if (_log->msg_len()) {
		_inst->log(const_cast<CHAR*>(_log->getLog().c_str()));
	}
}

VOID FP::callback_txd(ABSTIME time, EVENTID eventid) {
	_time = time;
	switch (eventid) {
	case TXD_EVENT:
		if ((_uart->txCompleted) && (!_out_buffer.empty())) {
			_uart->txData = _out_buffer.front(); //отправляем в uart
			_out_buffer.pop();
			_uart->txCompleted = false; // сбрасываем флаг окончания записи
		}
		_pin_txd->setstate(time, 1, _uart->txBit() ? SHI : SLO); // устанавливает значение выхода модели
		break;
	default:
		_log->log(string_format("UNKNOWN EVENT ID: %i", eventid));
		break;
	}
	if (_log->msg_len()) {
		_inst->log(const_cast<CHAR*>(_log->getLog().c_str()));
	}
}

VOID FP::callback(ABSTIME time, EVENTID eventid) {
	// unimplemented
}

FP::~FP() {
	if (_log != nullptr) {
		delete _log;
	}
	if (_uart != nullptr) {
		delete _uart;
	}
	if (_fiscal != nullptr) {
		delete _fiscal;
	}
}