#include "autocomplete_dictionary.h"
#include <iostream>

//Создает новый узел с пустыми детьми.
AutocompleteDictionary::AutocompleteDictionary()
{
  //Устанавливает флаг конца слова в false.
  endOfWord_ = false;
  //Инициализирует детей нулевым уазателем.
  for (size_t i = 0; i < ALPHABET_SIZE; i++)
    child_[i] = nullptr;
}
//Рекурсивно удаляет префиксное дерево.
void AutocompleteDictionary::Remove(AutocompleteDictionary* &root)
{
  for (auto i = 0; i < ALPHABET_SIZE; ++i)
    if (root->child_[i] != nullptr)
    {
      Remove(root->child_[i]);
      delete root->child_[i];
    }
}
//Вставляет ключ в дерево, если его нет.
//Иначе, если ключ явлется префиксом узла дерева, 
//помечает в качестве листового т.е. конец слова.
void AutocompleteDictionary::Insert(AutocompleteDictionary* &root,
  std::string const& word)
{
  AutocompleteDictionary* node = root;
  for (auto i = 0; i < word.length(); i++)
  {
    //Вычисляет индекс в алфавите через смещение относительно первой буквы.
    int index = word[i] + 32;
    //Если буква заглавная, то переводит в строчную.
    if (index < 0)
      index += 32;
    //Если указатель пустрой, т.е. детей с таким префиксом нет, 
    //то создает новый узел
    if (!node->child_[index])
      node->child_[index] = new AutocompleteDictionary;
    node = node->child_[index];
  }
  //Помечает последний узел как лист, т.е. конец слова.
  node->endOfWord_ = true;
}
//Возврашает true, если префикс содержится в дереве, иначе false. 
auto AutocompleteDictionary::Search(AutocompleteDictionary* &root,
  std::string const& word)->bool
{
  AutocompleteDictionary* node = root;
  for (size_t i = 0; i < word.length(); i++)
  {
    int index = word[i] + 32;
    if (index < 0)
      index += 32;
    if (!node->child_[index])
      return false;
    node = node->child_[index];
  }
  return true;
}
//Ищет все слова в дереве соответствующие префиксу.
void AutocompleteDictionary::FindAllPrefixes(AutocompleteDictionary* &root,
  char* wordConstructor, int index, std::string const& prefix)
{
  if (!root)
    return;
  //Очищает контейнер слов, содержащих префикс при первом вхождении в функцию.
  if (index == 0)
  {
    prefCount = 0;
    std::vector<std::string>().swap(prefixes_);
  }
  //Рекурсивно проходит узлы дерева согласно префиксу (снижение количества 
  //операций). Далее перебирает все существующие цепочки и сохраняет полученные
  //слова в контейнере подходящих под префикс слов. Заглавные буквы приводятся
  //к строчным и вне зависимости от регистра ввода учитываются программой.
  if (prefix.length() > index)
  {
    wordConstructor[index] = prefix[index];
    int i = wordConstructor[index] + 32;
    if (i < 0)
      i += 32;
    FindAllPrefixes(root->child_[i], wordConstructor, index + 1, prefix);
  }
  else
    for (int i = 0; i < ALPHABET_SIZE; ++i)
      if (root->child_[i] != nullptr)
      {
        wordConstructor[index - prefix.length()] = i - 32;
        if (root->child_[i]->endOfWord_)
        {
          wordConstructor[static_cast<unsigned long long>(index) + 1 -
            prefix.length()] = '\0';
          prefixes_.push_back(std::string(wordConstructor));
          ++prefCount;
        }
        FindAllPrefixes(root->child_[i], wordConstructor, index + 1, prefix);
      }
  //Если префикс = слову в дереве и уникален, создает 1 пустой элемент 
  //контейнера слов, содержащих префикс, для избежания ошибки обращения 
  //к несуществующему элементу контейнера
  if (root->endOfWord_ && prefCount == 0)
  {
    prefixes_.push_back("");
    ++prefCount;
  }
}
//Выводит на экран вариант автодополнения слова по префиксу, выделяя 
//предлагаемую часть альтернативным цветом фона. Проверяет наличие 
//альтернативных вариантов в контейнере слов, содержащих текущий префикс.
//Наличие вариантов предшествующих текущему отмечается "<-", 
//последующих - "->'.
void AutocompleteDictionary::PrintAutocomplete(AutocompleteDictionary* &root,
  size_t position)
{
  printf_s("\033[46m");
  std::cout << root->prefixes_[position];
  printf_s("\033[0m");
  if (position > 0)
    std::cout << "<-";
  if (root->prefCount > position + 1)
    std::cout << "->";
}
