# Домашнее задание 1
Делаем fork от этого репозитория кнопка на картинке:
![fork button][fork_button.png]

К следующему занятию нам понадобится библиотека Poco, её сборка занимает некоторое время. Лучше собрать ее дома, поэтому задание на дом следующее:

Используя conanfile
```
[requires]
Poco/1.9.0@pocoproject/stable
jsonformoderncpp/3.3.0@vthiery/stable

[generators]
cmake

[options]
Poco:shared=True
```

Написать программу которая, принимая на stdin строку вида `https://github.com/460s/meeting_backend/edit/hello-conan/README.md`
выводит в stdout json документ вида
```
{
  "schema": "https",
  "host": "github.com",
  "path": "/460s/meeting_backend/edit/hello-conan/README.md"
}
```
Затем выходит

Использовать:
- Как работать с конаном - https://docs.conan.io/en/latest/getting_started.html
- Дока на нужные классы от Poco - https://pocoproject.org/slides/160-URIandUUID.pdf
- Использовать nlohmann::json - https://github.com/nlohmann/json

