#include <iostream>
#include <handleapi.h>
#include <synchapi.h>
#include <windows.h>



void showMenu()
{
    std::cout << "--- Menu ---" << std::endl;
    std::cout << "0 - Exit" << std::endl;
    std::cout << "1 - Create pipe" << std::endl;
    std::cout << "2 - Connect pipe" << std::endl;
    std::cout << "3 - Write data" << std::endl;
}

int main() {
    // Размер передаваемого сообщения
    const int bufferSize = 100;
    HANDLE pipe = nullptr;
    HANDLE event;
    event = CreateEvent(nullptr, true, 0, nullptr);
    OVERLAPPED overlappedStruct;

    char item;
    do {
        showMenu();
        std::cin >> item;

        switch (item) {
            case '1':
                /*
                 * Создаём именованный канал
                 * . - обозначает локальный компьютер
                 * PIPE_ACCESS_OUTBOUND - поток данных идет только с сервера на клиент
                 * PIPE_TYPE_MESSAGE - данные записываться в канал как сообщения (а не как поток байтов)
                 * 1 - количество клиентов
                 * bufferSize - размеры входного и выходного буферов
                 * 0 - таймаут. Актуально при использовании WaitNamedPipe, который не используется здесь
                 * nullptr - SECURITY_ATTRIBUTES, default security descriptor
                 * */
                pipe = CreateNamedPipe(
                        "\\\\.\\pipe\\ServerPipe",
                        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                        PIPE_TYPE_MESSAGE,
                        1,
                        bufferSize,
                        bufferSize,
                        0,
                        nullptr);
                if (pipe == INVALID_HANDLE_VALUE)
                    std::cout << "Error while executing CreateNamedPipe() (error " << GetLastError() << ")" << std::endl;
                else
                    std::cout << "Pipe successfully created" << std::endl;
                break;
            case '2':
                std::cout << "Waiting for the client to connect" << std::endl;
                // Ожидаем подключение единственного возможного клиента
                if (ConnectNamedPipe(pipe, nullptr))
                    std::cout << "The client has successfully connected to the pipe" << std::endl;
                else
                    std::cout << "Error while executing ConnectNamedPipe() (error " << GetLastError() << ")" << std::endl;
                break;
            case '3': {
                char buffer[bufferSize];
                std::cout << "Enter data to be send" << std::endl;
                std::cin >> buffer;
                overlappedStruct.hEvent = event;
                overlappedStruct.Offset = 0;
                overlappedStruct.OffsetHigh = 0;
                DWORD bytesWritten;

                // Записываем сообщение
                WriteFile(pipe, buffer, strlen(buffer) + 1, &bytesWritten, &overlappedStruct);

                // Проверяем скольско байт было записано
                if (!bytesWritten)
                    std::cout << "Error " << GetLastError() << std::endl;
                else {
                    std::cout << bytesWritten << " bytes have been written\n";
                    std::cout << "Sending a buffer"  << std::endl;
                    WaitForSingleObject(event, INFINITE);
                    std::cout << "Message sent" << std::endl;
                }
            }
                break;
            case '0':
                break;
            default:
                std::cout << "Wrong menu item selected. Try again" << std::endl;
        }
    } while (item != '0');

    // Отключаемся от именованного канала
    DisconnectNamedPipe(pipe);

    // Закрываем HANDLE'ы
    CloseHandle(pipe);
    CloseHandle(event);

    return 0;

}