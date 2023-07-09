//Основной блок. Инициализирует переменную классом реализации
//основных команд и функций. Запускает ChatBot'а. Состоит из
//двух циклов. Внутренний работает до получения значения false
//при выходы из меню чата. Внешний до получения false при выходе 
//из меню входа.
#include <stdlib.h>
#include "command_module.h"
#include <iostream>

auto main() -> int
{
	system("chcp 1251");
	system("cls");
	srand((unsigned int) time(NULL));
	CommandModule unworkableChat;
	unworkableChat.ConnectionToSQL();
	if (unworkableChat.worksSQL_)
	{
		unworkableChat.InitChatBot();
		std::cout << "Добро пожаловать в Неработающий Чат!" << std::endl;
		while (unworkableChat.worksChat_)
		{
			unworkableChat.LoginMenu();
			while (unworkableChat.worksUser_)
			{
				unworkableChat.PrintHistory();
				unworkableChat.ChatMenu();
			}
		}
	}
	unworkableChat.DisconnectionFromSQL();
	return 0;
}