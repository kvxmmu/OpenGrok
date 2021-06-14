# FreeGrok Протокол

Здесь будет описан протокол, с помощью которого FreeGrok общается с клиентами. Раньше
много чего не поддерживалось, но с версией протокола 1.1a было внесено несколько изменений
в его структуру.

# Пакет
Все значения беззнаковые

| Имя поля      | Размер           | Описание |
| ------------- |:----------------:|---------:|
| type          | 8bit             | Тип пакета + флаги    |
| length        | 8-32bit          |   Длина полезной нагрузки, зависит от флагов    |
| payload       | ?                |    Полезная нагрузка, зависит от length    |

Какой формат у type?
| Номер бита | Описание |
|------------|----------|
| 0          | Тип      |
| 1          | Тип      |
| 2          | Тип      |
| 3          | Тип      |
| 4          | Тип      |
| 5          | Тип      |
| 6          | флаг сжатия  |
| 7          | флаг размера |

Флаг сжатия - определяет сжата ли полезная нагрузка, для сжатия используется алгоритм ZStandard

Флаг размера - определяет размер поля length, если флаг есть, то поле length имеет размер 8bit, иначе 32bit

Примерная маска: флаг сжатия - 0x1, флаг размера - 0x2.

# Полезная нагрузка

## PING
Номер: 0

Других полей не требуется, возвращает строку с именем сервера, размер строки равен размеру полезной нагрузки

## ERROR
Номер: 1

|   Имя поля  | Размер |            Описание           |
|-------------|--------|-------------------------------|
| Код ошибки  | 8bit   | Код ошибки, которая произошла |

Возможные ошибки:
```c++
#define NO_SUCH_COMMAND 0 // Прислана несуществующая команда
#define NO_SESSION 1 // сервер еще не создан
#define TOO_SHORT_BUFFER 2 // слишком маленький буфер для команды
#define TOO_LONG_BUFFER 3 // слишком большой буфер для команды
#define SERVER_ALREADY_CREATED 4 // сервер уже создан
#define BUFFER_SIZE_INCORRECT 5 // некорректный размер буфера(полезной нагрузки)
#define NO_SUCH_CLIENT 6 // такого клиента нет

#define INTERNAL_SERVER_ERROR 255 // внутреняя ошибка сервера, что-то произошло на стороне сервера
#define NOT_IMPLEMENTED 128 // функция не реализована
```

## GROK_CREATE_SERVER
Номер: 2

Не принимает никаких параметров

Возвращает порт, который был открыт на стороне сервера, на который проксируемый клиент может подключится

## GROK_PACKET
Номер: 3

|   Имя поля  | Размер |            Описание                                 |
|-------------|--------|-----------------------------------------------------|
| client_id   | 32bit  | Идентификатор подключенного клиента                 |
| payload     | ?      | Полезная нагрузка, которая будет переслана клиенту  |

**client_id** выдается другим событием, об этом ниже

## GROK_CONNECTED
Номер: 4

Отсылается только сервером

|   Имя поля      | Размер  |            Описание                |
|-----------------|---------|------------------------------------|
| Идентификатор   | 32bit   | Идентификатор, присвоенный клиенту |

## GROK_DISCONNECT
Номер: 5

|   Имя поля      | Размер  |            Описание                            |
|-----------------|---------|------------------------------------------------|
| Идентификатор   | 32bit   | Идентификатор клиента, которого надо отключить |

## GROK_STAT

Не реализовано на данный момент

