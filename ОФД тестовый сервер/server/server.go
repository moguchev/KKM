package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
)

const (
	REGISTRATION        = 1
	OPEN_SHIFT          = 2
	CHECK               = 3
	CLOSE_SHIFT         = 5
	CLOSE_FISCAL_REGIME = 6
)

var (
	NalogCode = map[byte]string{
		1:  "Общая",
		2:  "Упрощенная Доход",
		4:  "Упрощенная Доход минус Расход",
		8:  "Единый налог на вмененный доход",
		16: "Единый сельскохозяйственный налог",
		32: "Патентная система налогообложения",
	}

	WorkMode = map[byte]string{
		1:  "Шифрование",
		2:  "Автономный режим",
		4:  "Автоматический режим",
		8:  "Применение в сфере услуг",
		16: "Режим БСО",
		32: "Применение в Интернет-торговле",
	}

	OperationType = map[byte]string{
		1: "Приход",
		2: "Возврат прихода",
		3: "Расход",
		4: "Возврат расхода",
	}
)

func handle(conn net.Conn) error {
	defer func() {
		log.Printf("closing connection from %v", conn.RemoteAddr())
		conn.Close()
	}()
	r := bufio.NewReader(conn)
	w := bufio.NewWriter(conn)
	scanr := bufio.NewScanner(r)
	for {
		if !scanr.Scan() {
			if err := scanr.Err(); err != nil {
				log.Printf("%v(%v)", err, conn.RemoteAddr())
				return err
			}
			break
		}
		w.Write(process(scanr.Bytes()))
		w.Flush()
	}
	return nil
}

func process(r []byte) []byte {
	kktSize := int(r[0])
	kktNumber := r[1 : 1+kktSize]

	fnSize := int(r[1+kktSize])
	fnNumber := r[2+kktSize : 2+kktSize+fnSize]
	fmt.Println("----------------------------------------------")
	fmt.Printf("Получено сообщение от:\nККТ №: %s\nФН №: %s\n", string(kktNumber), string(fnNumber))
	fmt.Println("----------------------------------------------")
	fmt.Println("Полученные документы:")

	return processDocs(r[2+kktSize+fnSize:])
}

func processDocs(r []byte) []byte {
	nums := []byte{}

	for i := 0; i < len(r); {
		switch r[i] {
		case REGISTRATION:
			nums = append(nums, RegistrationDocument(r[i+1:i+1+47]))
			i = i + 1 + 47
			break
		case OPEN_SHIFT:
			nums = append(nums, OpenShiftDocument(r[i+1:i+1+15]))
			i = i + 1 + 15
		case CHECK:
			nums = append(nums, CheckDocument(r[i+1:i+1+19]))
			i = i + 1 + 19
		case CLOSE_SHIFT:
			nums = append(nums, CloseShiftDocument(r[i+1:i+1+15]))
			i = i + 1 + 15
		case CLOSE_FISCAL_REGIME:
		}
	}
	return nums
}

func date_time_from_bytes(bytes []byte) string {
	if len(bytes) != 5 {
		return ""
	}
	min := fmt.Sprintf("%d", bytes[4])
	if bytes[4] < 10 {
		min = "0" + min
	}
	hour := fmt.Sprintf("%d", bytes[3])
	if bytes[3] < 10 {
		hour = "0" + hour
	}
	day := fmt.Sprintf("%d", bytes[2])
	if bytes[2] < 10 {
		day = "0" + day
	}
	month := fmt.Sprintf("%d", bytes[1])
	if bytes[1] < 10 {
		month = "0" + month
	}
	year := fmt.Sprintf("%d", bytes[0])
	if bytes[0] < 10 {
		year = "0" + year
	}

	return fmt.Sprintf("%s.%s.20%s %s:%s", day, month, year, hour, min)
}

func union_bytes(b0, b1, b2, b3 uint8) uint32 {
	return (uint32(b0)) | (uint32(b1) << 8) | (uint32(b2) << 16) | (uint32(b3) << 24)
}

func union_bytes_64(b0, b1, b2, b3, b4, b5, b6, b7 uint8) uint64 {
	return (uint64(b0)) | (uint64(b1) << 8) | (uint64(b2) << 16) | (uint64(b3) << 24) | (uint64(b4) << 32) | (uint64(b5) << 40) | (uint64(b6) << 48) | (uint64(b7) << 56)
}

func RegistrationDocument(data []byte) byte {
	fmt.Println("----------------------------------------------")
	fmt.Println("############Документ: Регистрация ККТ#########")

	DateTime := date_time_from_bytes(data[0:5])
	fmt.Printf("Дата и время: %s\n", DateTime)

	Number := union_bytes(data[5], data[6], data[7], data[8])
	fmt.Printf("Номер ФД: %d\n", Number)

	FiscSign := union_bytes(data[9], data[10], data[11], data[12])
	fmt.Printf("Фискальный признак: %d\n", FiscSign)

	Inn := string(data[13:25])
	fmt.Printf("ИНН: %s\n", Inn)

	KTTNumber := string(data[25:45])
	fmt.Printf("Регистрационный номер ККТ: %s\n", KTTNumber)

	NCode := NalogCode[data[45]]
	fmt.Printf("Код налогообложения: %s\n", NCode)

	WMode := WorkMode[data[46]]
	fmt.Printf("Режим работы: %s\n", WMode)

	return byte(Number)
}

func OpenShiftDocument(data []byte) byte {
	fmt.Println("----------------------------------------------")
	fmt.Println("############Документ: Открытие смены##########")

	DateTime := date_time_from_bytes(data[0:5])
	fmt.Printf("Дата и время: %s\n", DateTime)

	Number := union_bytes(data[5], data[6], data[7], data[8])
	fmt.Printf("Номер ФД: %d\n", Number)

	FiscSign := union_bytes(data[9], data[10], data[11], data[12])
	fmt.Printf("Фискальный признак: %d\n", FiscSign)

	ShiftNum := union_bytes(data[13], data[14], 0, 0)
	fmt.Printf("Номер смены: %d\n", ShiftNum)

	return byte(Number)
}

func CloseShiftDocument(data []byte) byte {
	fmt.Println("----------------------------------------------")
	fmt.Println("############Документ: Закрытие смены##########")

	DateTime := date_time_from_bytes(data[0:5])
	fmt.Printf("Дата и время: %s\n", DateTime)

	Number := union_bytes(data[5], data[6], data[7], data[8])
	fmt.Printf("Номер ФД: %d\n", Number)

	FiscSign := union_bytes(data[9], data[10], data[11], data[12])
	fmt.Printf("Фискальный признак: %d\n", FiscSign)

	ShiftNum := union_bytes(data[13], data[14], 0, 0)
	fmt.Printf("Номер смены: %d\n", ShiftNum)

	return byte(Number)
}

func CheckDocument(data []byte) byte {
	fmt.Println("----------------------------------------------")
	fmt.Println("############Документ: Кассовый чек############")

	DateTime := date_time_from_bytes(data[0:5])
	fmt.Printf("Дата и время: %s\n", DateTime)

	Number := union_bytes(data[5], data[6], data[7], data[8])
	fmt.Printf("Номер ФД: %d\n", Number)

	FiscSign := union_bytes(data[9], data[10], data[11], data[12])
	fmt.Printf("Фискальный признак: %d\n", FiscSign)

	OpType := OperationType[data[13]]
	fmt.Printf("Тип операции: %s\n", OpType)

	Sum := union_bytes_64(data[14], data[15], data[16], data[17], data[18], 0, 0, 0)
	fmt.Printf("Сумма операции: %f\n", float64(Sum)/100)

	return byte(Number)
}

func main() {
	fmt.Println("Запущен ОФД сервер...")
	// Устанавливаем прослушивание порта
	listener, err := net.Listen("tcp", ":4321")
	if err != nil {
		log.Fatalf("listen error: %v", err.Error())
	}
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Fatalf("error accepting connection: %v", err.Error())
			continue
		}
		log.Printf("accepted connection from %v", conn.RemoteAddr())
		handle(conn)
	}
}
