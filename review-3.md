
Занятие №3. Интеграция с базой данных

**Домашнее задание:**
* Заменить работу с контейнером map, внутри приложения, на работу с базой данных
* Использовать СУБД Sqlite. `sudo apt install sqlite3`
* Встроенная библитека poco для работы с sqlite. Документация: https://pocoproject.org/docs/00200-DataUserManual.html
* Подсказки по SQL командам: http://www.sqltutorial.org/sql-cheat-sheet/
* Мини гайд C++ https://github.com/adambard/learnxinyminutes-docs/blob/master/ru-ru/c%2B%2B-ru.html.markdown

В приложении условимся использовать минимальную структуру данных:
```
struct Meeting {
	std::optional<int> id;
	std::string name;
	std::string description;
	std::string address;
	bool published;
};
```


