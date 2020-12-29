// Leonid Moguchev (c) 2020
#include "FSParser.h"
#include "utils.h"

bool FSParser::_reading_command = false;
uint16_t FSParser::_msg_len = 0;
size_t FSParser::_sum_bytes = 0;
size_t FSParser::_read_bytes = 0;

bool FSParser::process_byte(uint8_t b) {
    if (b == MSG_START && !_reading_command) {
        //std::cout << "MSG_START" << std::endl;
        _reading_command = true;
        _msg_len = 0x0000;
        _sum_bytes = 0;
        _read_bytes = 1;
        return true; // продолжить чтение
    }

    if (_reading_command) {
        switch (_read_bytes) {
        case 1: // младший байт LEN
            _msg_len = (uint16_t)b & 0x00FF;
            break;
        case 2: // старший байт LEN
            _msg_len |= (((uint16_t)b & 0x00FF) << 8);
            //std::cout << utils::string_format("LEN=%i", _msg_len) << std::endl;
            break;
        case 3: // код ответа ANSWER
            _msg_len--;
            //std::cout << utils::string_format("ANSWER=%i", b) << std::endl;
            break;
        default:
            //std::cout << utils::string_format("PROCESS=%i", _msg_len) << std::endl;
            if (_msg_len > 0) { // DATA
                _msg_len--;
            }
            else { // контрольная сумма
                if (_sum_bytes == 0) { // младший байт CRC
                    //std::cout << "L CRC" << std::endl;
                    _sum_bytes++;
                }
                else if (_sum_bytes == 1) { // старший байт CRC
                    //std::cout << "H CRC" << std::endl;
                    _sum_bytes++;
                    _reading_command = false;

                    return false; // прекратить чтение
                }
                else {
                    std::cerr << "WTF AFTER CONTROL SUM?" << std::endl;
                    return false; // прекратить чтение
                }
            }
            break;
        }
        _read_bytes++;
        return true; // продолжить чтение
    }

    std::cerr << "UNKNOWN STATE" << std::endl;
    return false; // прекратить чтение
}