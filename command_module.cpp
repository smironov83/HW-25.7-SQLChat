//Установлено ограничение на количество символов (100)
//в словах при сохранении в словаре автодополнения. Реализовано сохранение 
//учетных данных пользователей в единой БД MySQL. Вывод 
//текущей переписки ограничен 20 строками. Остальная история записывается в 
//БД. Реализован запрет на вход под учетными данными пользователя ChatBot.
#include "command_module.h"
#include <iostream>
#include <conio.h>

#define MSG_MYSQL_ERR std::cout << "Ошибка MySql: " << mysql_error(&mysql_) \
<< std::endl

//Меню входа. Обрабатывает ввод номера команды, ввод посторонних символов не
//принимается. Предлагает войти в чат, зарегистрироваться в чате, выйти из 
//программы. Возвращает значение true для main - программа запущена.
//При вводе команды выхода - возвращает false и цикл в main завершается
//с последующим завершением работы программы.
auto CommandModule::LoginMenu()->bool
{
	std::cout << std::endl;
	auto command = 0;
	std::cout << "Для продолжения работы введите номер команды:" << std::endl;
	std::cout << "1 - войти в чат" << std::endl;
	std::cout << "2 - зарегистрироваться в чате:" << std::endl;
	std::cout << "3 - выйти из программы" << std::endl;
	while (!(std::cin >> command) || (std::cin.peek() != '\n') || !(command == 1
		|| command == 2 || command == 3))
	{
		std::cin.clear();
		while (std::cin.get() != '\n');
		std::cout << "Вы ввели неверный номер команды. Пожалуйста, повторите ввод: "
			<< std::endl;
	}
	switch (command)
	{
	case 1:
		users_[currentUser_].online_ = LogIn();
		worksUser_ = users_[currentUser_].online_;
		break;
	case 2:
		Registration();
		break;
	case 3:
		std::cout << std::endl <<
			"Очень жаль, что Вы покидаете Неработающий Чат! Ждём Вашего возвращения!"
			<< std::endl;
		worksChat_ = false;
		autoDict_->Remove(autoDict_);
		delete autoDict_;
		break;
	}
	return worksChat_;
}
//Регистрирует ChatBot'а. Авторизует его в чате со статусом online.
//Инициализирует словарный запас бота. Проверяет наличие базы данных
//пользователей и считывает её, в случае отсутствия - создает новую и 
//записывает данные бота. 
void CommandModule::InitChatBot()
{
	mysql_query(&mysql_, "SHOW TABLES");
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	bool check = false;
	std::string table;
	if (res_ = mysql_store_result(&mysql_))
		while (row_ = mysql_fetch_row(res_))
		{
			if (row_[0] != NULL)
				table = row_[0];
			if (table == "users")
				check = true;
		}
	else
		MSG_MYSQL_ERR;
	if (!check)
	{
		mysql_query(&mysql_, "CREATE TABLE users (id INT NOT NULL AUTO_INCREMENT "
			"PRIMARY KEY, login VARCHAR(100), password VARCHAR(1000), name "
			"VARCHAR(100), online BOOL, history INT, dictionary INT)");
		if (mysql_errno(&mysql_))
			MSG_MYSQL_ERR;
		user_ = new User<std::string, std::vector<std::string>> {"ChatBot",
			user_->HashPassword("ChatBot", "qwerty"), "Бот", true};
		std::string stream = "INSERT INTO users (login, password, name, online) values('"
			+ user_->login_ + "', '" + user_->get_password() + "', '" + user_->name_ +
			"', true)";
		mysql_query(&mysql_, stream.c_str());
		if (mysql_errno(&mysql_))
			MSG_MYSQL_ERR;
		users_.push_back(*user_);
	}
	else
	{
		user_ = new User<std::string, std::vector<std::string>> {};
		mysql_query(&mysql_, "SELECT * FROM users");
		if (mysql_errno(&mysql_))
			MSG_MYSQL_ERR;
		if (res_ = mysql_store_result(&mysql_))
			while (row_ = mysql_fetch_row(res_))
			{
				if (row_[1] != NULL)
					user_->login_ = row_[1];
				if (row_[2] != NULL)
					user_->set_password(row_[2]);
				if (row_[3] != NULL)
					user_->name_ = row_[3];
				if (row_[4] != NULL)
				{
					std::string boolCheck = row_[4];
					if (boolCheck == "0")
						user_->online_ = false;
					else
						user_->online_ = true;
				}
				users_.push_back (*user_);
			}
		else
			MSG_MYSQL_ERR;
		users_[0].online_ = true;
	}
	chatBotAnswers_ = { "Здесь кто-то есть?",
		"Совершенно верно! Мои мысли ушли в том же направлении!",
		"В этом и заключался мой коварный замысел!",
		"А в это время в Африке дети голодают!",
		"А у меня есть котик! Неправда ли чудесное животное?",
		"Ну и кто Вы после этого?",
		"А я - Великий и Ужасный ChatBot этого Неработающего чата!",
		"Я Вас не понимаю!",
		"Я не говорю по-русски!" };
	delete user_;
}
//Регистрирует нового пользователя. Ввод данных: логин, пароль и имя.
//Проверяет логин на уникальность по всем зарегистрированным пользователям.
//Слово "exit" исключено из возможных значений логина, т.к. зарезервировано
//для выхода из меню входа в чат. Сохраняет данные пользователя в БД.
void CommandModule::Registration()
{
	std::cout << std::endl;
	user_ = new User<std::string, std::vector<std::string>>{};
	auto unique = true;
	std::cout << "Введите логин: ";
	do
	{
		std::cin >> user_->login_;
		unique = true;
		for (auto& user : users_)
		{
			if ((user_->login_ == user.login_) || (user_->login_ == "exit"))
			{
				unique = false;
				std::cout << "Логин занят. Введите другой логин: ";
			}
		}
	} while (!(unique));
	std::cout << "Введите пароль: ";
	std::string passEnter;
	std::cin >> passEnter;
	passEnter = user_->HashPassword(user_->login_, passEnter);
	user_->set_password(passEnter);
	std::cout << "Введите Ваше имя: ";
	std::cin >> user_->name_;
	user_->online_ = false;
	users_.push_back(*user_);
	std::cout << "Регистрация успешно завершена. Для продолжения работы войдите "
		"в Неработающий Чат под Вашими учетными данными!" << std::endl;
	std::string stream = "INSERT INTO users (login, password, name, online) values('"
		+ user_->login_ + "', '" + user_->get_password() + "', '" + user_->name_ +
		"', false)";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	stream = "CREATE TABLE history" + user_->login_ + " (id INT NOT NULL "
		"AUTO_INCREMENT PRIMARY KEY, messages VARCHAR(1000))";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	stream = "ALTER TABLE users ADD FOREIGN KEY (history) REFERENCES history" + 
		user_->login_ + " (id)";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	stream = "CREATE TABLE dictionary" + user_->login_ + " (id INT NOT NULL "
		"AUTO_INCREMENT PRIMARY KEY, words VARCHAR(100), UNIQUE(words))";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	stream = "ALTER TABLE users ADD FOREIGN KEY (dictionary) REFERENCES "
		"dictionary" + user_->login_ + " (id)";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	delete user_;
}
//Авторизирует пользователя по связке логин/пароль, сверяет по массиву
//зарегистрированных пользователей. Предусмотрен выход в предыдущее меню
//по ключевому слову "exit" на случай, если пользователь забыл учетные данные.
//Возвращает значение "true" для сетевого статуса пользователя и статуса 
//входа авторизованного пользователя в чат для main. Фиксирует значение индекса
//элемента массива, в котором хранятся данные пользователя, выполнившего вход в
//чат. Загружает последние 20 сообщений из БД истории переписки текущего 
//пользователя. После авторизации предлагает включить режим автодополнения 
//В случае включения подключает индивидуальный словарь автодополнения из БД.
auto CommandModule::LogIn() -> bool
{
	std::cout << std::endl;
	user_ = new User<std::string, std::vector<std::string>>{};
	std::cout << "Для входа в Неработающий Чат введите Ваши учетные данные!"
		<< std::endl;
	auto unique = true;
	auto online = true;
	do
	{
		do
		{
			auto counter = 0;
			std::cout << "Введите логин: ";
			std::cin >> user_->login_;
			for (auto& user : users_)
			{
				if (user_->login_ == user.login_ && counter != 0)
				{
					unique = false;
					currentUser_ = counter;
				}
				if (unique && (counter == users_.size() - 1) && (user_->login_ != 
					"exit"))
					std::cout << "Логин не зарегистрирован. Повторите ввод или введите "
						"exit для возврата в предыдущее меню." << std::endl;
				if (user_->login_ == "exit")
					unique = online = false;
				counter++;
			}
		} while (unique);
		if (user_->login_ != "exit")
		{
			std::cout << "Введите пароль: ";
			std::string passEnter;
			std::cin >> passEnter;
			if (user_->CheckPassword(passEnter) != users_[currentUser_].
				get_password())
			{
				std::cout << "Пароль неверный. Повторите ввод логина и пароля или "
					"введите exit для возврата в предыдущее меню." << std::endl;
				unique = true;
			}
		}
	} while (unique);
	delete user_;
	if (!online)
		return online;
	std::string stream = "SELECT * FROM history" + users_[currentUser_].login_;
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	if (res_ = mysql_store_result(&mysql_))
		while (row_ = mysql_fetch_row(res_))
			if (row_[1] != NULL)
				AddHistory(row_[1], currentUser_);
	else
		MSG_MYSQL_ERR;
	stream = "UPDATE users SET online = true where id = " + 
		std::to_string(currentUser_ + 1);
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	std::cout << std::endl;
	auto command = 0;
	std::cout << "Для включения режима автодополнения введите 1, иначе - 0" << 
		std::endl;
	while (!(std::cin >> command) || (std::cin.peek() != '\n') || !(command == 0
		|| command == 1))
	{
		std::cin.clear();
		while (std::cin.get() != '\n');
		std::cout << 
			"Вы ввели неверный номер команды. Пожалуйста, повторите ввод: "	
			<< std::endl;
	}
	if (command == 1)
	{
		autocompleteEnable_ = true;
		InitAutoDict();
		system("cls");
		std::cout << "Здравствуйте!\n\nВ данной программе реализован поиск в слова"
			"ре вариантов автодополнения по введенному префиксу.\nПрограмма работает"
			" только с символами кириллицы любого регистра.\nБазовый словарь отсутст"
			"вует. Все введенные слова на кириллице добавляются в словарь и сохраняю"
			"тся в файле на диске.\nПереключение между доступными вариантами автодоп"
			"олнения по префиксу осуществляется стрелками влево и вправо.\nНаличие т"
			"акой возможности подсвечивается <- и ->, соответственно.\nДля выбора те"
			"кущего варианта автодополнения нажмите стрелку вверх.\n\nP.S.Не рекомен"
			"дуется перемещение указателя в сторону начала строки от текущего положе"
			"ния.\nНапример, Backspace. Возможно неопредленное поведение программы."
			"\nНо это не точно!\n\n";
		system("pause");
	}
	else
		autocompleteEnable_ = false;
	return online;
}
//Меню чата. Обрабатывает ввод номера команды, ввод посторонних символов не
//принимается. Предлагает написать сообщение всем пользователям, выбранному 
//пользователю (личное сообщение), просмотреть профиль выбранного 
//пользователя, либо выйти из аккаунта. Возвращает значение true для main -
//пользователь авторизован, работает вложенный цикл.
//При вводе команды выхода - возвращает false и вложенный цикл в main 
//завершается с последующим возвратом в меню входа.
//Реализована функция вывода всей историию переписки для текущего 
//пользователя из БД.
auto CommandModule::ChatMenu() -> bool
{
	auto command = 0;
	std::cout << "Выберите действие:" << std::endl;
	std::cout << "1 - написать сообщение всем" << std::endl;
	std::cout << "2 - написать выбранному пользователю:" << std::endl;
	std::cout << "3 - просмотреть профиль выбранного пользователя" << std::endl;
	std::cout << "4 - показать всю историю переписки" << std::endl;
	std::cout << "5 - выйти из чата" << std::endl;
	while (!(std::cin >> command) || (std::cin.peek() != '\n') || !(command == 1
		|| command == 2 || command == 3 || command == 4 || command == 5))
	{
		std::cin.clear();
		while (std::cin.get() != '\n');
		std::cout << 
			"Вы ввели неверный номер комманды. Пожалуйста, повторите ввод: "
			<< std::endl;
	}
	switch (command)
	{
	case 1:
		system("cls");
		MessageToAll();
		break;
	case 2:
		system("cls");
		MessageToUser();
		break;
	case 3:
		system("cls");
		UserInfo();
		break;
	case 4:
		system("cls");
		PrintAllHistory();
		break;
	case 5:
		system("cls");
		std::cout << "Вы вышли из чата!" << std::endl;
		worksUser_ = users_[currentUser_].online_ = false;
		std::string stream = "UPDATE users SET online = false where id = " +
			std::to_string(currentUser_ + 1);
		mysql_query(&mysql_, stream.c_str());
		if (mysql_errno(&mysql_))
			MSG_MYSQL_ERR;
		break;
	}
	return worksUser_;
}
//Команда отправляет сообщение всем. Сообщение сохраняется у всех 
//зарегистрированных пользователей. Сообщение видит ChatBot и реагирует на 
//него.
void CommandModule::MessageToAll()
{
	PrintHistory();
	std::string text{ "[" + users_[currentUser_].login_ + "]->[All] : " };
	std::cout << text;
	std::cin.ignore();
	if (autocompleteEnable_)
	{
		text = characterInput(text);
		text.erase(0, users_[currentUser_].login_.size() + 12);
		autoDict_->stream_.clear();
	}
	else std::getline(std::cin, text);
	message_ = new Message{ message_->TimeStamp(), users_[currentUser_].login_,
		"All", text };
	text = message_->MessageConstructor();
	for (auto i = 1; i < users_.size(); ++i)
	{
		SaveHistory(text, i);
		if (users_[i].online_)
			AddHistory(text, i);
	}
	delete message_;
	text = AnswerChatBot();
	for (auto i = 1; i < users_.size(); ++i)
	{
		SaveHistory(text, i);
		if (users_[i].online_)
			AddHistory(text, i);
	}
}
//Выводит 20 последних сообщений переписки для текущего пользователя.
void CommandModule::PrintHistory()
{
	system("cls");
	for (auto& user : users_[currentUser_].history_)
		std::cout << user << std::endl;
	if (autocompleteEnable_)
		std::cout << autoDict_->stream_;
}
//Выводит из БД на экран всю историю переписки текущего пользователя 
//по 20 строк. 
void CommandModule::PrintAllHistory()
{
	auto counter = 0;
	std::string stream = "SELECT * FROM history"
		+ users_[currentUser_].login_;
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	if (res_ = mysql_store_result(&mysql_))
		while (row_ = mysql_fetch_row(res_))
			if (row_[1] != NULL)
			{
				++counter;
				std::cout << row_[1] << std::endl;
				if (counter % 20 == 0)
					system("pause");
			}
			else
				MSG_MYSQL_ERR;
	system("pause");
}
//Сохраняет переписку указанного пользователя в БД. 
void CommandModule::SaveHistory(std::string const &history, size_t userIndex)
{
	std::string stream = "INSERT INTO history" + users_[userIndex].login_ + 
		"(messages) VALUES ('" + history + "')";
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
}
//Добавляет сообщения в массив истории указанного пользователя. Если сообщений 
//больше 20, то удаляет наиболее старое сообщение.
void CommandModule::AddHistory(std::string const &history, size_t userIndex)
{
	if (users_[userIndex].history_.size() == 20)
		users_[userIndex].history_[0].erase();
	users_[userIndex].history_.push_back(history);
}
//Отправляет сообщение выбранному пользователю. После выбора команды предлагает
//выбрать получателя, путем вывода списка всех зарегистрированных 
//пользователей, путем ввода номера пользователя. Реализована обработка ввода,
//исключающая обращение к несуществующему элементу массива. Сохраняет историю
//для текущего пользователя и для получателя. Остальные пользователи не видят
//этого сообщения. ChatBot видит сообщение и реагирует на него, только если
//является получателем. Для ChatBot'а история не сохраняется. Можно отправить 
//личное сообщение самому себе.
void CommandModule::MessageToUser()
{
	PrintHistory();
	PrintUsers();
	std::cout <<
		"Введите номер пользователя, которому Вы хотите отправить сообщение: ";
	auto command = 0;
	while (!(std::cin >> command) || (std::cin.peek() != '\n') || (command == 0)
		|| !(command < users_.size() + 1))
	{
		std::cin.clear();
		while (std::cin.get() != '\n');
		std::cout 
			<< "Вы ввели неверный номер комманды. Пожалуйста, повторите ввод: "
			<< std::endl;
	}
	--command;
	PrintHistory();
	std::string text{ "[" + users_[currentUser_].login_ + "]->" 
		+ users_[command].login_ + " : " };
	std::cout << text;
	std::cin.ignore();
	if (autocompleteEnable_)
	{
		text = characterInput(text);
		text.erase(0, users_[currentUser_].login_.size() + 7 
			+ users_[command].login_.size());
		autoDict_->stream_.clear();
	}
	else std::getline(std::cin, text);
	message_ = new Message{ message_->TimeStamp(), users_[currentUser_].login_,
		users_[command].login_, text };
	text = message_->MessageConstructor();
	delete message_;
	AddHistory(text, currentUser_);
	SaveHistory(text, currentUser_);
	if (command != 0)
	{
		SaveHistory(text, command);
		if (users_[command].online_)
			AddHistory(text, command);
	}
	else
	{
		text = AnswerChatBot();
		AddHistory(text, currentUser_);
		SaveHistory(text, currentUser_);
	}
}
//Выводит на экран всех зарегистрированных пользователей с присвоением 
//порядкового номера. Предварительно обновляет данные из БД.
void CommandModule::PrintUsers()
{
	mysql_query(&mysql_, "SELECT * FROM users");
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	auto counter = 0;
	if (res_ = mysql_store_result(&mysql_))
		while (row_ = mysql_fetch_row(res_))
		{
			if (counter < users_.size())
				if (row_[4] != NULL)
				{
					std::string boolCheck = row_[4];
					if (boolCheck == "0")
						user_->online_ = false;
					else
						user_->online_ = true;
				}
			else
			{
				user_ = new User<std::string, std::vector<std::string>> {};
				if (row_[1] != NULL)
					user_->login_ = row_[1];
				if (row_[2] != NULL)
					user_->set_password(row_[2]);
				if (row_[3] != NULL)
					user_->name_ = row_[3];
				if (row_[4] != NULL)
				{
					std::string boolCheck = row_[4];
					if (boolCheck == "0")
						user_->online_ = false;
					else
						user_->online_ = true;
				}
				users_.push_back(*user_);
				delete user_;
			}
			++counter;		
		}
	else
		MSG_MYSQL_ERR;
	counter = 0;
	for (auto& user : users_)
	{
		++counter;
		std::cout << counter << " - " << user.login_;
		std::cout << (user.online_ ? " (online) " : " (offline) ");
		std::cout << std::endl;
	}
}
//Предлагает просмотреть профиль выбранного пользователя путем ввода 
//порядкового пользователя из выведенного списка. Реализована обработка ввода,
//исключающая обращение к несуществующему элементу массива. Выводит логин, имя 
//и текущий сетевой статус пользователя.
void CommandModule::UserInfo()
{
	PrintUsers();
	std::cout <<
		"Введите номер пользователя, профиль которого Вы хотите посмотреть: ";
	auto command = 0;
	while (!(std::cin >> command) || (std::cin.peek() != '\n') || (command == 0)
		|| !(command < users_.size() + 1))
	{
		std::cin.clear();
		while (std::cin.get() != '\n');
		std::cout << 
			"Вы ввели неверный номер комманды. Пожалуйста, повторите ввод: "
			<< std::endl;
	}
	std::cout << "Логин: " << users_[--command].login_ << std::endl;
	std::cout << "Имя: " << users_[command].name_ << std::endl;
	std::cout << (users_[command].online_ == true ? "В сети: да" : "В сети: нет")
		<< std::endl;
	std::cout << std::endl;
	system("pause");
	system("cls");
}
//Реализует реакцию ChatBot'а на появление новых сообщений. ChatBot отвечает
//на любое новое сообщение в зоне его видимости личным сообщением отправителю.
//Текст сообщения - случайный, из списка фраз массива ответов ChatBot'а.
//Ответ ChatBot'а сохраняется у получателя и отправителя.
auto CommandModule::AnswerChatBot()->std::string
{
	auto answer = rand() % chatBotAnswers_.size();
	std::string text = chatBotAnswers_[answer];
	message_ = new Message{ message_->TimeStamp(), users_[0].login_,
		users_[currentUser_].login_, text };
	text = message_->MessageConstructor();
	delete message_;
	return text;
}
//Инициализирует словарь префиксного дерева из БД.
void CommandModule::InitAutoDict()
{
	std::string stream = "SELECT * FROM dictionary" 
		+ users_[currentUser_].login_;
	mysql_query(&mysql_, stream.c_str());
	if (mysql_errno(&mysql_))
		MSG_MYSQL_ERR;
	if (res_ = mysql_store_result(&mysql_))
		while (row_ = mysql_fetch_row(res_))
			if (row_[1] != NULL)
				autoDict_->Insert(autoDict_, row_[1]);
			else
				MSG_MYSQL_ERR;
}
//При включенном режиме автодополнения обрабатывает каждую нажатую клавишу из 
//потока ввода в режиме реального времени. Различает нажатие символьных клавиш
//от системных при совпадающих кодах. В БД сохраняет только уникальные слова.
//Ограничение длины слова - 100.
auto CommandModule::characterInput(std::string const &text)->std::string
{
	autoDict_->stream_ = text;
	do
	{
		auto twinCode = false;
		//Считывает код нажатой клавиши.
		symbol_ = _getch();
		//Проверяет на наличие расширенного кода нажатой клавиши, в случе true - 
		//считывает повторно и отмечает наличие расширенного кода.
		if (_kbhit())
		{
			symbol_ = _getch();
			twinCode = true;
		}
		//Заменяет Ё и ё на Е и е.
		if (symbol_ == -88)
			symbol_ = -59;
		else if (symbol_ == -72)
			symbol_ = -27;
		//Switch по коду нажатой клавиши.
		switch (symbol_)
		{
			//Пробел. Если первый символ заглавный - заменяет на строчный. Сохраняет
			//слово до пробела в дерево. Очищает текущий префикс. Добавляет пробел
			//в stream. Обновляет экран. 
		case 32:
			if (prefix_.size() > 1)
			{
				prefix_.erase(remove(prefix_.begin(), prefix_.end(), ' '), 
					prefix_.end());
				if (prefix_[0] < -32)
					prefix_[0] += 32;
				autoDict_->Insert(autoDict_, prefix_);
				std::string stream = "INSERT INTO dictionary" +
					users_[currentUser_].login_ + "(words) VALUES ('" + prefix_ + "')";
				mysql_query(&mysql_, stream.c_str());
			}
			prefix_ = {};
			autoDict_->stream_.push_back(' ');
			PrintHistory();
			break;
			//Enter. Если первый символ заглавный - заменяет на строчный. Сохраняет 
			//слово до Enter в дерево. Очищает текущий префикс. Обновляет экран.
		case 13:
			if (prefix_.size() > 1)
			{
				prefix_.erase(remove(prefix_.begin(), prefix_.end(), ' '),
					prefix_.end());
				if (prefix_[0] < -32)
					prefix_[0] += 32;
				std::string stream = "INSERT INTO dictionary" +
					users_[currentUser_].login_ + "(words) VALUES ('" + prefix_ + "')";
				mysql_query(&mysql_, stream.c_str());
				std::cout << std::endl;
				autoDict_->Insert(autoDict_, prefix_);
			}
			prefix_ = {};
			PrintHistory();
			break;
			//Остальные варианты со вложенным переключателем, реализовано для 
			//разделения кодов системных клавиш и символьных при совпадении.
		default:
			//Если код расширенный.
			if (twinCode)
				switch (symbol_)
				{
					//Стрелка вверх. Дополняет префикс до выбранного слова, если префикс
					//не пустой. Дописывает в stream. Очищает текущий префикс.
				case 72:
					if (prefix_ != "")
						autoDict_->stream_ += autoDict_->prefixes_[position_] + " ";
					prefix_ = {};
					PrintHistory();
					break;
					//Стрелка влево. Если есть предыдущий вариант дополнения префикса - 
					//выводит его.
				case 75:
					if (position_ > 0 && autoDict_->prefCount != 0)
					{
						PrintHistory();
						--position_;
						autoDict_->PrintAutocomplete(autoDict_, position_);
					}
					break;
					//Стрелка вправо. Если есть последующий вариант дополнения префикса -
					//выводит его.
				case 77:
					if (position_ + 1 < autoDict_->prefCount)
					{
						PrintHistory();
						++position_;
						autoDict_->PrintAutocomplete(autoDict_, position_);
					}
					break;
					//Прочие системные клавиши не отрабатываются.
				default:
					break;
				}
			//Все остальные варианты кода нажатой клавиши. Добавляет в stream.
			//Если символ на кириллице и длина слова не более 100 символов - добавляет
			//к текущему префиксу. Иначе - сохраняет текущий префикс в дереве и
			//очищает текущий префикс. Если текущий префикс не пустой и содержится в
			//дереве - запускает поиск всех слов про префиксу в дереве. По умолчанию 
			//выводит первый из найденных и помечает наличие/отстуствие последующих
			//вариантов.
			else
			{
				autoDict_->stream_.push_back(symbol_);
				position_ = 0;
				PrintHistory();
				if (symbol_ >= -64 && symbol_ <= -1 && prefix_.size() < 100)
					prefix_.push_back(symbol_);
				else
				{
					if (prefix_.size() > 1)
					{
						prefix_.erase(remove(prefix_.begin(), prefix_.end(), ' '),
							prefix_.end());
						if (prefix_[0] < -32)
							prefix_[0] += 32;
						autoDict_->Insert(autoDict_, prefix_);
						std::string stream = "INSERT INTO dictionary" +
							users_[currentUser_].login_ + "(words) VALUES ('" + prefix_ + "')";
						mysql_query(&mysql_, stream.c_str());
					}
					prefix_ = {};
				}
				if (autoDict_->Search(autoDict_, prefix_) && prefix_ != "")
				{
					autoDict_->FindAllPrefixes(autoDict_, wordConstructor_, 0, prefix_);
					autoDict_->PrintAutocomplete(autoDict_, position_);
				}
			}
		}
		//Enter. Выходит из циклического ввода.
	} while (symbol_ != 13);
	return autoDict_->stream_;
}

//Подключает к серверу и БД. Если БД не найдена, то создает БД и рекурсивно 
//повторяет подключение к нужной БД. Устанавливает кодировку CHCP 1251.
void CommandModule::ConnectionToSQL()
{
	//Получаем дескриптор соединения.
	mysql_init(&mysql_);
	if (&mysql_ == nullptr) 
	{
		//Если дескриптор не получен — выводит сообщение об ошибке.
		std::cout << "Ошибка: невозможно создать MySQL-descriptor" << std::endl;
		worksSQL_ = false;
	}
	//Подключается к серверу.
	if (!mysql_real_connect(&mysql_, DB_SQL_SERVER, DB_SQL_LOGIN,
		DB_SQL_PASSWORD, DB_SQL, 0, 0, 0) && worksSQL_)
	{
		std::cout << "Ошибка: невозможно подключиться к серверу БД " <<
			mysql_error(&mysql_) << std::endl;
		//Если нет возможности установить соединение с БД пытается ее создать.
		std::cout << "Попытка создать БД sqlchat..." << std::endl;
		mysql_init(&mysql_);
		mysql_real_connect(&mysql_, DB_SQL_SERVER, DB_SQL_LOGIN,
			DB_SQL_PASSWORD, 0, 0, 0, 0);
		if (mysql_query(&mysql_, "CREATE DATABASE sqlchat"))
		{
			std::cout << "Ошибка создания базы данных: " << mysql_error(&mysql_)
				<< std::endl;
			worksSQL_ = false;
		}
		else
		{
			std::cout << "БД sqlchat успешно создана!" << std::endl;
			ConnectionToSQL();
		}
	}
	else
		mysql_set_character_set(&mysql_, "cp1251");
}

//Закрывает соединение с сервером.
void CommandModule::DisconnectionFromSQL()
{
	mysql_close(&mysql_);
}
