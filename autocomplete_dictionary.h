//Реализация функции автодополнения слов по введенному префиксу в консоли.
//Работает только с кириллицей.
//Ё и ё автоматически приравниваются к Е и и, что сокращает 
//количество операций и переменных.
#pragma once

#include <string>
#include <vector>

const size_t ALPHABET_SIZE = 32;

// Структура узела дерева 
class AutocompleteDictionary
{
  AutocompleteDictionary* child_[ALPHABET_SIZE] = {};
  bool endOfWord_ = false;
public:
  std::vector<std::string> prefixes_;
  int prefCount = 0;
  //Контейнер, в который будет складывать весь поток из строки ввода.
  std::string stream_;
  AutocompleteDictionary();
  ~AutocompleteDictionary() = default;
  void Remove(AutocompleteDictionary* &root);
  void Insert(AutocompleteDictionary* &root, std::string const& word);
  auto Search(AutocompleteDictionary* &root, std::string const& word) -> bool;
  void FindAllPrefixes(AutocompleteDictionary* &root, char* wordConstructor, 
    int index, std::string const& word);
  void PrintAutocomplete(AutocompleteDictionary* &root, size_t position);
};
