# Работа с конаном

## Создание библиотеки
В каталоге get_user находится conan-пакет с библиотекой.

В conanfile.py - описание пакета, как его собирать и какие файлы должны получиться

В include/ - заголовочные файлы

В src/ - исходные файлы библиотеки

Сборка библиотеки
```bash
cd get_user # переходим 
conan create . demo/testing
```

**demo** - пользователь, чтобы отличать библиотеки одной версии от разных поставщиков
**testing** - стадия разработки, может принимать значения stable, testing, experimental, rc и другие

В ~/.conan создастся локальный пакет.

## Сборка проекта с зависимостями

Теперь переходим в каталог с файлом main и делаем там:
```bash
mkdir build
cd build
conan install ..
cmake ..
make
```
Наш бинарь будет располагаться по адресу bin/hello

Сделаем на него **ldd**
```bash
ldd bin/hello
```
Увидим какие динамические библиотеки используются.

## Различия в динамических и статических библиотеках

### Динамическая линковка
Проверим наличие строки World в нашем бинаре
```bash
strings bin/hello | grep World
```
Строки быть не должно

Проверим наличие строки World в нашей либе
```bash
strings ~/.conan/.../libget_user.so | grep World
```
Строка должна быть

### Статическая линковка
Теперь слинкуем hello с библиотекой libget_user статически
Для этого в conanfile.txt пропишем
```
[options]
get_user:shared=False
```

И снова
```bash
conan install ..
cmake ..
make
```

Проверим наличие строки World в нашем бинаре
```bash
strings bin/hello | grep World
```
Строка должна быть

Проверим наличие строки World в нашей либе
```bash
strings .conan/.../libget_user.a | grep World
```
Строка должна быть

# conan cheetsheet
conan create . demo/testing
conan install .. # путь до conanfile.txt
conan info .. # путь до conanfile посмотреть зависимости проекта
