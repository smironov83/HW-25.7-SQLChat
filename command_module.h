//Реализует все функциональные команды, предусмотренные программой.
//Меню входа в чат: регистрация, аутентификация, выход из программы.
//Меню чата: отправка сообщений всем, выбранному пользователю,
//просмотр профиля выбранного пользователя, выход из аккаунта.
//Отправка сообщений. Сохранение истории сообщений. 
//ChatBot - принцип попугая, реагирует на все сообщения в чате, в зоне его
//видимости. Всегда онлайн. Словарный запас ограничен, но доступно расширение 
//через код. Задел на будущего конкурента AI ChatGPT (пока времени не хватило).
//Реализована функция автодополнения слов по префиксу.
#pragma once
#include "message.h"
#include "user.h"
#include "autocomplete_dictionary.h"
#include <mysql.h>

constexpr auto DB_SQL_SERVER = "localhost";
constexpr auto DB_SQL_LOGIN = "root"; 
constexpr auto DB_SQL_PASSWORD = "547302";
constexpr auto DB_SQL = "sqlchat";

class CommandModule
{
	std::vector<User<std::string, std::vector<std::string>>> users_;
	User<std::string, std::vector<std::string>>* user_ = nullptr;
	Message* message_ = nullptr;
	std::vector<std::string> chatBotAnswers_;
	size_t currentUser_ = 0;
	AutocompleteDictionary* autoDict_ = new AutocompleteDictionary;
	char wordConstructor_[100] = {};
	char symbol_ = {};
	size_t position_ = 0;
	std::string prefix_;
	MYSQL mysql_ = {};
	MYSQL_RES *res_ = {};
	MYSQL_ROW row_ = {};
	void Registration();
	auto LogIn() -> bool;
	void MessageToAll();
	void MessageToUser();
	void PrintUsers();
	void UserInfo();
	auto AnswerChatBot() -> std::string;
public:
	bool worksChat_ = true;
	bool worksUser_ = false;
	bool worksSQL_ = true;
	bool autocompleteEnable_ = false;
	CommandModule() = default;
	~CommandModule() = default;
	auto LoginMenu() -> bool;
	void InitChatBot();
	auto ChatMenu() -> bool;
	void PrintHistory();
	void PrintAllHistory();
	void SaveHistory(std::string const &history, size_t userIndex);
	void AddHistory(std::string const &history, size_t userIndex);
	void InitAutoDict();
	auto characterInput(std::string const& text) -> std::string;
	void ConnectionToSQL();
	void DisconnectionFromSQL();
};